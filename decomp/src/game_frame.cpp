// The in-game frame (game mode 1, called by the frame step): the first frame resets the mode's globals, then one pass
// of the game logic runs for every engine tick that has elapsed (input, the main menu, the player, the characters, the
// effects, the camera); the scene is prepared and drawn once at the end. Written in stages: the markers
// "// STAGE n" below stand for code that is not reconstructed yet (pseudo.c is the Ghidra pseudocode, see STAGES.md).

#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

#include "model_bank.h"

#include "char_slot.h"

#include "game.h"

#include "tracker.h"

#include "dinput_min.h"

#include "demo_recording.h"

#include "bgm_track.h"

#include "item_dictionary.h"

#include "message_window.h"

#include "field_item_menu.h"

#include "partner_follow.h"

#include "bs003.h"

#include "hud_frame.h"

#include "menu_main.h"

#include "camera_refresh_vars.h"

#include "scene_draw.h"

#include "npc_lookout.h"

#include "char_vars.h"

#include "char_tables.h"

#include "camera_vars.h"

#include "map_vars.h"

#include "ui_vars.h"

#include "render_vars.h"

#include "game_vars.h"

#include "sound_vars.h"

#include "script_system.h"

#include "map_setup.h"

#include "field_update.h"

#include "title_screen.h"

#include "engine_memory.h"

#include "frame_vars.h"

#include "world.h"

#include "game_load_data.h"

#include "player_frame.h"

#include "action_select_vars.h"

#include "game_main.h"

#include "script_vars.h"

#include "game_frame.h"

// Most engine ticks run in one frame; a longer backlog (or a halted engine) runs a single tick.
#define MAX_CATCH_UP_TICKS 20
// Menu windows closed when a queued event interrupts the open overlay mode.
#define WINDOW_ID_SAVE_MENU 0x22c
#define WINDOW_ID_CHOICE_MENU 0x218
#define WINDOW_ID_YES_NO 0x216
// Texture slots of the HUD (DAT_10202808) released by the first frame.
#define HUD_TEXTURE_SLOT_COUNT 15
// Texture slots of the overlay mode (DAT_1030b870).
#define OVERLAY_TEXTURE_SLOT_COUNT 0x22
// Characters whose models the frame updates (DAT_10db6ee8[0..MODEL_UPDATE_COUNT-1]).
#define MODEL_UPDATE_COUNT 124
// Tracks below this number drive the beat counter DAT_1032d6a0.
#define BGM_TRACK_LIMIT 0xff
// Beat counter: BGM_BEAT_STEPS steps per BGM_BEAT_SAMPLES / BgmTrack::m_002c samples.
#define BGM_BEAT_STEPS 15
#define BGM_BEAT_SAMPLES 0x285ff0
// The model of the second character (the partner), recorded in the demo.
#define SECOND_MODEL 75
// While model NEAR_MODEL is within NEAR_MODEL_DISTANCE of the player DAT_104c02f0 is 1.
#define NEAR_MODEL 16
#define NEAR_MODEL_DISTANCE 300.0f
// Recorded demo: the playback position at which it ends, and the music fades started on a desync or at the end.
#define DEMO_END_FRAME 0x3d22
#define DEMO_DESYNC_FADE_FRAMES 0xf
#define DEMO_END_FADE_FRAMES 0x1e
#define BGM_VOLUME_FULL 100
// The sun's lens flare is only added when DAT_10467798 is above this.
#define LENS_FLARE_MIN_COUNT 2

// 0x1011d810 (the in-game frame). Mode 1 of the frame step. The first call resets the mode's globals (DAT_104bf8bc
// was 0 until then). Each call computes how many engine ticks have passed since the last one (at most
// MAX_CATCH_UP_TICKS, otherwise one); every tick polls the input (and records or replays it), runs the main menu and
// the queued events and then the whole game logic for the player and the characters. After the last tick the model
// bank and the lens flare are prepared and the scene is drawn. Returns 0 when the frame was cut short, 1 otherwise.
extern "C" int FUN_1011d810(void)
{
    int ticks;
    int tick;
    int inputActive;
    int stepForced;
    int engineTick;
    int windowBusy;
    int modelCount;
    int modelPos;
    int modelIdx;
    int i;
    int modelOrder[125];

    if (DAT_104bfd84 != 0)
        return 0;
    if (DAT_104bf8bc == 0)
    {
        DAT_102024a8 = 1;
        DAT_104bf8bc = 1;
        DAT_10db9380 = 1;
        DAT_11018734 = 1;
        DAT_1101de30 = 1;
        DAT_104687b0 = DAT_10db9274;
        memset(DAT_102550e0, 0, sizeof(DAT_102550e0));
        memset(DAT_10253e98, 0, sizeof(DAT_10253e98));
        memset(DAT_104686b0, 0, sizeof(DAT_104686b0));
        memset(DAT_10210d30, 0, sizeof(DAT_10210d30));
        memset(DAT_1030c278, 0, sizeof(DAT_1030c278));
        DAT_104bf8b4 = 0;
        DAT_104c00b2 = 0;
        DAT_104c00b0 = 0;
        DAT_102024bc = 0xffff;
        DAT_104c0092 = 0;
        DAT_104c0093 = 0;
        DAT_104c00a0 = 0;
        DAT_102024b8 = -1;
        DAT_104c0097 = 0;
        DAT_104c0094 = 0;
        DAT_1030c478[0] = 0;
        DAT_1049f1f8[0] = 0;
        DAT_1030c479 = 0;
        DAT_1049f1f9 = 0;
        DAT_1030c47a = 0;
        DAT_1049f1fa = 0;
        DAT_1030c47b = 0;
        DAT_1049f1fb = 0;
        DAT_104c00b7 = 0;
        DAT_104c0128 = 0;
        DAT_10f6553c = Hud_DrawFrame;
        DAT_104c010c = 0;
        DAT_10db6ee8[0].m_0075 = 0;
        DAT_104c0068 = 0;
        DAT_104bff00 = 0;
        DAT_104c009e = 0;
        DAT_104c009d = 0;
        DAT_102024b0 = -1;
        DAT_104c0030 = 0;
        DAT_104bfeb9 = 0;
        DAT_1030b868 = 0;
        DAT_104bfd74 = 4;
        DAT_104bfeb0 = 0;
        DAT_104bfe70 = 0;
        DAT_104bfe74 = 0;
        DAT_104c0028 = 0;
        DAT_104c002c = 0;
        DAT_104c00bd = 0;
        DAT_104bfe30 = 0;
        DAT_104bfdd0 = 0;
        DAT_104bfdcc = 0;
        DAT_104bfddc = 0;
        DAT_104bfdc8 = 0;
        DAT_1020d430 = 0;
        DAT_10468128 = 0;
        DAT_104bfe20 = 0;
        DAT_104c009c = 0;
        DAT_104bfd78 = 0;
        DAT_104bfd80 = 0;
        DAT_104bfe3c = 0;
        DAT_104bfe7c = 0;
        DAT_104bfe4c = 0;
        DAT_104bfe50 = 0;
        for (i = 1; i < CHAR_SLOT_COUNT; i++)
            DAT_10daaae8[i].stats[STAT_HEALTH] = DAT_10daaae8[i].stats[STAT_MAX_HEALTH];
        if (DAT_104b7210 > -1)
            FUN_10174c70(DAT_104b7210, 0);
        DAT_104b7210 = -1;
        for (i = 0; i < HUD_TEXTURE_SLOT_COUNT; i++)
        {
            if (DAT_10202808[i] > -1)
                FUN_10174c70(DAT_10202808[i], 0);
            DAT_10202808[i] = -1;
        }
        FUN_1013fa10(0, 0, 1.0f, 1.0f, 1.0f, 1.0f);
        FUN_1013fa10(1, 0, 0.0f, 0.0f, 0.0f, 1.0f);
        FUN_1000ec70(0, 1.0f, 1.0f, 1.0f, 1.0f);
        DAT_1101ddf4 = 0;
        DAT_104c0138 = GetTickCount();
    }
    if (DAT_1065cb90 != 0 && DAT_104bf8b8 == DAT_10f65504)
        return 0;
    DWORD now = GetTickCount();
    if (now - DAT_104c0138 >= 1000)
    {
        unsigned int seconds = (now - DAT_104c0138) / 1000;
        DAT_104c00dc = DAT_104c00dc + seconds;
        DAT_104c0138 = DAT_104c0138 + seconds * 1000;
    }
    if (DAT_10db6ee8[NEAR_MODEL].root == 0)
        DAT_104c02f0 = 0;
    else
    {
        float modelDistance = DAT_10db6ee8[NEAR_MODEL].matrix.GetTranslation().Distance(DAT_10db6ee8[0].matrix.GetTranslation());
        DAT_104c02f0 = 1;
        if (modelDistance >= NEAR_MODEL_DISTANCE)
            DAT_104c02f0 = 0;
    }
    windowBusy = DAT_104b7200.IsAnyWindowBusy();
    engineTick = DAT_10f65504;
    stepForced = DAT_104c0134;
    if (windowBusy != 0 || (DAT_104c0135 = 0, DAT_104c03dc != 0))
        DAT_104c0135 = 1;
    DAT_10db6ee8[DAT_104bf8b4].m_02a0 = 0;
    if (DAT_104bf8b8 == engineTick && stepForced == 0)
        goto prepareFrame;
    if (DAT_104bf87c > 0 && DAT_104bf87c < 10)
        DAT_104bf87c = DAT_104bf87c + 1;
    ticks = engineTick - DAT_104bf8b8;
    DAT_104bf8b8 = engineTick;
    if (DAT_1065cb90 != 0)
        DAT_104c0184 = DAT_104c0184 + 0xf - ticks * 5;
    if (ticks > MAX_CATCH_UP_TICKS || DAT_104bf8a0 != 0 || DAT_104bf880 != 0 || ticks == 0 || stepForced != 0
        || DAT_1065cb90 != 0)
        ticks = 1;
    DAT_104c020c = 1;
    for (tick = 0; tick < ticks; tick++)
    {
        if (DAT_1032d69c != 0 && DAT_104b7200.IsQueuedEventReady() != 0)
        {
            if (DAT_104bfe1c != 0)
            {
                DAT_1030c54c = 0;
                DAT_104bfd78 = 0;
                DAT_104b7200.CloseWindow(WINDOW_ID_SAVE_MENU);
                DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
                DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
                DAT_1020e0f8[0] = 0;
                FUN_1000cb30("");
                FUN_1000cb90("");
                DAT_104bf8d4 = 1;
                if (DAT_104c03c8 != 0)
                {
                    DAT_104c03dc = 0;
                    DAT_104c03c8->Release();
                    DAT_11b1ba40(DAT_104c03c8);
                    DAT_104c03c8 = 0;
                    DAT_104c03cc = 0;
                    DAT_104c03d0 = 0;
                    DAT_104c03d4 = 0;
                    DAT_104c03d8 = 0;
                    DAT_104c03e0 = 0;
                    DAT_104c03e4 = 0;
                    for (i = 0; i < OVERLAY_TEXTURE_SLOT_COUNT; i++)
                    {
                        if (DAT_1030b870[i] > -1)
                        {
                            FUN_10174c70(DAT_1030b870[i], 1);
                            DAT_1030b870[i] = -1;
                        }
                    }
                }
                DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
                DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
                DAT_1020e0f8[0] = 0;
                DAT_104bfe1c = 0;
                memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
            }
            DAT_104bf8b8 = DAT_104bf8b8 + (tick - ticks);
            break;
        }
        FUN_1000e870();
        DAT_10210b2c = 0;
        DAT_1030c990 = 0;
        DAT_1020d9e0.M_10001110(0, 0, 0, 0, 0, 0);
        DAT_104c0134 = 0;
        inputActive = 0;
        DAT_10f08008.Update();
        DAT_10210f30 = DAT_10f08008.ReadControls(0);
        DAT_1020d9e0.M_10001550();
        DAT_104bf890 = DAT_104bf890 + 1;
        if (DAT_10202358 < BGM_TRACK_LIMIT && DAT_108db358[DAT_10202358].m_002c != 0)
            DAT_1032d6a0 = ((DAT_10254088 - DAT_108db358[DAT_10202358].start) * BGM_BEAT_STEPS)
                           / (BGM_BEAT_SAMPLES / DAT_108db358[DAT_10202358].m_002c);
        DAT_1065cb28.SetButtons(DAT_10210f30);
        DAT_1065cb28.M_1015caa0(DAT_10f08008.ReadControls(1), 0);
        DAT_1065cb28.M_1015caa0(DAT_10f08008.ReadControls(2), 1);
        DAT_1065cb28.M_1015caa0(DAT_1032d6a0, 2);
        DAT_1065cb28.M_1015caa0(DAT_10f65558, 3);
        DAT_1065cb28.M_1015caa0(DAT_104bf890, 4);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[0].matrix._41, 0);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[0].matrix._42, 1);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[0].matrix._43, 2);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[SECOND_MODEL].matrix._41, 3);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[SECOND_MODEL].matrix._42, 4);
        DAT_1065cb28.M_1015cac0(DAT_10db6ee8[SECOND_MODEL].matrix._43, 5);
        DAT_1065cb28.M_1015cac0(DAT_118e6cb8.x, 6);
        DAT_1065cb28.M_1015cac0(DAT_118e6cb8.y, 7);
        DAT_1065cb28.M_1015cac0(DAT_118e6cb8.z, 8);
        if (DAT_1065cb94 != 0 && DAT_1065cb90 != 0)
            DAT_1065cb28.RecordFrame();
        if (DAT_1065cb98 != 0 && DAT_1065cb90 != 0)
        {
            DAT_1065cb28.PlayFrame();
            DAT_1032d6a0 = DAT_1065cb28.values_now[2];
            if (DAT_104bf890 != DAT_1065cb28.values_now[4] || DAT_10f65558 != DAT_1065cb28.values_now[3]
                || DAT_10db6ee8[0].matrix._41 != DAT_1065cb28.floats_now[0]
                || DAT_10db6ee8[0].matrix._42 != DAT_1065cb28.floats_now[1]
                || DAT_10db6ee8[0].matrix._43 != DAT_1065cb28.floats_now[2]
                || DAT_10db6ee8[SECOND_MODEL].matrix._41 != DAT_1065cb28.floats_now[3]
                || DAT_10db6ee8[SECOND_MODEL].matrix._42 != DAT_1065cb28.floats_now[4]
                || DAT_10db6ee8[SECOND_MODEL].matrix._43 != DAT_1065cb28.floats_now[5]
                || DAT_118e6cb8.x != DAT_1065cb28.floats_now[6] || DAT_118e6cb8.y != DAT_1065cb28.floats_now[7]
                || DAT_118e6cb8.z != DAT_1065cb28.floats_now[8])
            {
                DAT_1065cb28.playFrame = DAT_1065cb28.frameCount;
                Bgm_Play(0, 0);
                DAT_104c012c = 0;
                FUN_1000da40(0, BGM_VOLUME_FULL, DEMO_DESYNC_FADE_FRAMES);
            }
            if (DAT_1065cb90 != 0 && DAT_1065cb28.playFrame == DEMO_END_FRAME)
            {
                Bgm_Play(0, 0);
                DAT_104c012c = 0;
                FUN_1000da40(0, BGM_VOLUME_FULL, DEMO_END_FADE_FRAMES);
            }
        }
        if (DAT_104bfe7c != 0)
            DAT_104bfe7c = DAT_104bfe7c - 1;
        if (FUN_10112140() != 0)
            return 0;
        UpdateDrillLevel();
        if ((DAT_104bf890 & 1) != 0 && DAT_10255f08 != 0)
            DAT_10255f08->M_10194ee0();
        if (DAT_104bfe1c == 0 && DAT_104bfe6c == 1)
        {
            DAT_104bf8a4 = DAT_10d4ee30;
            FUN_10189080();
        }
        Camera_Refresh();

        // STAGE 1: per-tick timers, the overlay and book state, pseudo.c lines 1635-2927
        // STAGE 2: model sweeps, the player's mode counters and the event fades, pseudo.c lines 2929-3477

        if (DAT_104c00d0 < 2 && FUN_1015d510() == 0 && DAT_104c0334 == 0 && DAT_104bf890 > 4 && DAT_104c0135 == 0
            && DAT_104bfe78 == 0 && DAT_104bfe6c == 0 && DAT_104bfd08 == 0 && DAT_104bfd60 == 0
            && DAT_10db6ee8[DAT_104bf8b4].M_1017b160() == 0)
        {
            inputActive = 1;

            // STAGE 3: player input and its actions, first part, pseudo.c lines 3483-4499
            // STAGE 4: player input and its actions, second part, pseudo.c lines 4500-5864
            // STAGE 5: player input and its actions, last part (ends the block), pseudo.c lines 5865-6523
        }
        // The block above is left by a goto at pseudo line 6480 (label LAB_1012984b, next line).

        // STAGE 6: the player after the input, the camera and the world, pseudo.c lines 6525-7860
        // STAGE 7: the events, the carried object and the model list, pseudo.c lines 7861-8944

        DAT_104c0098 = 0;
        if (DAT_104bfdc8 == 0)
        {
            DAT_102027e4 = 135.0f;
            DAT_104bfdd8 = 0;
        }
        DAT_104bf888 = 0;
        if (DAT_104bfdc8 == 0)
            DAT_104bfdd0 = 0;
        if (DAT_10255f08 != 0)
            DAT_10255f08->UpdateVisibleCells();
        modelCount = 0;
        for (i = 0; i < MODEL_UPDATE_COUNT; i++)
        {
            if (DAT_10db6ee8[i].root != 0)
            {
                modelOrder[modelCount] = i;
                modelCount = modelCount + 1;
            }
        }
        qsort(modelOrder, modelCount, sizeof(int), FUN_1000c2f0);
        if (DAT_104c0135 == 0)
        {
            for (modelPos = 0; modelPos < modelCount; modelPos++)
            {
                modelIdx = modelOrder[modelPos];
                DAT_104c01f0 = 0;
                if (DAT_10db6ee8[modelIdx].root != 0)
                {
                    // STAGE 8: the model's fade timers and its pending removal, pseudo.c lines 8984-9022
                    if (modelIdx > 0 && DAT_104bfe6c == 0 && DAT_104bfeb0 == 0)
                    {
                        if (DAT_1021f3c0[modelIdx] == 3 && modelIdx > 0x39)
                        {
                            // STAGE 9: the enemy behaviours (slot kind 3), pseudo.c lines 9026-9683
                        }
                        else
                        {
                            // STAGE 10: the other slot kinds and the per-kind dispatch, pseudo.c lines 9686-10956
                        }
                    }
                }
                // STAGE 11: the player's own update and the others' tail work (label LAB_10135f91), pseudo.c lines 10960-11281
            }
        }

        // STAGE 12: the models' draw passes, the counters and the model bank, pseudo.c lines 11283-11597
    }
prepareFrame:
    DAT_10804bf8.PrepareFrame(&DAT_10836ec8, &DAT_108392d8, DAT_1032d67c[0], 1);
    if (DAT_10467798 > LENS_FLARE_MIN_COUNT)
        DAT_10804bf8.AddFlarePoint(DAT_10864ce8, &DAT_108392d8);
    if (DAT_104bf890 != 0)
        Scene_DrawFrame(0);
    return 1;
}
