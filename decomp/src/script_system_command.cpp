// The script system command dispatcher: one big if-chain over the command id that counts, stores and restores
// script flags, spawns shaga effects, reloads the player model and starts the light morphs of the water regions.

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "vec4.h"
#include "matrix4.h"
#include "key_track.h"
#include "node.h"
#include "node_data.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game.h"
#include "game_vars.h"
#include "game_load_data.h"
#include "script_flag.h"
#include "map_vars.h"
#include "map_records.h"
#include "map_setup.h"
#include "menu_save_load.h"
#include "menu_scene.h"
#include "camera_vars.h"
#include "scene_command.h"
#include "action_select_vars.h"
#include "route_flier.h"
#include "render_vars.h"
#include "player_frame.h"
#include "path_walk.h"
#include "title_screen.h"
#include "text_window.h"
#include "gimmick_slot.h"
#include "world.h"
#include "region_view.h"
#include "shaga.h"
#include "tutorial_book.h"
#include "field_item_menu.h"
#include "doppler_loops.h"
#include "engine_memory.h"
#include "tracker.h"
#include "win_vars.h"
#include "minigame_hud.h"
#include "ui_vars.h"
#include "script_system.h"

extern "C" int FUN_10021380(int map);                  // Map_SetEntryFlags
extern "C" int FUN_10034920(int index, int variant);   // Bs003_SetVariant
extern "C" int FUN_1000da40(int id, int value, int frames);
extern "C" int FUN_1000db30(int id, int value, int frames);
extern "C" int FUN_10020f60(void);                     // collide_regions
extern "C" int FUN_10020560();                         // Regions_Build
extern "C" int FUN_100114a0(void);
extern "C" int FUN_10137b10(void);                     // Screenshot_Capture
#include "value_command.h"
extern "C" int FUN_10188fb0(void);                     // Texture_ReloadMissing
extern "C" int FUN_1015fea0(const char *path, int index, unsigned int value);   // SystemSav_WriteRecord
int FUN_100425f0(const Vec4 *position, const char *model, int radius, float heading, float force);   // Shaga_Create
int FUN_100fc4a0(int map, const char *startMarker, int heightOffset);             // Map_SetupByNumber
int ClearMarkers_Create();
extern "C" int FUN_1013f880(void);                     // Random

// Stores `value` in script flag `index` (as Game::SetScriptFlag does it inline: flag 0 and indexes past 3999 stay).
#define STORE_FLAG(index, value) \
    do \
    { \
        int flagIndex = (index); \
        if (flagIndex <= 3999 && flagIndex != 0) \
            DAT_10c62edc[flagIndex] = (value); \
    } while (0)

// The shaga effect of `model` at `position` (radius `radius`, turned by `heading`, pushed by `force`).
#define SPAWN_SHAGA(model, radius) FUN_100425f0(&position, model, radius, heading, force)

// Adds the set bits of the script flag words firstFlag .. firstFlag + flagCount - 1 to `bits`.
#define COUNT_FLAG_BITS_LOOP(firstFlag, flagCount) \
    for (int index = (firstFlag); index < (firstFlag) + (flagCount); index++) \
    { \
        int value = ScriptFlag(index); \
        for (int bit = 0; bit < FLAG_BITS_PER_WORD; bit++) \
            if (value & (1 << bit)) \
                bits++; \
    }

// Releases the HUD textures and the texture slot of the haze ("@moya") node, then reloads the missing textures.
#define RELEASE_HUD_TEXTURES(force) \
    for (int slot = 0; slot < 0x10; slot++) \
    { \
        if (DAT_10202808[slot] >= 0) \
            FUN_10174c70(DAT_10202808[slot], force); \
        DAT_10202808[slot] = -1; \
    }

// 0x100fedd0 (Script_SystemCommand). Runs system command `command` (a SYSCMD_* id) with `argument`. The commands
// count and store script flag bit masks (the room and timed-run records), spawn shaga effects, switch levers,
// read and write the shot registers DAT_10c6356c.., reload the player model, place the observer and start the
// flood or drain light morph of the map's regions. Unknown ids do nothing. Returns 0.
int Script_SystemCommand(int command, int argument)
{
    Vec4 cameraPoint;
    if (command == SYSCMD_COUNT_FLAG_BITS)
    {
        int bits = 0;
        if (argument == 0)
        {
            COUNT_FLAG_BITS_LOOP(1000, 0x18)
            DAT_10c6356c = bits;
        }
        else if (argument == 1)
        {
            COUNT_FLAG_BITS_LOOP(ROOM_BITS_FLAG_BASE, 0xc)
            DAT_10c6356c = bits;
        }
        else if (argument == 2)
        {
            for (int pair = 0; pair < 0x18; pair++)
            {
                int value = DAT_10874848.TestScriptFlag(ROOM_BITS_PAIR_FLAG_BASE + pair);
                for (int bit = 0; bit < FLAG_BITS_PER_WORD; bit++)
                    if (value & (1 << bit))
                        bits++;
            }
            DAT_10c6356c = bits;
        }
        return 0;
    }
    if (command == SYSCMD_BEGIN_TIMED_RUN)
    {
        for (int index = 1000; index < 1200; index++)
            STORE_FLAG(index, 0);
        DAT_10c63ff4 = DAT_10c62ed0;
        DAT_10c63e9c = 0;
        DAT_10c66d18 = 2;
        DAT_10db717c = 0;
        DAT_10db6ee8[0].PlayMotion(0, 0, 5, 1.0f);
        FUN_10021380(argument);
        DAT_10c64004 = GetTickCount();
        int startFlag = DAT_10c64028 + 500;
        DAT_10c63ffc = 0;
        STORE_FLAG(startFlag, 1);
        return 0;
    }
    if (command == SYSCMD_RECORD_ROOM_BITS)
    {
        int stateBits = DAT_10874848.TestScriptFlag(DAT_10c63ffc + ROOM_BITS_FLAG_BASE);
        for (int i = 0; i < ROOM_BITS_SLOT_COUNT; i++)
        {
            int slot = ROOM_BITS_FIRST_SLOT + i;
            if (DAT_1021f3c0[slot] == 0xf)
            {
                for (int gimmick = 0; gimmick < 0x40; gimmick++)
                {
                    if (DAT_10860df0[gimmick].active == (int)&DAT_10db6ee8[slot] &&
                        stricmp(DAT_10db6ee8[slot].root->name, "o_002") == 0)
                    {
                        int state = DAT_10874848.TestScriptFlag(DAT_10860df0[gimmick].id);
                        if (state < 0 || state == 1)
                            stateBits |= 1 << i;
                    }
                }
            }
        }
        STORE_FLAG(DAT_10c63ffc + ROOM_BITS_FLAG_BASE, stateBits);
        int lowWord = ScriptFlag(DAT_10c63ffc * 2 + ROOM_BITS_PAIR_FLAG_BASE);
        int highWord = ScriptFlag(DAT_10c63ffc * 2 + ROOM_BITS_PAIR_FLAG_BASE + 1);
        for (int slot = ROOM_BITS_FIRST_SLOT; slot < ROOM_BITS_FIRST_SLOT + ROOM_BITS_SLOT_COUNT; slot++)
        {
            if (DAT_1021f3c0[slot] == (char)0xb0 && (DAT_10db6ee8[slot].m_0280 == 0 || DAT_10db6ee8[slot].m_0280 == 1))
            {
                int bit = slot - ROOM_BITS_FIRST_SLOT;
                if (slot < ROOM_BITS_FIRST_SLOT + ROOM_BITS_BITS_PER_WORD)
                    lowWord |= 1 << bit;
                else
                    highWord |= 1 << (bit - ROOM_BITS_BITS_PER_WORD);
            }
        }
        STORE_FLAG(DAT_10c63ffc * 2 + ROOM_BITS_PAIR_FLAG_BASE, lowWord);
        STORE_FLAG(DAT_10c63ffc * 2 + ROOM_BITS_PAIR_FLAG_BASE + 1, highWord);
        return 0;
    }
    if (command == SYSCMD_SET_ENTRY_FLAGS)
    {
        FUN_10021380(argument);
        return 0;
    }
    if (command == SYSCMD_SPAWN_SHAGA)
    {
        float force = 1.0f;
        if (DAT_10c6357c == 100000)
            force = (float)DAT_10c63580 * 0.01f;
        if (argument == 0)
        {
            argument = FUN_1013f880() % SHAGA_KIND_MAX_RANDOM + 1;
            if (argument > SHAGA_KIND_FIRST_FAULTED)
                argument += 0x20;
        }
        Vec4 position;
        float heading = -1000.0f;
        if (DAT_10c63578 != 0)
            heading = (float)DAT_10c63578 * 3.1415f / 180.0f;
        position.x = (float)DAT_10c6356c;
        position.y = (float)DAT_10c63570;
        position.z = (float)DAT_10c63574;
        int first = -1;
        int second = -1;
        bool pair = false;
        switch (argument)
        {
        case 2:
            first = SPAWN_SHAGA("EEQ_08d.it3", 0x14);
            break;
        case 3:
            first = SPAWN_SHAGA("EEQ_09d.it3", 0x19);
            break;
        case 4:
            first = SPAWN_SHAGA("EEQ_10d.it3", 0x19);
            break;
        case 5:
            first = SPAWN_SHAGA("EEQ_11d.it3", 0x1e);
            break;
        case 6:
            first = SPAWN_SHAGA("EEQ_16d.it3", 0x15);
            break;
        case 7:
            first = SPAWN_SHAGA("EEQ_17d.it3", 0x14);
            break;
        case 8:
            first = SPAWN_SHAGA("EEQ_18d.it3", 0x1e);
            break;
        case 9:
            first = SPAWN_SHAGA("EEQ_14d.it3", 0x23);
            break;
        case 10:
            first = SPAWN_SHAGA("EEQ_07b.it3", 0x2d);
            break;
        case 11:
            first = SPAWN_SHAGA("EEQ_08bd.it3", 0x14);
            break;
        case 12:
            first = SPAWN_SHAGA("EEQ_09bd.it3", 0x19);
            break;
        case 13:
            first = SPAWN_SHAGA("EEQ_10bd.it3", 0x19);
            break;
        case 14:
            first = SPAWN_SHAGA("EEQ_11bd.it3", 0x1e);
            break;
        case 15:
            first = SPAWN_SHAGA("EEQ_14bd.it3", 0x23);
            break;
        case 16:
            first = SPAWN_SHAGA("EEQ_16bd.it3", 0x15);
            break;
        case 17:
            first = SPAWN_SHAGA("EEQ_18bd.it3", 0x15);
            break;
        case 0x32:
            first = SPAWN_SHAGA("EEQ_19d.it3", 0x14);
            break;
        case 0x33:
            first = SPAWN_SHAGA("EEQ_19bd.it3", 0x19);
            break;
        case 0x34:
            first = SPAWN_SHAGA("EEQ_20d.it3", 0x14);
            break;
        case 0x35:
            first = SPAWN_SHAGA("EEQ_20bd.it3", 0x14);
            break;
        case 0x36:
            first = SPAWN_SHAGA("EEQ_21d.it3", 0x14);
            second = SPAWN_SHAGA("EEQ_21td.it3", 0x14);
            pair = true;
            break;
        case 0x37:
            first = SPAWN_SHAGA("EEQ_21bd.it3", 0x14);
            second = SPAWN_SHAGA("EEQ21btd.it3", 0x14);
            pair = true;
            break;
        case 0x38:
            first = SPAWN_SHAGA("EEQ_22d.it3", 0x14);
            break;
        case 0x39:
            first = SPAWN_SHAGA("EEQ_22bd.it3", 0x14);
            break;
        case 0x3a:
            first = SPAWN_SHAGA("EEQ_23d.it3", 0x14);
            break;
        case 0x3b:
            first = SPAWN_SHAGA("EEQ_23bd.it3", 0x14);
            break;
        case 0x3c:
            first = SPAWN_SHAGA("EEQ_24rd.it3", 0x23);
            second = SPAWN_SHAGA("EEQ_24ld.it3", 0x23);
            pair = true;
            break;
        case 0x3d:
            first = SPAWN_SHAGA("EEQ_24bd.it3", 0x14);
            second = SPAWN_SHAGA("EEQ_24bd.it3", 0x14);
            pair = true;
            break;
        default:
            first = SPAWN_SHAGA("EEQ_07.it3", 0x2d);
            break;
        }
        if (first > -1 || second > -1)
        {
            if (argument >= SHAGA_KIND_PAIRED_FIRST)
            {
                if (first > -1)
                {
                    DAT_1042db08[first] = argument;
                    DAT_1021d018[first] = -1;
                    DAT_10446e00[first]->RestartColourPlugs("stop", DAT_104bf8a4);
                }
                if (pair && second > -1)
                {
                    DAT_1042db08[second] = argument + SHAGA_PARTNER_KIND_OFFSET;
                    DAT_1021d018[second] = -1;
                    DAT_10446e00[second]->RestartColourPlugs("stop", DAT_104bf8a4);
                }
            }
            else
            {
                DAT_1042db08[first] = -1;
                DAT_1021d018[first] = -1;
                DAT_10446e00[first]->RestartColourPlugs("stop", DAT_104bf8a4);
            }
        }
        return 0;
    }
    if (command == SYSCMD_GET_PLAYER_STATE)
    {
        DAT_10c6356c = DAT_104c00d0;
        return 0;
    }
    if (command == SYSCMD_TUTORIAL_OPEN_FIRST)
    {
        FUN_1004ed60(0);
        return 0;
    }
    if (command == SYSCMD_TUTORIAL_CLOSE)
    {
        FUN_1004f250();
        return 0;
    }
    if (command == SYSCMD_TUTORIAL_OPEN_SECOND)
    {
        FUN_1004ed60(1);
        return 0;
    }
    if (command == SYSCMD_UNUSED_C353 || command == SYSCMD_UNUSED_C355)
        return 0;
    if (command == SYSCMD_SET_BS003_VARIANT)
    {
        FUN_10034920(0x3b, argument);
        return 0;
    }
    if (command == SYSCMD_VOLUME_FADE)
    {
        if (argument == 0)
            FUN_1000da40(0, 0x64, 0);
        else
            FUN_1000db30(0, 0x64, 0);
        return 0;
    }
    if (command == SYSCMD_TOGGLE_LEVERS)
    {
        for (int i = 1; i < 0x41; i++)
        {
            GimmickSlot &lever = DAT_10860cec[i];
            if (lever.type == 0xb && lever.id == argument)
            {
                if (lever.targetFrame == 0xf)
                {
                    FUN_1003ad20(0x37e, &lever.node->worldMatrix.GetTranslation(), 1.0f);
                    lever.targetFrame = 0;
                    lever.frame = 0xe;
                    if (lever.node != 0)
                        lever.node->RestartColourPlugs(lever.motionOff, DAT_104bf8a4);
                }
                else
                {
                    FUN_1003ad20(0x37e, &lever.node->worldMatrix.GetTranslation(), 1.0f);
                    lever.targetFrame = 0xf;
                    lever.frame = 1;
                    if (lever.node != 0)
                        lever.node->RestartColourPlugs(lever.motionOn, DAT_104bf8a4);
                }
            }
        }
        return 0;
    }
    if (command == SYSCMD_GET_KSK_POSITION)
    {
        char name[0x50];
        int number = argument;
        if (number >= 0x3f)
            number = 0x3f;
        else if (number <= 0)
            number = 0;
        sprintf(name, "@ksk%02d_", number);
        if (DAT_10255f08 != 0)
        {
            Node *marker = DAT_10255f08->Find(name, 0);
            if (marker != 0)
            {
                DAT_10c6356c = (int)marker->matrix._41;
                DAT_10c63570 = (int)marker->matrix._42;
                DAT_10c63574 = (int)marker->matrix._43;
            }
            else
            {
                DAT_10c6356c = 0;
                DAT_10c63570 = 0;
                DAT_10c63574 = 0;
            }
        }
        return 0;
    }
    if (command == SYSCMD_SET_SAVED_VALUE)
    {
        DAT_104c03f0 = argument;
        return 0;
    }
    if (command == SYSCMD_GET_SAVED_VALUE)
    {
        DAT_10c6356c = DAT_104c03f0;
        return 0;
    }
    if (command == SYSCMD_SAVE_OR_LOAD_FLAGS)
    {
        if (argument == 0)
        {
            for (int i = 0; i < 4000; i++)
                DAT_1021fb48[i] = DAT_10874848.TestScriptFlag(i);
            return 0;
        }
        for (int i = 0; i < 4000; i++)
            DAT_10874848.SetScriptFlag(i, DAT_1021fb48[i]);
        FUN_1000e750();
        return 0;
    }
    if (command == SYSCMD_NOP_1868B)
        return 0;
    if (command == SYSCMD_PLACE_OBSERVER)
    {
        DAT_1086df38._41 = DAT_10db9240;
        DAT_1086df38._42 = DAT_10db9244;
        DAT_1086df38._43 = DAT_10db9248 - 75.0f;
        DAT_10e3dec0 = DAT_10db9364;
        DAT_10e3debc = DAT_10db9364;
        return 0;
    }
    if (command == SYSCMD_MARK_PLAYER_START)
    {
        DAT_1030b660 = DAT_10c62ed0;
        DAT_108029b8.Copy(DAT_10db9210.GetTranslation());
        DAT_108029b8.w = DAT_10db9364;
        return 0;
    }
    if (command == SYSCMD_RESET_GAME_FLAGS)
    {
        if (DAT_103150fd != 0)
            DAT_10c64f1c[0] = 1;
        if (DAT_103150f9 != 0)
            DAT_10c64f1c[1] = 1;
        if (DAT_103150f8 != 0)
            DAT_10c64f1c[2] = 1;
        if (DAT_10c655e4 > 0x3de)
            DAT_10c655e4 = 0x3de;
        if (DAT_10c64aac != 0)
            DAT_10c64f2c[0] = 1;
        if (DAT_10c64ab8 != 0)
            DAT_10c64f2c[1] = 1;
        if (DAT_10c64ab4 != 0)
            DAT_10c64f2c[2] = 1;
        if (DAT_10c64abc != 0)
            DAT_10c64f2c[3] = 1;
        if (DAT_10c64ac0 != 0)
            DAT_10c64f2c[4] = 1;
        if (DAT_10c64ac4 != 0)
            DAT_10c64f2c[5] = 1;
        for (int flag = 1; flag < 4000; flag++)
        {
            if (flag != 0x12a && flag != 0x9c2 && (flag < 0xc9a || flag > 0xcb2) && flag != 0xe0c && flag != 0x6fd &&
                flag != 0x7d5 && flag != 0x7d4 && flag != 0x7db && flag != 0x7e0 && flag != 0x7e3 && flag != 0x7e5 &&
                flag != 0x7eb && flag != 0x7ed && flag != 0x7ef && flag != 0x7f1 && flag != 0x807 && flag != 0x810 &&
                flag != 0x811 && flag != 0x812 && flag != 0x814 && flag != 0x815 && flag != 0x816 && flag != 0x817 &&
                flag != 0x818 && flag != 0x819)
                DAT_10874848.SetScriptFlag(flag, 0);
        }
        DAT_10daaaf0 = 0x32;
        DAT_10daaaec = 0x32;
        FUN_1000e750();
        for (int item = 0; item < 0x50; item++)
        {
            if (item != 2)
            {
                int itemIndex = item;
                int amount = 0x64;
                FUN_100120a0(&itemIndex, &amount);
            }
        }
        if (DAT_10daabb8 != 0)
            DAT_10daabb8 = 0;
        DAT_10daabb4 = 0x29;
        DAT_10c62ed0 = 1000;
        return 0;
    }
    if (command == SYSCMD_FLOOD_LIGHTS || command == SYSCMD_DRAIN_LIGHTS)
    {
        int count = argument;
        if (count >= SCRIPT_LIGHT_POINT_MAX)
            count = SCRIPT_LIGHT_POINT_MAX;
        else if (count <= 0)
            count = 0;
        FUN_10062e70(DAT_10edbce8, count, command == SYSCMD_FLOOD_LIGHTS);
        FUN_10020f60();
        return 0;
    }
    if (command == SYSCMD_SET_CAMERA_FRAME)
    {
        DAT_1020c2fc = argument;
        return 0;
    }
    if (command == SYSCMD_RELOAD_PLAYER_A)
    {
        RELEASE_HUD_TEXTURES(0)
        FUN_10188fb0();
        DAT_102028e0 = -1;
        DAT_102024a0 = -1;
        Matrix4 savedMatrix = DAT_10db9210;
        DAT_10db6ee8[0].Unload();
        FUN_1003b6e0(0, 0);
        DAT_10db9210 = savedMatrix;
        Vec4 position = savedMatrix.GetTranslation();
        DAT_10db6ee8[0].SetPosition(position.x, position.y, position.z);
        DAT_10db9210 = savedMatrix;
        DAT_10db6ee8[0].SetDropCollider(DAT_10255f08);
        FUN_100154c0(DAT_10daabb4);
        if (DAT_10db6ee8[0].root != 0)
        {
            DAT_10db6ee8[0].root->M_101946b0(0x1f, -1);
            DAT_10db6ee8[0].root->M_10194730(0x1f, -1);
        }
        DAT_10db6f68 = 1;
        FUN_10012510(DAT_10c652cc != 0, 0);
        DAT_10daaaec = DAT_10daaaf0;
        DAT_10255960[0].kind = 0;
        DAT_10255960[1].kind = 0;
        DAT_10255960[2].kind = 0;
        FUN_1000e750();
        DAT_1020c2c0 = argument;
        DAT_10c64a5c = 1;
        return 0;
    }
    if (command == SYSCMD_WAIT_FRAME_STEPS)
    {
        while (DAT_104c0394 == 0 && FUN_10137b10() != 0)
            ;
        return 0;
    }
    if (command == SYSCMD_SET_LIGHT_POINT)
    {
        int point = argument;
        if (point >= SCRIPT_LIGHT_POINT_MAX)
            point = SCRIPT_LIGHT_POINT_MAX;
        else if (point <= 0)
            point = 0;
        DAT_10edbce8[point].x = (float)DAT_10c6356c;
        DAT_1020c370[point] = DAT_10c63578;
        DAT_10edbce8[point].y = (float)DAT_10c63570;
        DAT_10edbce8[point].z = (float)DAT_10c63574;
        return 0;
    }
    if (command == SYSCMD_BUILD_CLEAR_MARKERS)
    {
        if (DAT_104c0394 != 0)
        {
            DAT_11b1ba40(DAT_104c0394);
            DAT_104c0394 = 0;
        }
        if (DAT_104c03ac != 0 && DAT_10c63378 == 1)
        {
            DAT_104c0394 = (char *)DAT_11b0b938(DAT_104c03ac->extra->tracks[NODE_TRACK_TRANSLATION].count, "tsm_lpnonroot");
            FUN_10020f60();
            FUN_10020560();
            ClearMarkers_Create();
            if (DAT_10c66d08 == 0)
                FUN_1000da40(0, 0, 0x1e);
        }
        return 0;
    }
    if (command == SYSCMD_SHOW_FIELD_MAP)
    {
        FUN_100114a0();
        DAT_10c63384 = 1;
        DAT_10c63380 = 1;
        DAT_10c6337c = 1;
        DAT_1030c84c = 1;
        if (DAT_10255f08 != 0)
        {
            Node *field = DAT_10255f08->Find("Field3", 0);
            if (field != 0)
                field->SetFlags(0x20);
        }
        while (DAT_10c63380 != 0 && FUN_10137b10() != 0)
            ;
        return 0;
    }
    if (command == SYSCMD_CAMERA_PATH_POINT)
    {
        DAT_1020c314 = argument;
        cameraPoint.Copy(DAT_104c03ac->extra->AnimatedMatrix(argument).GetTranslation());
        DAT_1086df38.SetTranslation(cameraPoint);
        return 0;
    }
    if (command == SYSCMD_UNLOCK_COSTUME || command == SYSCMD_UNLOCK_CURRENT_COSTUME)
    {
        if (command == SYSCMD_UNLOCK_COSTUME)
            DAT_104c0230 |= (unsigned short)(1 << argument);
        else
        {
            if (DAT_10c635f8 == 0)
                DAT_104c0230 |= 2;
            else if (DAT_10c635f8 == 1)
                DAT_104c0230 |= 1;
            else if (DAT_10c635f8 == 2)
                DAT_104c0230 |= 4;
            if (DAT_10c66cc0 == 1)
                DAT_104c0230 |= 0x10;
            if (DAT_10c66cc0 == 2)
                DAT_104c0230 |= 0x100;
        }
        FUN_1015fea0(DAT_10256110, SYSTEM_RECORD_COSTUMES, (unsigned short)DAT_104c0230);
        return 0;
    }
    if (command == SYSCMD_GET_COSTUME_BITS)
    {
        unsigned int unlocked = (unsigned short)DAT_104c0230;
        DAT_10c6356c = unlocked & 1;
        DAT_10c63570 = unlocked & 2;
        DAT_10c63574 = unlocked & 4;
        DAT_10c6357c = unlocked & 8;
        DAT_10c63580 = unlocked & 0x10;
        DAT_10c63584 = unlocked & 0xf7;
        DAT_10c63588 = unlocked & 0x40;
        DAT_10c6358c = unlocked & 0x80;
        DAT_10c63590 = unlocked & 0x20;
        DAT_10c63594 = unlocked & 0x100;
        DAT_10c63598 = unlocked & 0x200;
        DAT_10c63578 = unlocked;
        return 0;
    }
    if (command == SYSCMD_GET_MAP_RECORD_POINT)
    {
        if (DAT_104c03ac != 0)
            cameraPoint.Copy(DAT_104c03ac->extra->AnimatedMatrix(DAT_10210fc0[argument].camera.frame).GetTranslation());
        DAT_10c6356c = (int)cameraPoint.x;
        DAT_10c63570 = (int)cameraPoint.y;
        DAT_10c63574 = (int)cameraPoint.z;
        return 0;
    }
    if (command == SYSCMD_SET_CAMERA_FRAME_OF_RECORD)
    {
        if (argument < 0)
            DAT_1020c2fc = -1;
        else
            DAT_1020c2fc = DAT_10210fc0[argument].camera.frame;
        return 0;
    }
    if (command == SYSCMD_GET_TARGET_MAP)
    {
        DAT_10c6356c = DAT_1020c2c0;
        return 0;
    }
    if (command == SYSCMD_ENTER_FIELD_MODE)
    {
        DAT_10c652cc = 1;
        DAT_104bf8c0 = 1;
        DAT_104bf8bc = 0;
        DAT_10c66d08 = 0;
        DAT_1020d9e0.M_10001010((int)DAT_10f6569c, (int)FUN_1000fd50, (int)FUN_1013e6e0);
        DAT_10db86d0 = 0;
        DAT_10db7c34 = 1.0f;
        DAT_10db9380 = 1;
        DAT_10db717c = 0;
        if (DAT_104c039c != 0)
        {
            DAT_104c039c->Release();
            DAT_11b1ba40(DAT_104c039c);
            DAT_104c039c = 0;
            DAT_11b1ba40(DAT_104c03a0);
            DAT_104c03a0 = 0;
            DAT_104c03a8->Release();
            DAT_11b1ba40(DAT_104c03a8);
            DAT_104c03a8 = 0;
            DAT_104c03ac = 0;
            for (int i = 0; i < 36; i++)
            {
                char textureName[0x50];
                strcpy(textureName, DAT_101d65b0[i]);
                textureName[strlen(textureName) - 4] = 0;
                int texture = FUN_10161920(textureName);
                if (texture > -1)
                    FUN_10174c70(texture, 1);
            }
        }
        Script_ValueCommand(SYSCMD_RELOAD_PLAYER_MODEL, 0);
        for (int flag = 0xe0e; flag < 0xf3c; flag++)
            DAT_10874848.SetScriptFlag(flag, 0);
        for (int flag = 0xb10; flag < 0xb30; flag++)
            DAT_10874848.SetScriptFlag(flag, 0);
        FUN_10012510(1, 0);
        return 0;
    }
    if (command == SYSCMD_RELOAD_PLAYER_B)
    {
        RELEASE_HUD_TEXTURES(0)
        FUN_10188fb0();
        DAT_102028e0 = -1;
        DAT_102024a0 = -1;
        Matrix4 savedMatrix = DAT_10db9210;
        DAT_10db6ee8[0].Unload();
        FUN_1003b6e0(0, 0);
        DAT_10db9210 = savedMatrix;
        Vec4 position = savedMatrix.GetTranslation();
        DAT_10db6ee8[0].SetPosition(position.x, position.y, position.z);
        DAT_10db9210 = savedMatrix;
        DAT_10db6ee8[0].SetDropCollider(DAT_10255f08);
        FUN_100154c0(DAT_10daabb4);
        if (DAT_10db6ee8[0].root != 0)
        {
            DAT_10db6ee8[0].root->M_101946b0(0x1f, -1);
            DAT_10db6ee8[0].root->M_10194730(0x1f, -1);
        }
        DAT_10db6f68 = 1;
        FUN_10012510(DAT_10c652cc != 0, 0);
        DAT_10daaaec = DAT_10daaaf0;
        DAT_10255960[0].kind = 0;
        DAT_10255960[1].kind = 0;
        DAT_10255960[2].kind = 0;
        DAT_104bf8c0 = 0xf;
        DAT_104bf8bc = 0;
        DAT_10c652cc = 0;
        FUN_1000e750();
        for (int i = 0; i < 0x100; i++)
        {
            DAT_10c62250[i] = 0;
            DAT_10c62650[i * 2] = 0;
        }
        DAT_10c62ed0 = 999;
        if (argument != 0)
        {
            DAT_104c0096 = 1;
            FUN_100fc4a0(999, 0, 0);
            DAT_104c0096 = 0;
        }
        else
            FUN_100fc4a0(999, 0, 0);
        DAT_1101d8a8 = 0xf;
        DAT_11018734 = 1;
        DAT_104c0092 = 0;
        DAT_104c0093 = 0;
        return 0;
    }
    if (command == SYSCMD_RELOAD_PLAYER_MODEL)
    {
        RELEASE_HUD_TEXTURES(0)
        FUN_10188fb0();
        DAT_10db6ee8[0].Unload();
        FUN_1003b6e0(0, 0);
        FUN_100154c0(DAT_10daabb4);
        DAT_102024a0 = -1;
        FUN_10012510(DAT_10c652cc != 0, 0);
        return 0;
    }
    return 0;
}
