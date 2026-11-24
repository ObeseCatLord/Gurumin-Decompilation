// The in-game frame (game mode 1, called by the frame step): the first frame resets the mode's globals, then one pass
// of the game logic runs for every engine tick that has elapsed (input, the main menu, the player, the characters, the
// effects, the camera); the scene is prepared and drawn once at the end. Written in stages: the markers
// "// STAGE n" below stand for code that is not reconstructed yet (pseudo.c is the Ghidra pseudocode, see STAGES.md).

#include <math.h>
#include <stdio.h>
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

#include "hop_spit_enemy.h"

#include "node_collision_vars.h"

#include "shaga.h"

#include "texture_manager.h"

#include "name_entry.h"

#include "doppler_loops.h"

#include "scene_command_ids.h"

#include "texture_load.h"

#include "mesh_lit_vars.h"

#include "map_trigger_setup.h"

#include "tutorial_book.h"

#include "ground_height.h"

#include "ground_mesh.h"

#include "scene_callback.h"

#include "texture_object.h"

#include "mmv_player.h"

#include "dshow_stream.h"

#include "char_hikari.h"

#include "game_frame.h"

// Hidden-part loop, light shafts: DAT_10f65848 holds SHAFT_COUNT shafts (x, y offsets and w opacity).
#define SHAFT_COUNT 2
#define SHAFT_LIMIT 2500.0f
#define SHAFT_FADE 0.05f
#define SHAFT_SPEED 20.0f
#define SHAFT_RESTART 0x5dc
// Effect holds that start an effect with a fire flag.
#define HOLD_VALUE_FIRE_A 0x10
#define HOLD_VALUE_FIRE_B 0x50
// Frames the HUD stays shown after one of its triggers.
#define HUD_HOLD_FRAMES 0x96
// Rain effect in the weather box (mode 2).
#define RAIN_EFFECT 0x82dc
// Levers ("@kskNN+") and their parts.
#define KSK_COUNT 0x40
#define KSK_PARTS 6
// Fish (DAT_10210f38): FISH_MAX swim, the other nodes up to FISH_NODE_COUNT are hidden far away.
#define FISH_MAX 8
#define FISH_NODE_COUNT 0x20
#define FISH_HIDDEN_X -200000.0f
#define FISH_START_Y 2000.0f
#define FISH_DEPTH_MIN 260
#define FISH_DEPTH_MAX 400
#define FISH_FLEE_START 250.0f
#define FISH_FLEE_END 400.0f
#define FISH_ARRIVE_DISTANCE 10.0f
#define FISH_SWIM_SPEED 5.0f
#define FISH_FLEE_SPEED 10.0f
#define FISH_WAVE_FRAMES 40
#define FISH_RADIUS 20.0f
#define FISH_TURN_TRIES 10
#define FISH_TURN_SMALL 0.785375f
#define FISH_TURN_LARGE 1.57075f
#define FISH_COURSE_KEEP 0.8f
#define FISH_COURSE_STEER 0.2f
#define FISH_PI 3.1415f
// The swing (DAT_104c0240): seats within SWING_REACH of the player (2D), beat of SWING_BEAT frames in two halves.
#define SWING_MIN_HEIGHT 5.0f
#define SWING_REACH 150.0f
#define SWING_START_POWER 13
#define SWING_LEVEL_STEP 10
#define SWING_BEAT 0x60
#define SWING_HALF_BEAT 0x30
#define SWING_SAVE_POWER_BEAT 0x28
// The player's scene state DAT_104c00d0: ranges and the steps inside them.
#define SCENE_TIMELINE_LIMIT 0xc9       // 2..200: timeline counting down from SCENE_TIMELINE_START to 1
#define SCENE_BLANK_LIMIT 1000          // 201..999 do nothing
#define SCENE_SPARK_LIMIT 2000          // 1000..1999: spark effect at the player
#define SCENE_HAND_LIMIT 5000           // 2000..4999: the left hand glows
#define SCENE_DAMAGE_LIMIT 0x2711       // 5000..10000: damage state; 10001..19999 do nothing
#define SCENE_SHOVE_WAIT 20000          // waits for the player to stand still, then becomes SCENE_SHOVE
#define SCENE_SHOVE 0x4e21              // the player is shoved aside
#define SCENE_SWAN_WAIT 30000           // starts the swan ride, then becomes SCENE_SWAN
#define SCENE_SWAN 0x7531
#define SCENE_TIMELINE_START 200
#define SCENE_TIMELINE_EFFECT 0xaa
#define SCENE_TIMELINE_VOICE 0x74
#define SCENE_TIMELINE_SHADE 0x6e
#define SCENE_TIMELINE_RELEASE 0xc6
#define SCENE_SPARK_START 0x3ec
#define SCENE_SPARK_END 0x41b
#define SCENE_SPARK_FINISH 1000
#define SCENE_HAND_START 0x7e3
#define SCENE_HAND_END 0x806
#define SCENE_HAND_FINISH 2000
#define SCENE_DAMAGE_TICK 0x13a6
#define SCENE_DAMAGE_RECOVER 0x13c4
#define SCENE_DAMAGE_END 0x13a0
// The second player model (a double of the player, shown unless the costume has three pointers).
#define SCENE_DOUBLE_MODEL 124
#define SCENE_HIDDEN_X -200000.0f
#define COSTUME_THREE_POINTERS 6
// Face bits of the worn helmet (DAT_104c02c8) that scale the effect strength, and the damage bits.
#define HELMET_FACE_400 0x400
#define HELMET_FACE_300 0x300
#define HELMET_FACE_200 0x200
#define HELMET_FACE_100 0x100
#define HELMET_NO_DAMAGE 0x8000000
#define HELMET_HALF_DAMAGE 0x4000000
// The shove: four directions of length SHOVE_SIDE sideways and SHOVE_DOWN down.
#define SHOVE_SIDE 10.0f
#define SHOVE_DOWN -5.0f
// The swan ride: frames before it ends, the player's heading offset to the swan and the sound period.
#define SWAN_RIDE_FRAMES 200
#define SWAN_HEADING_OFFSET 1.57075f
#define SWAN_SOUND_PERIOD 0xf
#define PLAYER_TRAIL_LENGTH 16
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

// Closes the event overlay mode if it is open (windows, caption, book model and its textures). The scene states
// run this code at the top of each timeline.
static __forceinline void CloseEventOverlay(void)
{
    int slot;
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
            for (slot = 0; slot < OVERLAY_TEXTURE_SLOT_COUNT; slot++)
            {
                if (DAT_1030b870[slot] > -1)
                {
                    FUN_10174c70(DAT_1030b870[slot], 1);
                    DAT_1030b870[slot] = -1;
                }
            }
        }
        DAT_104b7200.CloseWindow(WINDOW_ID_CHOICE_MENU);
        DAT_104b7200.CloseWindow(WINDOW_ID_YES_NO);
        memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
        DAT_1020e0f8[0] = 0;
        DAT_104bfe1c = 0;
    }
}

// Fills the ring of recent player positions with the current one.
static __forceinline void ResetPlayerTrail(void)
{
    DAT_104c0228 = 0;
    DAT_104c022c = 0;
    DAT_104c022d = 0;
    for (int step = PLAYER_TRAIL_LENGTH; step != 0; step--)
    {
        DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        DAT_104c022c = DAT_104c022c + 1 & 0xf;
    }
}

// Doors (switch models of kind 0x13/0x14, models DOOR_FIRST_MODEL to DOOR_END_MODEL - 1): animation frames of
// DAT_1020b128 that start a motion of the door's node tree.
#define DOOR_FIRST_MODEL 0x5c
#define DOOR_END_MODEL 0x7c
#define DOOR_FRAME_START 1
#define DOOR_FRAME_SETTLE 0x19
#define DOOR_FRAME_OPENED 0x1f
#define DOOR_FRAME_OPEN_LOOP 0x37
#define DOOR_FRAME_CLOSING 0x3d
#define DOOR_FRAME_CLOSE_SETTLE 0x55
#define DOOR_FRAME_CLOSED 0x5b
#define DOOR_FRAME_END 0x73
#define DOOR_SWING_EFFECT 0xfbe
// Message fade steps.
#define MESSAGE_FADE_MAX 0xf
#define MESSAGE_SLIDE_FULL 0x1e
// Dig (DAT_104bfd60): frames of a short dig; a long dig (DAT_104bfd64) lasts DIG_LONG_FRAMES per level.
#define DIG_LONG_FRAMES 600
#define DIG_START_FRAME 9
#define DIG_END_FRAME 0xc
#define DIG_DONE_FRAME 0x14
#define DIG_ANGLE_OFFSET 1.57075f
#define DIG_HEADING (-DAT_10db6ee8[DAT_104bf8b4].targetHeading - DIG_ANGLE_OFFSET)
#define DIG_REACH 40.0
#define DIG_RADIUS_SHORT 0x78
#define DIG_RADIUS_LONG 0x32
#define DIG_SHADE_OFFSET 30.0
#define DIG_SHADE_SIZE 150.0
// Event markers: projected to the 640x480 screen; they live EVENT_MARKER_FRAMES frames.
#define EVENT_MARKER_COUNT 0x80
#define EVENT_MARKER_FRAMES 0x1f
#define MARKER_HEAD_LIFT 25.0f
#define MARKER_OFFSCREEN_X -50000.0f
#define SCREEN_HALF_WIDTH 320.0f
#define SCREEN_HALF_HEIGHT 240.0f
#define SCREEN_HEIGHT 480.0f
// Game over (DAT_104bfe78 counts its frames).
#define GAME_OVER_FADE_FRAME 0x32
#define GAME_OVER_SOUND_FRAME 0x3c
#define GAME_OVER_END_FRAME 0x83
#define GAME_OVER_END_FRAME2 0x78
#define GAME_OVER_SKIP_FRAME 0x3a
#define GAME_OVER_RESTART_FRAME 0x8c

// The door models that play the swing effect when their animation starts.
#define DoorSwingsEffect(model) ((model) == 0xe7 || (model) == 0xe8 || (model) == 0xcd || (model) == 0xce)

// Starts the door swing effect at the origin of the door's own matrix.
static __forceinline void StartDoorEffect(AnimModel *door)
{
    Vec4 origin;
    origin.Fill(0.0f);
    Matrix4 placement = door->matrix;
    DAT_10daaa80.Start(DOOR_SWING_EFFECT, origin, -1, 0, 0, 0, 0, &placement, 0, 0);
}

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
    char mouseLeft;
    char mouseRight;
    char mouseMiddle;
    char mouseExtra;
    int mouseWheel;
    int mousePos[2];
    float inputStrength;
    char gameOverButton;
    char gameOverPadA;
    char gameOverPadB;
    int gameOverPos[2];
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

        // The alpha-pass nodes of the listed parts are hidden while the camera sees the player through their mesh.
        for (i = 0; i < DAT_1032d868; i++)
        {
            if (DAT_1049ef90[i]->SegmentHitsMesh(&DAT_118e69e8, &DAT_118e6cb8) == 0)
            {
                DAT_102550c0[i]->ClearFlags(NODE_FLAG_HIDDEN);
                DAT_1020aefc[i]->ClearFlags(NODE_FLAG_HIDDEN);
            }
            else
            {
                DAT_102550c0[i]->SetFlags(NODE_FLAG_HIDDEN);
                DAT_1020aefc[i]->SetFlags(NODE_FLAG_HIDDEN);
            }
        }
        DAT_10f65548 = 0;
        DAT_10f6554c = 0;

        // The two light shafts slide across the screen and fade in and out.
        for (i = 0; i < SHAFT_COUNT; i++)
        {
            Vec4 &shaft = DAT_10f65848[i];
            if (shaft.x > SHAFT_LIMIT || shaft.y > SHAFT_LIMIT)
            {
                shaft.w = shaft.w - SHAFT_FADE;
                if (shaft.w <= 0.0f)
                {
                    shaft.w = 0.0f;
                    shaft.x = (float)(-FUN_1013f880() % 1000 - SHAFT_RESTART);
                    shaft.y = (float)(-FUN_1013f880() % 1000 - SHAFT_RESTART);
                }
            }
            else
            {
                shaft.x = shaft.x + SHAFT_SPEED;
                shaft.y = shaft.y + SHAFT_SPEED;
                shaft.w = shaft.w + SHAFT_FADE;
                if (shaft.w >= 1.0f)
                    shaft.w = 1.0f;
            }
        }
        DAT_10f65368 = (float)sin((double)((float)DAT_104bf890 * 0.1f)) * 0.3f;
        if (DAT_104c0232 != 0)
            DAT_104c0232 = DAT_104c0232 - 1;
        if (DAT_104c0233 != 0)
            DAT_104c0233 = DAT_104c0233 - 1;

        // The effects the map keeps requesting start when their wait is over.
        if (DAT_104bfe1c == 0 && DAT_104c0135 == 0 && DAT_1020b22c > 0)
        {
            for (i = 0; i < DAT_1020b22c; i++)
            {
                EffectHold &hold = DAT_1020dcf8[i];
                hold.age = hold.age + 1;
                if (hold.m_0010 < hold.age)
                {
                    EffectOptions options;
                    options.m_0004 = hold.value;
                    options.settings[2] = (char)hold.m_001c;
                    if (options.m_0004 == HOLD_VALUE_FIRE_A || options.m_0004 == HOLD_VALUE_FIRE_B)
                        options.m_000e = 6;
                    options.m_0000 = 0;
                    hold.age = 0;
                    Vec4 where;
                    where.x = (float)hold.position[0];
                    where.y = (float)hold.position[1];
                    where.z = (float)hold.position[2];
                    where.w = 0.0f;
                    DAT_10daaa80.Start(hold.effect, where, -1, -1, 0, 0, 0, 0, &options, 0);
                }
            }
        }
        if (DAT_104c0128 > 0)
        {
            DAT_104c0128 = DAT_104c0128 - 1;
            if (DAT_104c0128 == 0 || DAT_104bfe6c != 0)
            {
                DAT_104c0128 = 0;
                Bgm_Play(DAT_104c012c, 0);
            }
        }

        // The HUD counters count down.
        if (DAT_104c0119 != 0)
            DAT_104c0119 = DAT_104c0119 - 1;
        if (DAT_104c011e != 0)
            DAT_104c011e = DAT_104c011e - 1;
        if (DAT_104c011f != 0)
            DAT_104c011f = DAT_104c011f - 1;
        if (DAT_104c011d != 0)
            DAT_104c011d = DAT_104c011d - 1;
        if (DAT_104c011a != 0)
            DAT_104c011a = DAT_104c011a - 1;
        if (DAT_104c011c != 0)
            DAT_104c011c = DAT_104c011c - 1;
        if (DAT_104c011b != 0)
        {
            DAT_104c011b = DAT_104c011b - 1;
            if (DAT_104c011b == 0)
                DAT_104c0114 = 0;
        }
        if (DAT_104c0108 > 10 || DAT_104c00e0 != 0 || DAT_102024c4 != 0 || DAT_102024ac > 0)
            DAT_104c0124 = HUD_HOLD_FRAMES;
        if (DAT_104bfe6c == 0)
        {
            if (DAT_104c0124 > 0)
                DAT_104c0124 = DAT_104c0124 - 1;
        }
        else
            DAT_104c0124 = 0;
        DAT_10daaae8[0].stats[STAT_EFFECT_PARAM] = 10;
        DAT_10daaae8[0].stats[STAT_VALUE_4] = 5;

        // Rain falls inside the weather box of the map: effects in mode 2, falling drops otherwise.
        if (DAT_104c0224 != 0 && DAT_1083631c.x < DAT_10db6ee8[0].matrix._41 && DAT_1083631c.y < DAT_10db6ee8[0].matrix._42
            && DAT_10db6ee8[0].matrix._41 < DAT_1083631c.z && DAT_10db6ee8[0].matrix._42 < DAT_1083631c.w)
        {
            if (DAT_104c0224 == 2)
            {
                if ((DAT_104bf890 & 1) == 0)
                {
                    Vec4 spawn = DAT_118e6cb8;
                    spawn.x = (float)(FUN_1013f880() % 2000 - 1000) + spawn.x;
                    spawn.y = (float)(FUN_1013f880() % 2000 - 1000) + spawn.y;
                    spawn.z = spawn.z - 200.0f;
                    EffectOptions options;
                    options.flags = 1;
                    DAT_10daaa80.Start(RAIN_EFFECT, spawn, -1, 0, 0, 0, 0, 0, &options, 0);
                }
            }
            else
            {
                i = 2;
                do
                {
                    Vec4 drop = DAT_118e6cb8;
                    drop.x = (float)(FUN_1013f880() % 0xa28 - 0x514) + drop.x;
                    drop.y = (float)(FUN_1013f880() % 0xa28 - 0x514) + drop.y;
                    drop.z = (float)(400 - FUN_1013f880() % 0x32) + drop.z;
                    FUN_1000c5c0(drop);
                    i = i - 1;
                } while (i != 0);
            }
        }
        FUN_1000c390();

        // The levers count down their pulse and press their parts when it ends.
        for (i = 0; i < KSK_COUNT; i++)
        {
            if (DAT_10219970[i] != 0)
                DAT_10219970[i] = DAT_10219970[i] - 1;
            if (DAT_10467718[i] != 0)
            {
                DAT_10467718[i] = DAT_10467718[i] - 1;
                if (DAT_10467718[i] == 0)
                {
                    char kskName[0x40];
                    sprintf(kskName, "@ksk%02d+", i);
                    for (modelPos = 0; modelPos < KSK_PARTS; modelPos++)
                    {
                        Node *kskPart = (Node *)DAT_10255f08->M_1019ed10(kskName, modelPos);
                        if (kskPart == 0)
                            break;
                        if (kskPart->name[strlen(kskPart->name) - 1] != '0')
                            kskPart->M_10191d90();
                    }
                }
            }
        }

        // The fish swim: they keep their distance from the player and turn away from walls.
        if (DAT_10210f38[0] != 0)
        {
            int fish;
            int dirTry;
            for (i = FISH_MAX; i < FISH_NODE_COUNT; i++)
            {
                Vec4 away;
                away.Set(FISH_HIDDEN_X, 0.0f, 0.0f, 0.0f);
                DAT_10210f38[i]->matrix.SetTranslation(away);
            }
            for (fish = 0; fish < FISH_MAX; fish++)
            {
                Vec4 swim;
                swim.Fill(0.0f);
                int swimming = 0;
                int fleeing = 0;
                DAT_1042d958[fish] = DAT_1042d958[fish] % FISH_WAVE_FRAMES;
                Vec4 pos = DAT_10210f38[fish]->matrix.GetTranslation();
                if (DAT_1021a1c0[fish] == 0)
                {
                    DAT_1021a1c0[fish] = 1;
                    pos.Set((float)(FUN_1013f880() % 0x5dc), 0.0f, (float)(-FISH_DEPTH_MIN - FUN_1013f880() % 0x3c), 0.0f);
                    pos.RotateZ((float)FUN_1013f880() / 100.0f);
                    pos.y = pos.y + FISH_START_Y;
                    DAT_10210f38[fish]->frameSource = &DAT_1042d958[fish];
                    DAT_10210f38[fish]->matrix.SetTranslation(pos);
                    DAT_10868618[fish].Set(0.0f, 1.0f, 0.0f, 0.0f);
                }
                Vec4 playerPos = DAT_10db6ee8[0].matrix.GetTranslation();
                if ((playerPos - pos).Length() < FISH_FLEE_START)
                    DAT_1021a1c0[fish] = 3;
                if (DAT_1021a1c0[fish] == 1)
                {
                    if (FUN_1013f880() % 1000 < 10)
                    {
                        DAT_1021a1c0[fish] = 2;
                        DAT_10daa880[fish].Set((float)(FUN_1013f880() % 200), 0.0f, 0.0f, 0.0f);
                        DAT_10daa880[fish].RotateZ((float)FUN_1013f880() / 100.0f);
                        DAT_10daa880[fish].RotateX((float)FUN_1013f880() / 100.0f);
                        DAT_10daa880[fish].Copy(pos + DAT_10daa880[fish]);
                        if (DAT_10daa880[fish].z > -FISH_DEPTH_MIN)
                            DAT_10daa880[fish].z = -FISH_DEPTH_MIN;
                        if (DAT_10daa880[fish].z < -FISH_DEPTH_MAX)
                            DAT_10daa880[fish].z = -FISH_DEPTH_MAX;
                    }
                    swim.x = (float)sin((double)((float)(fish * DAT_104bf890) / 100.0f));
                    swim.y = (float)sin((double)((float)(fish * DAT_104bf890 + 0x96) / 100.0f));
                    swim.z = (float)cos((double)((float)(fish * DAT_104bf890 + 0x122) / 100.0f));
                    swimming = 0;
                }
                if (DAT_1021a1c0[fish] == 2)
                {
                    swim.Copy(DAT_10daa880[fish] - pos);
                    if (swim.Length() < FISH_ARRIVE_DISTANCE)
                        DAT_1021a1c0[fish] = 1;
                    swim.Normalized3();
                    swim.Copy(swim * FISH_SWIM_SPEED);
                    DAT_1042d958[fish] = DAT_1042d958[fish] + 1;
                    if (DAT_1042d958[fish] > FISH_WAVE_FRAMES - 1)
                        DAT_1042d958[fish] = DAT_1042d958[fish] - FISH_WAVE_FRAMES;
                    swimming = 1;
                }
                if (DAT_1021a1c0[fish] == 3)
                {
                    if ((playerPos - pos).Length() > FISH_FLEE_END)
                        DAT_1021a1c0[fish] = 1;
                    swim.Copy(pos - playerPos);
                    swim.z = swim.z * 0.1f;
                    swim.Normalized3();
                    swim.Copy(swim * FISH_FLEE_SPEED);
                    DAT_1042d958[fish] = DAT_1042d958[fish] + 1;
                    if (DAT_1042d958[fish] > FISH_WAVE_FRAMES - 1)
                        DAT_1042d958[fish] = DAT_1042d958[fish] - FISH_WAVE_FRAMES;
                    fleeing = 1;
                    swimming = 1;
                }
                DAT_1042d958[fish] = DAT_1042d958[fish] + 1;
                if (DAT_1042d958[fish] > FISH_WAVE_FRAMES - 1)
                    DAT_1042d958[fish] = DAT_1042d958[fish] - FISH_WAVE_FRAMES;
                Matrix4 swimSpace;
                swimSpace.SetIdentity();
                Vec4 turn;
                Vec4 hitNormal;
                Vec4 newPos;
                float hitDepth;

                // The swim direction is tried straight, then turned by 45 and 90 degrees about each axis, until the
                // fish has room (inside the depth range and clear of the map).
                for (dirTry = 0; dirTry <= FISH_TURN_TRIES; dirTry++)
                {
                    turn.Copy(swim);
                    if (dirTry == 1)
                        turn.RotateZ(FISH_TURN_SMALL);
                    else if (dirTry == 2)
                        turn.RotateZ(-FISH_TURN_SMALL);
                    else if (dirTry == 3)
                    {
                        turn.RotateX(FISH_TURN_SMALL);
                        turn.RotateY(FISH_TURN_SMALL);
                    }
                    else if (dirTry == 4)
                    {
                        turn.RotateX(-FISH_TURN_SMALL);
                        turn.RotateY(-FISH_TURN_SMALL);
                    }
                    else if (dirTry == 5)
                        turn.RotateZ(FISH_TURN_LARGE);
                    else if (dirTry == 6)
                        turn.RotateZ(-FISH_TURN_LARGE);
                    else if (dirTry == 7)
                        turn.RotateX(FISH_TURN_LARGE);
                    else if (dirTry == 8)
                        turn.RotateX(-FISH_TURN_LARGE);
                    else if (dirTry == 9)
                        turn.RotateY(FISH_TURN_LARGE);
                    else if (dirTry == 10)
                        turn.RotateY(-FISH_TURN_LARGE);
                    newPos.Copy(pos + turn);
                    if (newPos.z > -FISH_DEPTH_MIN || newPos.z < -FISH_DEPTH_MAX
                        || DAT_10255f08->Query(&swimSpace, &newPos, FISH_RADIUS, &hitNormal, &hitDepth, 0, 0) != 0)
                        continue;
                    if (swimming != 0)
                    {
                        turn.Normalized3();
                        DAT_10868618[fish].Copy(DAT_10868618[fish] * FISH_COURSE_KEEP + turn * FISH_COURSE_STEER);
                        DAT_10868618[fish].Normalized3();
                        turn.Copy(DAT_10868618[fish]);
                        float yaw = (float)(atan2(-turn.x, -turn.y) + FISH_PI);
                        turn.RotateZ(yaw);
                        float pitch = (float)(atan2(-turn.z, -turn.y) + FISH_PI);
                        DAT_10210f38[fish]->matrix = RotationX(pitch) * RotationZ(yaw + FISH_PI);
                    }
                    DAT_10210f38[fish]->matrix.SetTranslation(newPos);
                    break;
                }
                if (swimming == 0)
                {
                    turn.Normalized3();
                    DAT_10868618[fish].z = DAT_10868618[fish].z * 0.8f;
                    DAT_10868618[fish].Normalized3();
                    turn.Copy(DAT_10868618[fish]);
                    float yaw = (float)(atan2(-turn.x, -turn.y) + FISH_PI);
                    turn.RotateZ(yaw);
                    float pitch = (float)(atan2(-turn.z, -turn.y) + FISH_PI);
                    newPos.Copy(DAT_10210f38[fish]->matrix.GetTranslation());
                    DAT_10210f38[fish]->matrix = RotationX(pitch) * RotationZ(yaw + FISH_PI);
                    DAT_10210f38[fish]->matrix.SetTranslation(newPos);
                }
                if (dirTry != 0 && DAT_1021a1c0[fish] == 2)
                    DAT_1021a1c0[fish] = 1;
                DAT_1042d958[fish] = DAT_1042d958[fish] + fleeing * FISH_WAVE_FRAMES;
            }
        }

        // The swing ("@BurankoRoot", seats "@b1" and "@b2"): the player standing within reach of a seat pumps it. The
        // power DAT_104c024c is level * 10 + a sub-step; the swing plays the motion "00d".."80d" of its level, and a
        // push that fits the 0x60 frame rhythm (DAT_104c0254 = start frame) raises it, a late or early one lets it fade.
        if (DAT_104c0240 != 0)
        {
            int swingFading = 0;
            if (DAT_10db9274 != 0 && DAT_10db9248 > SWING_MIN_HEIGHT)
            {
                Vec4 playerPos = DAT_10db6ee8[0].matrix.GetTranslation();
                Vec4 seatA = DAT_104c0244->worldMatrix.GetTranslation();
                Vec4 seatB = DAT_104c0248->worldMatrix.GetTranslation();
                if ((seatA - playerPos).Length2D() < SWING_REACH && DAT_104c024c == 0)
                {
                    DAT_104c0258 = DAT_104bf8a4;
                    DAT_104c0254 = DAT_104bf8a4;
                    DAT_104c0240->RestartColourPlugs("02d", DAT_104bf8a4);
                    DAT_104c024c = DAT_104c024c + SWING_START_POWER;
                    DAT_104c0250 = DAT_104c024c;
                }
                if ((seatB - playerPos).Length2D() < SWING_REACH && DAT_104c024c == 0)
                {
                    DAT_104c0258 = DAT_104bf8a4;
                    DAT_104c0254 = DAT_104bf8a4 - SWING_HALF_BEAT;
                    DAT_104c0240->RestartColourPlugs("02d", DAT_104c0254);
                    DAT_104c024c = DAT_104c024c + SWING_START_POWER;
                    DAT_104c0250 = DAT_104c024c;
                }
                if (((seatA - playerPos).Length2D() < SWING_REACH && DAT_104c024c != 0
                     && (DAT_104bf8a4 - DAT_104c0254) % SWING_BEAT >= SWING_HALF_BEAT)
                    || ((seatB - playerPos).Length2D() < SWING_REACH && DAT_104c024c != 0
                        && (DAT_104bf8a4 - DAT_104c0254) % SWING_BEAT < SWING_HALF_BEAT))
                {
                    swingFading = 1;
                    if (DAT_104c024c % 10 > 1 && DAT_104c024c % 10 != 9)
                        DAT_104c024c = DAT_104c024c - 1;
                }
                else if (((seatA - playerPos).Length2D() < SWING_REACH && DAT_104c024c != 0
                          && (DAT_104bf8a4 - DAT_104c0254) % SWING_BEAT < SWING_HALF_BEAT)
                         || ((seatB - playerPos).Length2D() < SWING_REACH && DAT_104c024c != 0
                             && (DAT_104bf8a4 - DAT_104c0254) % SWING_BEAT >= SWING_HALF_BEAT))
                {
                    if (DAT_104c024c % 10 < 9)
                        DAT_104c024c = DAT_104c024c + 1;
                }
            }
            if (DAT_104c024c != 0)
            {
                if (DAT_104bf890 % 5 == 0 && DAT_104c024c % 10 > 1)
                    DAT_104c024c = DAT_104c024c - 1;
                if (DAT_104c024c < SWING_LEVEL_STEP)
                {
                    DAT_104c0240->RestartColourPlugs("00d", DAT_104c0254);
                    DAT_104c024c = 0;
                }
                unsigned int swingBeat = (DAT_104bf8a4 - DAT_104c0258) % SWING_HALF_BEAT;
                if (swingBeat == SWING_SAVE_POWER_BEAT)
                    DAT_104c0250 = DAT_104c024c;
                int powerStep = DAT_104c0250 % 10;
                if ((powerStep == 8 || powerStep == 9 || powerStep == 0 || powerStep == 1) && swingBeat == 0)
                {
                    if (swingFading == 0)
                    {
                        if (powerStep == 8)
                            DAT_104c024c = DAT_104c0250 + 4;
                        if (powerStep == 9)
                            DAT_104c024c = DAT_104c0250 + 3;
                    }
                    else if (swingFading == 1)
                    {
                        if (powerStep == 8)
                            DAT_104c024c = DAT_104c0250;
                        if (powerStep == 9)
                            DAT_104c024c = DAT_104c0250;
                    }
                    if (powerStep == 0)
                    {
                        DAT_104c024c = DAT_104c0250 - 3;
                        if (DAT_104c024c < 0)
                            DAT_104c024c = 0;
                    }
                    if (powerStep == 1)
                    {
                        DAT_104c024c = DAT_104c0250 - 4;
                        if (DAT_104c024c < 0)
                            DAT_104c024c = 0;
                    }
                    if (DAT_104c024c / 10 == 0)
                        DAT_104c0240->RestartColourPlugs("00d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 1)
                        DAT_104c0240->RestartColourPlugs("02d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 2)
                        DAT_104c0240->RestartColourPlugs("04d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 3)
                        DAT_104c0240->RestartColourPlugs("06d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 4)
                        DAT_104c0240->RestartColourPlugs("10d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 5)
                        DAT_104c0240->RestartColourPlugs("15d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 6)
                        DAT_104c0240->RestartColourPlugs("20d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 7)
                        DAT_104c0240->RestartColourPlugs("30d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 8)
                        DAT_104c0240->RestartColourPlugs("40d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 9)
                        DAT_104c0240->RestartColourPlugs("50d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 10)
                        DAT_104c0240->RestartColourPlugs("60d", DAT_104c0254);
                    if (DAT_104c024c / 10 == 11)
                        DAT_104c0240->RestartColourPlugs("70d", DAT_104c0254);
                    if (DAT_104c024c / 10 > 11)
                        DAT_104c0240->RestartColourPlugs("80d", DAT_104c0254);
                }
            }
        }
        // The player's scene state DAT_104c00d0 (non-zero: the player cannot be pushed). 1 = ends when the counter
        // DAT_104c00d4 is 0; the others count down through timelines (below SCENE_TIMELINE_LIMIT, 1000..1999,
        // 2000..4999, 5000..10000) or run until they finish (SCENE_SHOVE_WAIT.., SCENE_SWAN_WAIT..).
        if (DAT_104c01a0 != 0)
            DAT_104c01a0 = DAT_104c01a0 - 1;
        if (DAT_104c00d4 == 0 && DAT_104c00d0 == 1)
            DAT_104c00d0 = 0;
        else if (DAT_104c00d0 > 1)
        {
            if (DAT_104c00d0 < SCENE_SWAN_WAIT)
            {
                if (DAT_104c00d0 < SCENE_SHOVE_WAIT)
                {
                    if (DAT_104c00d0 < SCENE_DAMAGE_LIMIT)
                    {
                        if (DAT_104c00d0 < SCENE_HAND_LIMIT)
                        {
                            if (DAT_104c00d0 < SCENE_SPARK_LIMIT)
                            {
                                if (DAT_104c00d0 < SCENE_BLANK_LIMIT)
                                {
                                    if (DAT_104c00d0 < SCENE_TIMELINE_LIMIT)
                                    {
                                        // The timeline counts down from SCENE_TIMELINE_START to 1.
                                        if (DAT_104c00d0 == SCENE_TIMELINE_START)
                                        {
                                            CloseEventOverlay();
                                            DAT_10db6ee8[0].m_0240 = 0;
                                            FUN_1003ad20(0x35e, 0, 1.0f);
                                            Node *seat = (Node *)DAT_104c00d8;
                                            if (DAT_104c00d8 != 0)
                                                seat->RestartColourPlugs("ON", DAT_104bf8a4);
                                            Vec4 effectAt;
                                            effectAt.Fill(0.0f);
                                            DAT_10daaa80.Start(0xfaa, effectAt, -1, 0, 0, 0, 0, &seat->worldMatrix, 0, 0);
                                            DAT_10daaa80.Start(0x5015, effectAt, -1, 0, 0, 0, 10, &seat->worldMatrix, 0, 0);
                                            DAT_10edbdd8.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                                            DAT_10db6ee8[0].m_0294 = 0;
                                            DAT_10db6ee8[0].motionFrame = 0;
                                            DAT_10db6ee8[0].UpdateFrame(&DAT_108392d8, 1, 0, 0, 0);
                                            DAT_10db6ee8[0].PlayMotion(2, 0, 1, 1.0f);
                                            if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                            {
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].LoadFile(FUN_1013f8a0("char02bn.it3", 0));
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].m_0294 = 0;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].motionFrame = 0;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].UpdateFrame(&DAT_108392d8, 1, 0, 0, 0);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].matrix = DAT_10db6ee8[0].matrix;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].heading = DAT_10db6ee8[0].heading;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].targetHeading = DAT_10db6ee8[0].targetHeading;
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].m_17e8 = 1;
                                                DAT_10804bf8.AddLitModel(&DAT_10db6ee8[SCENE_DOUBLE_MODEL]);
                                                DAT_10db6ee8[0].root->M_10193cb0(0x21, 0x31);
                                                DAT_10db6ee8[0].root->M_10193cb0(0x1f, 0x32);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x21, 0x2f);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x1f, 0x30);
                                                FUN_1013faf0();
                                            }
                                            DAT_10edbdd8.z = DAT_10edbdd8.z + 40.0f;
                                            DAT_10f65598 = DAT_10edbdd8.z - 60.0f;
                                            DAT_10836c70.ResetFrameTimer();
                                        }
                                        if (DAT_104c00d0 == SCENE_TIMELINE_EFFECT)
                                        {
                                            FUN_1000f360(0, 3);
                                            Node *seat = (Node *)DAT_104c00d8;
                                            Vec4 effectAt;
                                            effectAt.Fill(0.0f);
                                            DAT_10daaa80.Start(0x5019, effectAt, -1, 0, 0, 0, 0, &seat->worldMatrix, 0, 0);
                                        }
                                        if (DAT_104c00d0 == SCENE_TIMELINE_VOICE)
                                        {
                                            Vec4 voiceAt = DAT_10db6ee8[0].matrix.GetTranslation();
                                            int voice = FUN_1013f880();
                                            if (DAT_104c0332 == 0)
                                                FUN_1003ad20(voice % 3 + 0x8f, &voiceAt, 1.0f);
                                            Node *seat = (Node *)DAT_104c00d8;
                                            Vec4 effectAt;
                                            effectAt.Fill(0.0f);
                                            DAT_10daaa80.Start(0x5016, effectAt, -1, 0, 0, 0, 0, &seat->worldMatrix, 0, 0);
                                            DAT_10daaae8[0].stats[STAT_HEALTH] = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
                                            FUN_1000e750();
                                        }
                                        if (DAT_104c00d0 == SCENE_TIMELINE_SHADE)
                                        {
                                            DAT_10f65598 = DAT_10edbdd8.z - 60.0f;
                                            DAT_10db6ee8[0].root->M_10193cb0(0x31, 0x2f);
                                            DAT_10db6ee8[0].root->M_10193cb0(0x32, 0x30);
                                            if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                            {
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x2f, 0x31);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x30, 0x32);
                                            }
                                        }
                                        DAT_10f65598 = DAT_10f65598 + 3.0f;
                                        if (DAT_104c00d0 == SCENE_TIMELINE_RELEASE)
                                        {
                                            DAT_104c0074 = 0;
                                            DAT_104bfdc8 = 0;
                                            DAT_104bfdcc = 0;
                                            DAT_104bfdd0 = 0;
                                            DAT_104c0090 = 1;
                                            FUN_10016500(0, "mt_refre.it3", 0, 200, 1);
                                            if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                                FUN_10016500(SCENE_DOUBLE_MODEL, "mt_refre.it3", 0, 200, 1);
                                            DAT_10836c70.ResetFrameTimer();
                                        }
                                        Vec4 playerAt = DAT_10db6ee8[0].matrix.GetTranslation();
                                        playerAt.Copy(playerAt + (DAT_10edbdd8 - playerAt) * 0.1f);
                                        DAT_10db6ee8[0].matrix.SetTranslation(playerAt);
                                        if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                        {
                                            DAT_10db6ee8[SCENE_DOUBLE_MODEL].matrix = DAT_10db6ee8[0].matrix;
                                            DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
                                            DAT_10db6ee8[SCENE_DOUBLE_MODEL].heading = DAT_10db6ee8[0].targetHeading;
                                            DAT_10db6ee8[SCENE_DOUBLE_MODEL].targetHeading = DAT_10db6ee8[0].targetHeading;
                                            DAT_10db6ee8[SCENE_DOUBLE_MODEL].m_0267 = 1;
                                        }
                                        DAT_104c00d0 = DAT_104c00d0 - 1;
                                        if (DAT_104c00d0 == 1)
                                        {
                                            DAT_10db6ee8[0].m_0240 = 1;
                                            DAT_10db6ee8[DAT_104c0030].motionFrame = 0;
                                            DAT_10db6ee8[DAT_104c0030].M_10161b60();
                                            FUN_100121e0(0);
                                            DAT_10db6ee8[DAT_104c0030].m_0294 = 0;
                                            DAT_10db6ee8[0].PlayMotion(2, 0, 5, 1.0f);
                                            if (DAT_10c655f0 != COSTUME_THREE_POINTERS)
                                            {
                                                DAT_10db6ee8[0].root->M_10193cb0(0x2f, 0x21);
                                                DAT_10db6ee8[0].root->M_10193cb0(0x30, 0x1f);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].SetPosition(FISH_HIDDEN_X, 0.0f, 0.0f);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x31, 0x21);
                                                DAT_10db6ee8[SCENE_DOUBLE_MODEL].root->M_10193cb0(0x32, 0x1f);
                                            }
                                            FUN_1013faf0();
                                        }
                                    }
                                }
                                else
                                {
                                    // 1000..1999: a spark effect at the player while the counter is in 1005..1049.
                                    CloseEventOverlay();
                                    if (DAT_104c00d0 < SCENE_SPARK_END && DAT_104c00d0 > SCENE_SPARK_START)
                                    {
                                        EffectOptions options;
                                        int strength = DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
                                        if ((DAT_104c02c8 & HELMET_FACE_400) != 0)
                                            options.m_0004 = (int)((float)strength * 2.0f);
                                        else if ((DAT_104c02c8 & HELMET_FACE_300) == HELMET_FACE_300)
                                            options.m_0004 = (int)((float)strength * 1.7f);
                                        else if ((DAT_104c02c8 & HELMET_FACE_200) != 0)
                                            options.m_0004 = (int)((float)strength * 1.5f);
                                        else if ((DAT_104c02c8 & HELMET_FACE_100) != 0)
                                            options.m_0004 = (int)((float)strength * 1.2f);
                                        else
                                            options.m_0004 = strength;
                                        options.m_0008 = 1.0f;
                                        options.m_001c = 1;
                                        options.m_0000 = 1;
                                        options.settings[4] = 0;
                                        options.m_000e = 5;
                                        options.settings[5] = 1;
                                        options.settings[0] = 0x19;
                                        options.settings[1] = 0xc;
                                        Vec4 effectAt;
                                        effectAt.Fill(0.0f);
                                        Matrix4 playerMatrix;
                                        playerMatrix = DAT_10db6ee8[0].matrix;
                                        DAT_10daaa80.Start(0x8ec8, effectAt, -1, 0, 0, 0, 0, &playerMatrix, &options, 0);
                                    }
                                    DAT_104c00d0 = DAT_104c00d0 - 1;
                                    if (DAT_104c00d0 == SCENE_SPARK_FINISH)
                                        DAT_104c00d0 = 0;
                                }
                            }
                            else
                            {
                                // 2000..4999: the left hand glows while the counter is in 2020..2053.
                                DAT_104c00d0 = DAT_104c00d0 - 1;
                                CloseEventOverlay();
                                if (DAT_104c00d0 < SCENE_HAND_END && DAT_104c00d0 > SCENE_HAND_START)
                                {
                                    DAT_1101de30 = 1;
                                    DAT_1020344c = 0.6f;
                                    Node *hand = DAT_10db6ee8[0].root->Find("Bip01 L Hand", 0);
                                    Vec4 effectAt;
                                    effectAt.Fill(0.0f);
                                    Matrix4 handMatrix;
                                    handMatrix = DAT_10db6ee8[0].matrix;
                                    if (hand != 0)
                                        handMatrix.SetTranslation(hand->worldMatrix.GetTranslation());
                                    DAT_10daaa80.Start(0x8f02, effectAt, -1, 0, 0, 0, 0, &handMatrix, 0, 0);
                                }
                                if (DAT_104c00d0 == SCENE_HAND_FINISH)
                                {
                                    DAT_1020344c = 1.0f;
                                    if (DAT_104c01b0 == 0)
                                        FUN_10001b70((10 - DAT_102027d8) * 500);
                                    DAT_104687b0 = 1;
                                    DAT_104c00d0 = 0;
                                }
                            }
                        }
                        else
                        {
                            // 5000..10000: a damage state (the player takes damage every time the counter reaches
                            // SCENE_DAMAGE_TICK) and its recovery.
                            CloseEventOverlay();
                            DAT_104c00d0 = DAT_104c00d0 - 1;
                            if (DAT_104c00d0 == SCENE_DAMAGE_TICK)
                            {
                                DAT_104c00d0 = 0;
                                unsigned int damage = 5;
                                if (DAT_10c635f8 == 1)
                                    damage = 2;
                                else if (DAT_10c635f8 == 2)
                                {
                                    if (DAT_10c66cc0 == 1)
                                        damage = 0x14;
                                    else if (DAT_10c66cc0 == 0)
                                        damage = 10;
                                    else
                                        damage = 0x32;
                                }
                                if ((DAT_104c02c8 & HELMET_NO_DAMAGE) == 0)
                                {
                                    if ((DAT_104c02c8 & HELMET_HALF_DAMAGE) != 0)
                                        damage = damage / 2;
                                }
                                else
                                {
                                    FUN_10011aa0(0, 1);
                                    damage = 0;
                                }
                                FUN_1005dbf0(damage);
                                DAT_104c00d0 = SCENE_DAMAGE_TICK;
                                if (DAT_10daaae8[0].stats[STAT_HEALTH] > 0)
                                    FUN_1000da40(0, 0, 0x1e);
                            }
                            else
                            {
                                if (DAT_104c00d0 == SCENE_DAMAGE_RECOVER)
                                {
                                    DAT_104c0190 = 0;
                                    DAT_104c0108 = 0;
                                    DAT_104bfe70 = 0;
                                    FUN_1000ec70(5, 1.0f, 1.0f, 1.0f, 1.0f);
                                    ResetPlayerTrail();
                                    DAT_104687b0 = 1;
                                    DAT_10db9274 = 1;
                                    DAT_10db6ee8[0].Teleport(DAT_10daa400[DAT_1030c274].x, DAT_10daa400[DAT_1030c274].y,
                                                             DAT_10daa400[DAT_1030c274].z);
                                    DAT_10db6ee8[0].PlayMotion(2, 1, 5, 1.0f);
                                    FUN_1016a8e0();
                                    DAT_11018734 = 1;
                                    DAT_1101de30 = 1;
                                }
                                if (DAT_104c00d0 == SCENE_DAMAGE_END)
                                    DAT_104c00d0 = 0;
                            }
                        }
                    }
                }
                else
                {
                    // 20000..29999: the player is shoved aside: first wait until he stands still, then push him
                    // along the first of four directions that is clear, with growing strength.
                    if (DAT_104c00d0 == SCENE_SHOVE_WAIT)
                    {
                        if (DAT_10db6ee8[0].velocity.Length2D() >= 1.0f || DAT_10db9274 == 0)
                            DAT_104c00d0 = 0;
                        else
                        {
                            DAT_104c00d0 = SCENE_SHOVE;
                            DAT_10314eb8 = 0.2f;
                            DAT_10868828.Fill(0.0f);
                            ResetPlayerTrail();
                        }
                    }
                    if (DAT_104c00d0 == SCENE_SHOVE)
                    {
                        Vec4 shoveDirs[4];
                        Vec4 shoveFrom;
                        shoveDirs[0].Set(SHOVE_SIDE, 0.0f, SHOVE_DOWN, 0.0f);
                        shoveDirs[1].Set(-SHOVE_SIDE, 0.0f, SHOVE_DOWN, 0.0f);
                        shoveDirs[2].Set(0.0f, SHOVE_SIDE, SHOVE_DOWN, 0.0f);
                        shoveDirs[3].Set(0.0f, -SHOVE_SIDE, SHOVE_DOWN, 0.0f);
                        shoveFrom.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                        for (i = 0; i < 4; i++)
                        {
                            Vec4 shoveDir;
                            shoveDir.Copy(shoveDirs[i]);
                            shoveDirs[i].Copy(shoveDirs[i] * DAT_10314eb8);
                            if (DAT_10db6ee8[0].IsClearOfMapLimited(&shoveDirs[i]) != 0)
                            {
                                DAT_10db6ee8[0].important = 1;
                                DAT_10db6ee8[0].PlayMotion(0x67, 1, 8, 1.0f);
                                DAT_10314eb8 = DAT_10314eb8 + 0.05;
                                if (DAT_10314eb8 >= 4.0f)
                                    DAT_10314eb8 = 4.0f;
                                DAT_10868828.Copy(shoveDir);
                                DAT_10db6ee8[0].Push(shoveDirs[i].x, shoveDirs[i].y, 0.0f);
                                shoveFrom.Copy(shoveFrom + shoveDirs[i]);
                                DAT_10db6ee8[0].FaceAway(shoveFrom);
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
                                goto sceneStateDone;
                            }
                        }
                        DAT_10314eb8 = DAT_10314eb8 - 0.2f;
                        Vec4 shove;
                        shove.Copy(DAT_10868828 * DAT_10314eb8);
                        DAT_10db6ee8[0].Push(shove.x, shove.y, 0.0f);
                        if (DAT_10314eb8 < 1.05)
                        {
                            DAT_104c00d0 = 0;
                            DAT_10db6ee8[0].PlayMotion(2, 1, 8, 1.0f);
                        }
                    }
                }
            }
            else
            {
                // From 30000: the swan ride. The player follows the swan node ("swanat") and turns with the swan.
                CloseEventOverlay();
                if (DAT_104c00d0 == SCENE_SWAN_WAIT)
                {
                    ResetPlayerTrail();
                    DAT_10db6ee8[0].PlayMotion(0x68, 1, 8, 1.0f);
                    DAT_104c00d0 = SCENE_SWAN;
                    DAT_104c0260 = 0;
                    if (DAT_104c025c == 1)
                    {
                        DAT_104c0268->RestartColourPlugs("A", DAT_104bf8a4);
                        DAT_104c025c = 0;
                    }
                    else
                    {
                        DAT_104c0268->RestartColourPlugs("B", DAT_104bf8a4);
                        DAT_104c025c = 1;
                    }
                }
                DAT_10db6ee8[0].SetPosition(DAT_104c0264->worldMatrix._41, DAT_104c0264->worldMatrix._42,
                                            DAT_104c0264->worldMatrix._43);
                Matrix4 swanMatrix;
                swanMatrix = DAT_104c0268->worldMatrix;
                swanMatrix._41 = 0.0f;
                swanMatrix._42 = 0.0f;
                swanMatrix._43 = 0.0f;
                float swanHeading = swanMatrix.M_101b0a80();
                DAT_104c0260 = DAT_104c0260 + 1;
                DAT_10db6ee8[0].targetHeading = swanHeading - SWAN_HEADING_OFFSET;
                DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
                if (DAT_104c0260 > SWAN_RIDE_FRAMES)
                {
                    DAT_104c025d = 0x1e;
                    DAT_104c00d0 = 0;
                    DAT_10db6ee8[0].PlayMotion(2, 1, 8, 1.0f);
                }
                if (DAT_104c0260 % SWAN_SOUND_PERIOD == 0)
                    FUN_10065b10(0, 0x3b5, 0, 0);
            }
        }
sceneStateDone:
        // Caption scroll timer: counts up every fifth frame and wraps after the scroll range.
        if (DAT_104bf890 % 5 == 0)
        {
            DAT_104c02cc++;
            if (DAT_104c02cd + 10 < DAT_104c02cc)
                DAT_104c02cc = 0;
        }
        DAT_104c00d5 = DAT_104c00d4;
        if (DAT_104c00d4 != 0)
            DAT_104c00d4--;

        // Doors (switch models of kind 0x13/0x14): the animation frame in DAT_1020b128 triggers the motions.
        AnimModel *door = &DAT_10db6ee8[DOOR_FIRST_MODEL];
        for (int doorSlot = DOOR_FIRST_MODEL; door < &DAT_10db6ee8[DOOR_END_MODEL]; doorSlot++, door++)
        {
            if (DAT_1021f3c0[doorSlot] != 0x13 && DAT_1021f3c0[doorSlot] != 0x14)
                continue;
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_START)
            {
                Vec4 facing;
                Vec4 toPlayer;
                facing.Set(0.0f, 1.0f, 0.0f, 0.0f);
                facing.RotateZ(3.1415f - door->targetHeading);
                toPlayer.Copy(DAT_10db6ee8[0].matrix.GetTranslation() - door->matrix.GetTranslation());
                toPlayer.Normalized3();
                float side = Vec3_Dot(&facing, &toPlayer);
                if (DoorSwingsEffect(door->m_0284))
                    StartDoorEffect(door);
                if (side <= 0.0f)
                {
                    DAT_1021b3c8[doorSlot] = 1;
                    door->root->RestartColourPlugs("OPEN1", DAT_104bf8a4);
                }
                else
                {
                    DAT_1021b3c8[doorSlot] = 0;
                    door->root->RestartColourPlugs("OPEN0", DAT_104bf8a4);
                }
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_SETTLE)
                DAT_1020b128[doorSlot] = 0;
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_OPENED)
            {
                if (DoorSwingsEffect(door->m_0284))
                    StartDoorEffect(door);
                door->root->RestartColourPlugs("OPEN0", DAT_104bf8a4);
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_OPEN_LOOP)
            {
                DAT_1020b128[doorSlot] = 0;
                door->root->RestartColourPlugs("OPEN", DAT_104bf8a4);
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_CLOSING)
            {
                door->root->RestartColourPlugs(DAT_1021b3c8[doorSlot] == 0 ? "CLOSE0" : "CLOSE1", DAT_104bf8a4);
                if (DoorSwingsEffect(door->m_0284))
                    StartDoorEffect(door);
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_CLOSE_SETTLE)
                DAT_1020b128[doorSlot] = 0;
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_CLOSED)
            {
                door->root->RestartColourPlugs("CLOSE0", DAT_104bf8a4);
                if (door->m_0284 == 0xcd || door->m_0284 == 0xce)
                    StartDoorEffect(door);
            }
            if (DAT_1020b128[doorSlot] == DOOR_FRAME_END)
            {
                DAT_1020b128[doorSlot] = 0;
                door->root->RestartColourPlugs("CLOSE", DAT_104bf8a4);
            }
            if (DAT_1020b128[doorSlot] != 0)
                DAT_1020b128[doorSlot]++;
        }

        // Per-tick updates of the effects, windows, messages, fades and the other systems.
        DAT_10daaa80.Update((DAT_104bfe1c == 0 && DAT_104c0135 == 0) ? 0 : 1,
                            (tick == ticks - 1 || DAT_1065cb90 != 0) ? 1 : 0, ~DAT_104bf890 & 1);
        if (DAT_104bf8d4 != 0)
            FUN_1000cbe0();
        FUN_1000ed20();
        FUN_1003a6c0();
        FUN_1000f320();
        FUN_100f5a80();
        TutorialBook_Update();
        FUN_10058500();
        if (DAT_104bfe1c == 0)
        {
            FUN_100564d0();
            FUN_1000d670();
            FUN_1015f4e0();
        }
        FUN_1000f490();
        FUN_1003a9d0();
        if (DAT_104bfd28 != 0)
            FUN_1000ec10();
        FUN_1000cdf0();
        if (DAT_104bf8c8 != 0)
            FUN_1000ee60();
        if (DAT_104bff00 == 0)
        {
            if (DAT_104bff0c > 0)
                DAT_104bff0c--;
        }
        else if (DAT_104bff0c < MESSAGE_FADE_MAX)
            DAT_104bff0c++;
        FUN_1003a940();
        FUN_1005c680();
        if (DAT_104bfd78 != 0)
        {
            if (DAT_104bfd7c == 1 && (DAT_104bfd78 += 2, DAT_104bfd78 > MESSAGE_SLIDE_FULL - 1))
            {
                DAT_104bfd78 = MESSAGE_SLIDE_FULL;
                DAT_104bfd7c = 0;
            }
            else if (DAT_104bfd78 == 0 || DAT_104bfd7c != 2 || (DAT_104bfd78 -= 2, DAT_104bfd78 > 0))
                ;
            else
            {
                DAT_104bfd78 = 0;
                DAT_104bfd7c = 0;
            }
        }
        DAT_104bf88c = DAT_104bf88c + DAT_10202348;
        if (DAT_104bf88c == 10 || DAT_104bf88c == 0)
            DAT_10202348 = -DAT_10202348;
        DAT_1020234c = 0.5f;
        if (DAT_104c00bd != 0 && (DAT_104c00bd++, DAT_104c00bd > 0x3b))
            DAT_104c00bd = 0;

        // Dig: DAT_104bfd60 counts the frames of the dig (long when DAT_104bfd64 is set); it digs the ground in
        // front of the player once and then waits for the end.
        if (DAT_104bfd60 != 0)
        {
            int digStart = DAT_104bfd64 * DIG_LONG_FRAMES + DIG_START_FRAME;
            if (DAT_104bfd60 < digStart && DAT_104bfd64 != 0 && DAT_10db9274 != 0)
            {
                DAT_10db6ee8[0].verticalSpeed = DAT_104bfd68;
                DAT_104bfd60 = digStart;
            }
            if (DAT_10db6ee8[0].verticalSpeed < DAT_104bfd68)
                DAT_104bfd68 = DAT_10db6ee8[0].verticalSpeed;
            if (DAT_104bfd60 == digStart)
            {
                DAT_104bfd60++;
                Vec4 digger = DAT_10db6ee8[0].matrix.GetTranslation();
                DAT_104bfdb0 = 0;
                FUN_1003ad20(0x35f, &DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                int digRadius = DAT_104bfd64 == 0 ? DIG_RADIUS_SHORT : DIG_RADIUS_LONG;
                FUN_1003bc60((int)(cos(DIG_HEADING) * DIG_REACH + (digger.x - DAT_10edbd38.x * 2.0f)),
                             (int)(sin(DIG_HEADING) * DIG_REACH + (digger.y - DAT_10edbd38.y * 2.0f)), 1, digRadius);
                if (DAT_104bfd64 != 0)
                    DAT_104bfd60 = DAT_104bfd64 * DIG_LONG_FRAMES + DIG_END_FRAME;
            }
            if (DAT_104bfd60 == DAT_104bfd64 * DIG_LONG_FRAMES + DIG_END_FRAME)
            {
                DAT_104c0091 = 8;
                Vec4 digger = DAT_10db6ee8[0].matrix.GetTranslation();
                IntRect area;
                area.left = (int)(cos(DIG_HEADING) * DIG_SHADE_OFFSET + (digger.x - DAT_10edbd38.x * 2.0f) - DIG_SHADE_SIZE);
                area.top = (int)(sin(DIG_HEADING) * DIG_SHADE_OFFSET + (digger.y - DAT_10edbd38.y * 2.0f) - DIG_SHADE_SIZE);
                area.right = (int)(cos(DIG_HEADING) * DIG_SHADE_OFFSET + (digger.x - DAT_10edbd38.x * 2.0f) + DIG_SHADE_SIZE);
                area.bottom = (int)(sin(DIG_HEADING) * DIG_SHADE_OFFSET + (digger.y - DAT_10edbd38.y * 2.0f) + DIG_SHADE_SIZE);
                FUN_10016ec0(&area, -1, 1);
            }
            if (DAT_104bfd60 == DAT_104bfd64 * DIG_LONG_FRAMES + DIG_DONE_FRAME)
                DAT_104bfd60 = 0;
            else if (DAT_104bfd60 != 0)
                DAT_104bfd60++;
        }

        // Event markers (DAT_1020d660): age them and project their world position to the 640x480 screen.
        for (int marker = 0; marker < EVENT_MARKER_COUNT; marker++)
        {
            if (DAT_1020d660[marker] == 0)
                continue;
            if (DAT_104463f8[marker] < EVENT_MARKER_FRAMES)
                DAT_104463f8[marker]++;
            else
                DAT_1020d660[marker] = 0;
            Vec4 markerAt;
            int markerY;
            if (DAT_1021f640[marker] == 3)
            {
                if (DAT_104463f8[marker] < 2)
                {
                    markerAt.Copy(DAT_10db6ee8[0].headNode->worldMatrix.GetTranslation());
                    markerAt.z += MARKER_HEAD_LIFT;
                    markerAt.Copy(Project(markerAt, DAT_108392d8));
                    DAT_10253c58[marker] = (int)((markerAt.x + 1.0f) * SCREEN_HALF_WIDTH);
                    DAT_10256218[marker] = (int)(SCREEN_HEIGHT - (markerAt.y + 1.0f) * SCREEN_HALF_HEIGHT);
                }
                markerY = DAT_10256218[marker] - 2;
            }
            else
            {
                int target = DAT_10447fd8[marker];
                if (DAT_1021f3c0[target] == 3)
                {
                    markerAt.Copy(DAT_1020e338[target]->GetTranslation());
                    if (markerAt.x <= MARKER_OFFSCREEN_X)
                        continue;
                }
                else if (DAT_10db6ee8[target].root == 0)
                    markerAt.Copy(DAT_10db6ee8[target].matrix.GetTranslation());
                else
                    markerAt.Copy(DAT_10db6ee8[target].root->worldMatrix.GetTranslation());
                markerAt.Copy(Project(markerAt, DAT_108392d8));
                DAT_10253c58[marker] = (int)((markerAt.x + 1.0f) * SCREEN_HALF_WIDTH);
                markerY = (int)(SCREEN_HEIGHT - (markerAt.y + 1.0f) * SCREEN_HALF_HEIGHT);
            }
            DAT_10256218[marker] = markerY;
        }

        // Movies: DAT_104bfe6c 3 = DirectShow movie, 2 = movie library frames; DAT_104bfe70 = the game over movie.
        if (DAT_104bfe6c == 3)
        {
            if (DAT_10836e98.M_10148930() == 0)
            {
                DAT_10836e98.M_10148960();
                DAT_10836e98.M_1015c6a0(&DAT_118d0ba8[DAT_10202478]);
            }
            else
            {
                DAT_10836e98.M_10148920();
                DAT_10836e98.Close();
                DAT_104bfe6c = 0;
            }
        }
        if (DAT_104bfe6c == 2 && (DAT_104bf890 & 1) != 0)
        {
            if (tick == ticks - 1 || tick == ticks - 2)
            {
                if (FUN_101b25a0(0) == 0)
                    DAT_104bfe6c = 0;
                else
                    DAT_118d0ba8[0].Write16(0, 0, DAT_11b1bb58, DAT_11b1bb64, DAT_11b1bb58 * 2, DAT_11b1bb54);
            }
            else if (FUN_101b25a0(2) == 0)
                DAT_104bfe6c = 0;
        }
        if (DAT_104bfe70 != 0)
        {
            if ((DAT_104bfe70 & 1) == 0)
                DAT_10f08ab0.M_10161050();
            DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
            DAT_104bfe70++;
            if (DAT_10f08ab0.m_004b == 0)
                DAT_104bfe70 = 0;
        }

        // Game over: the player fell out of the world (or the game over movie is running).
        if (DAT_104bfe6c == 0)
        {
            if (DAT_1032d69c != 1 || -3450.0f <= DAT_10db9248 || DAT_104bfe78 != 0)
            {
                if (DAT_104bfe78 < GAME_OVER_END_FRAME)
                {
                    if (DAT_104bfe78 != 0)
                        goto gameOverStep;
                }
                else
                    DAT_104bfe78 = 0;
                goto waterThaw;
            }
            DAT_10f08ab0.Open(FUN_1013f8a0("gameover.mmv", 0));
            DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
            FUN_1003ad20(0x335, &DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
            DAT_10f08ab0.state = MMV_PLAYING;
            DAT_10f08ab0.loop = 0;
            FUN_1000da40(0, 100, 0x28);
            DAT_10db6ee8[0].PlayMotion(0x84, 1, 5, 1.0f);
            DAT_104bfe78 = 1;
            if (DAT_104bfe6c == 0)
            {
gameOverStep:
                DAT_104bfe70 = 0;
                gameOverButton = 0;
                gameOverPadA = 0;
                gameOverPadB = 0;
                if (DAT_104bfe78 == 2)
                {
                    FUN_1013fa10(0, 10, 1.0f, 1.0f, 1.0f, 1.0f);
                    FUN_1013fa10(1, 10, 0.0f, 0.0f, 0.0f, 1.0f);
                    FUN_1000ec70(10, 1.0f, 1.0f, 1.0f, 1.0f);
                }
                if (DAT_104bfe78 == GAME_OVER_FADE_FRAME)
                {
                    if (DAT_104bf8e8 < 100.0f)
                        FUN_1000da40(0, 100, 0x14);
                    goto gameOverCount;
                }
                if (DAT_104bfe78 == GAME_OVER_SOUND_FRAME)
                {
                    if (DAT_104c0332 != 0)
                        goto gameOverCount;
                    FUN_1003ad20((DAT_104c033a + 10) * 0xf, 0, 0.8f);
                }
                if (DAT_104bfe78 < GAME_OVER_END_FRAME2)
                {
gameOverCount:
                    DAT_104bfe78++;
                }
                if ((DAT_104bfe78 & 1) == 0)
                    DAT_10f08ab0.M_10161050();
                DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
                DAT_1020d9e0.M_10001450();
                DAT_1020d9e0.M_10001110(gameOverPos, &gameOverButton, &gameOverPadA, &gameOverPadB, 0, 0);
                if (DAT_104bfe78 > GAME_OVER_SKIP_FRAME && (DAT_1065cb30 != 0 || gameOverButton == 2))
                {
                    // Skipped: back to the map's start point with fresh health.
                    int gameOvers;
                    FUN_1000da40(0, 0, 0x1e);
                    if (DAT_104c01e0 != 0)
                    {
                        DAT_11018734 = 1;
                        DAT_1101de30 = 1;
                    }
                    DAT_104c01e0 = 0;
                    DAT_104c0190 = 0;
                    DAT_104c0108 = 0;
                    FUN_1000ec70(5, 1.0f, 1.0f, 1.0f, 1.0f);
                    ResetPlayerTrail();
                    DAT_10c62ed0 = DAT_1030b660;
                    DAT_10db6ee8[0].Teleport(DAT_108029b8.x, DAT_108029b8.y, DAT_108029b8.z);
                    DAT_10db9364 = DAT_108029b8.w;
                    DAT_10db9368 = (int)DAT_108029b8.w;
                    DAT_104bfe4c = 0;
                    DAT_104bfe50 = 0;
                    DAT_104c00b0 = 0;
                    DAT_102024bc = -1;
                    DAT_104bfe30 = 0;
                    DAT_10db6ee8[0].PlayMotion(5, 1, 0, 1.0f);
                    DAT_10db6ee8[0].PlayMotion(2, 1, 0, 1.0f);
                    DAT_104686b0[PLAYER_SLOT] = 0;
                    DAT_10db7c34 = 1.0f;
                    DAT_10db9380 = 1;
                    DAT_104bfeb0 = 0;
                    DAT_104bfeb4 = 0;
                    FUN_1013fb10(0, 0, 0);
                    gameOvers = DAT_10c64000;
                    DAT_104c0128 = 0;
                    DAT_10253e98[PLAYER_SLOT] = 0;
                    if (DAT_10c635f8 == 0)
                        DAT_10daaaec = DAT_10daaaf0 / 2;
                    else if (DAT_10c635f8 == 1)
                        DAT_10daaaec = DAT_10daaaf0;
                    else
                    {
                        int divisor;
                        if (DAT_10c635f8 != 2)
                            goto gameOverResetDone;
                        if (DAT_10c66cc0 == 1)
                            divisor = DAT_10c64000 + 3;
                        else
                        {
                            divisor = DAT_10c64000 * 2 + 4;
                            if (DAT_10c66cc0 == 0)
                                divisor = DAT_10c64000 + 2;
                        }
                        DAT_10daaaec = DAT_10daaaf0 / divisor;
                        if (DAT_10daaaec < 1)
                            DAT_10daaaec = 1;
                    }
                    FUN_1000e750();
gameOverResetDone:
                    DAT_1021cff8 = 1;
                    DAT_103086a8 = 0;
                    DAT_1030c900 = 0;
                    DAT_1042de20 = DAT_1030b660;
                    DAT_104bfe78 = GAME_OVER_RESTART_FRAME;
                    DAT_10c62ed0 = DAT_1030b660;
                    DAT_10c635ec = 1;
                    DAT_10c64000 = gameOvers + 1;
                    DAT_10db6ee8[0].m_0075 = 0;
                    DAT_11018734 = 1;
                    DAT_1101de30 = 1;
                    return 0;
                }
                goto gameOverAfter;
            }
        }
        else
        {
gameOverAfter:
            if (DAT_104bfe78 == 0)
            {
waterThaw:
                // The frozen water thaws when its timer runs out.
                if (DAT_104c02dc != 0 && DAT_104c02e7 != 0 && (DAT_104c02e7--, DAT_104c02e7 < 2))
                {
                    FUN_1000dab0(0, 100, 0, 0xff5050);
                    FUN_10011fb0(0);
                    FUN_1000dab0(0, 0, 0xf, 0xff5050);
                    DAT_104c02dc = 0;
                    DAT_104c02e7 = 0x1e;
                }
            }
        }
        if (DAT_104bfe20 == 0)
            DAT_104c00b2 = 0;
        else if (DAT_104c00b2 < 0x32)
            DAT_104c00b2++;

        // Input of this tick: mouse state, the Enter key and the caption timers.
        mouseLeft = 0;
        mouseRight = 0;
        mouseMiddle = 0;
        mouseExtra = 0;
        mouseWheel = 0;
        DAT_1020d9e0.M_10001450();
        mousePos[0] = 0;
        mousePos[1] = 0;
        if (DAT_1065cb98 == 0)
            DAT_1020d9e0.M_10001110(mousePos, &mouseLeft, &mouseRight, &mouseMiddle, &mouseWheel, &mouseExtra);
        if (DAT_104c030a != 0 && ((GetKeyState(VK_RETURN) & 0xf000) != 0 || mouseLeft == 2) && DAT_10f6811c > 5)
            DAT_104c030c = DAT_104bf890;
        if (DAT_104c0148 != 0)
            DAT_104c0148--;
        DAT_104c022e = 0;
        inputStrength = 0.0f;
        if ((DAT_104bfe6c == 2 || DAT_104bfe6c == 3) && (DAT_104bfeb4 != 0 || (DAT_104c0230 & 3) != 0)
            && (mouseLeft == 2 || DAT_1065cb30 != 0))
            DAT_104bfe6c = 0;
        if (DAT_104c0334 != 0)
            DAT_104c0334--;

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
