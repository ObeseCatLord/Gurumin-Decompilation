// Scene_HostCallback: the world object's notifications from the script engine (character model files, sounds,
// effects, movies, map loads, game flags).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "char_tables.h"
#include "char_vars.h"
#include "camera_vars.h"
#include "effect_manager.h"
#include "game.h"
#include "game_vars.h"
#include "last_position.h"
#include "map_placement.h"
#include "map_vars.h"
#include "mmv_player.h"
#include "model_bank.h"
#include "render_vars.h"
#include "texture_object.h"
#include "text_window.h"
#include "tracker.h"
#include "world.h"
#include "ui_vars.h"
#include "world_renderer.h"
#include "dshow_stream.h"
#include "bgm_track.h"
#include "scene_callback.h"

extern "C" int FUN_1000c020(char *out, const char *text);                   // Text_Substitute
extern "C" const char *FUN_1013f8a0(const char *name, int folder);          // Path_Data
extern "C" int FUN_1013faf0(void);                                          // FrameTimer_ResetNext2
extern "C" void FUN_1003b6e0(int index, int variant);                       // Costume_LoadPlayerModel
extern "C" int FUN_100143f0(int index, const char *name);                   // swaps the item model of a character
extern "C" int FUN_10012510(int mode, int index);                           // Char_SetHandTool
extern "C" int FUN_10012780(int mode, int index);                           // Char_SetHandToolPlain
extern "C" int FUN_10017aa0(int id);                                        // Enemy_FindByKind
extern "C" int FUN_1005f530(int slot);                                      // Switch_TurnOn
extern "C" int FUN_1005f640(int slot);                                      // Switch_TurnOff
extern "C" int FUN_101011b0(int id, int value);                             // sets a script value
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);   // Sound_PlayEffect
extern "C" int FUN_1003ac20(const char *name, const Vec4 *position, float volume);   // Sound_PlayWav
extern "C" int FUN_1003a7d0(const char *path, float x, float y, float z, int start, float ox, float oy, float oz);  // EffectModel_Place
int FUN_100fc4a0(int map, const char *startMarker, int heightOffset);       // Map_SetupByNumber
extern "C" void FUN_101b2580(void);                                         // starts the movie library
extern "C" void FUN_101b27d0(const char *path);                             // opens a movie file in it
extern "C" int FUN_101b25a0(int a);                                         // decodes its next frame
extern "C" void FUN_101b2610(void);                                         // closes it

// Character slots that codes 1..99 can select: all before the enemy slots; the enemy slot range.
#define CALLBACK_PLAYER_SLOT 0
#define CALLBACK_SLOT_END 0x3a
#define CALLBACK_ENEMY_SLOT_FIRST 0x3a
#define CALLBACK_ENEMY_SLOT_END 0x5c
#define CALLBACK_SLOT_KIND_ENEMY 3
#define CALLBACK_SLOT_KIND_NEW 0x21
#define CALLBACK_FILE_EXTENSION_LENGTH 4
// Colour masks and states the model's nodes are tagged with.
#define CALLBACK_NODE_MASK_BODY 0x400
#define CALLBACK_NODE_MASK_SHADOW 0x200
#define CALLBACK_NODE_STATE_FE 0xfe
#define CALLBACK_NODE_STATE_FF 0xff
#define CALLBACK_NODE_STATE_2F 0x2f
#define CALLBACK_NODE_STATE_8F 0x8f
#define CALLBACK_FULL_HEALTH 1000
#define CALLBACK_CAMERA_FOLLOW_KIND 100
// Slots whose item model file depends on what the player unlocked.
#define CALLBACK_SLOT_ARMOUR 0x10
#define CALLBACK_SLOT_ITEM_6 6
#define CALLBACK_SLOT_ITEM_7 7
#define CALLBACK_SLOT_ITEM_11 0xb
// The effect command's turn angle is in degrees; the original converts with this approximation of pi.
#define CALLBACK_PI 3.1415f
#define CALLBACK_HALF_TURN 180.0f
// Flag ids of the flag command SCENE_CB_SET_GAME_FLAG.
#define CALLBACK_FLAG_OUTLINE_SLOT 0xa1c
#define CALLBACK_FLAG_SOUND_MUTE 0xb27
#define CALLBACK_FLAG_OUTLINE_MODE 0xa1d
#define CALLBACK_FLAG_OUTLINE_OPACITY 0xa1b
#define CALLBACK_FLAG_HAND_TOOL 300
#define CALLBACK_FLAG_SWITCH_ON 0x8fc
#define CALLBACK_FLAG_SWITCH_OFF 0x8fd
#define CALLBACK_FLAG_CUTSCENE_RESET 0x369
#define CALLBACK_FLAG_FADE_ON_TOP 0xa28
#define CALLBACK_FLAG_VALUE_DA7DD8 0xa29
#define CALLBACK_FLAG_LIMITS 0xdac
#define CALLBACK_FLAG_E74 0xe74
#define CALLBACK_LIMITS_ON 2
#define CALLBACK_LIMIT_MIN -3000000.0f
#define CALLBACK_LIMIT_MAX 3000000.0f
#define CALLBACK_OUTLINE_FULL 100
// Table of the character slot of each model.
#define CALLBACK_NO_SLOT -1
#define CALLBACK_COPY_PARAMETER_COUNT 0x20
#define CALLBACK_MOVIE_SLOT_SHIFT 10
// Script values the cut-scene reset sets.
#define CALLBACK_VALUE_HIDE_NODE 0x753b
#define CALLBACK_VALUE_COUNT_NODES 0x753c
#define CALLBACK_VALUE_NODE_GROUP 0x753d
#define CALLBACK_VALUE_4E2B 0x4e2b
#define CALLBACK_VALUE_4E2C 0x4e2c
#define CALLBACK_FAR_REGISTER 1000
#define CALLBACK_GROUP_BELOW -1000

// 0x10092af0 (Scene_HostCallback). The world object (WorldRenderer::Bind) calls it with a command `code` and the
// command's arguments (`args`: integers, or a string for the file and node names). Codes 1..99 select the character
// slot whose model file name matches the string (loading its model when it has none, else starting a new slot for
// it) and tell the world to use it. The rest start sounds, effects and movies, load maps, set game flags and handle
// the skip key. Returns 1 when the code was handled, for the movie codes the frame size, else 0.
int Scene_HostCallback(int code, int *args)
{
    char text[512];

    if (code == SCENE_CB_SUBSTITUTE_TEXT)
    {
        FUN_1000c020(text, (const char *)args);
        strcpy((char *)args, text);
        return 1;
    }
    if (code == SCENE_CB_MOUSE_SKIP)
    {
        int mouse[2];
        char mouseButton;
        char mouseB;
        char mouseC;
        int mouseD;
        mouseD = 0;
        mouseButton = 0;
        mouseB = 0;
        mouseC = 0;
        DAT_1020d9e0.M_10001110(mouse, &mouseButton, &mouseB, &mouseC, &mouseD, 0);
        if (DAT_104c0309 != 0)
            return 1;
        if (*args != 0 && (DAT_104bfeb4 != 0 || (DAT_104c0230 & 3) != 0) && (mouseButton == 2 || DAT_1065cb30 != 0))
        {
            DAT_104c0309 = 1;
            return 1;
        }
    }
    else if (code == SCENE_CB_RESET_SOUND_MUTE)
    {
        DAT_104bfe6c = 0;
        DAT_11b044e0 = 0;
    }
    else
    {
        if (code < 100)
        {
            if (0 < code)
            {
                const char *name = (const char *)args;
                int slot;
                for (slot = 0; slot < CALLBACK_SLOT_END; slot++)
                {
                    if (slot == CALLBACK_PLAYER_SLOT)
                    {
                        if (DAT_10daaae8[0].m_0165[0] != 0)
                        {
                            if (_strnicmp(DAT_10daaae8[0].m_0165, name, strlen(DAT_10daaae8[0].m_0165)) == 0 &&
                                DAT_1021f3c0[0] != CALLBACK_SLOT_KIND_ENEMY)
                            {
                                DAT_10c68000.M_10166910(code, 0, -1);
                                DAT_10db6ee8[0].m_022c.Fill(0.0f);
                                DAT_10db6ee8[0].alpha = 1.0f;
                                return 1;
                            }
                        }
                    }
                    else if (DAT_10daaae8[slot].m_0165[0] != 0)
                    {
                        if (strlen(name) - CALLBACK_FILE_EXTENSION_LENGTH == strlen(DAT_10daaae8[slot].m_0165))
                        {
                            if (_strnicmp(DAT_10daaae8[slot].m_0165, name, strlen(name) - CALLBACK_FILE_EXTENSION_LENGTH) == 0)
                            {
                                if (DAT_10db6ee8[slot].root == 0)
                                {
                                    FUN_1003b6e0(slot, 0);
                                    if (slot == CALLBACK_SLOT_ARMOUR)
                                    {
                                        FUN_100143f0(slot, 0);
                                        if (DAT_10c66cc4 != 0)
                                            FUN_100143f0(slot, "G_003E.it3");
                                    }
                                    else if (slot == CALLBACK_SLOT_ITEM_6)
                                    {
                                        FUN_100143f0(slot, 0);
                                        if (DAT_10c66cc8 != 0)
                                            FUN_100143f0(slot, "G_008E.it3");
                                    }
                                    else if (slot == CALLBACK_SLOT_ITEM_7)
                                    {
                                        FUN_100143f0(slot, 0);
                                        if (DAT_10c66ccc != 0)
                                            FUN_100143f0(slot, "G_009E.it3");
                                    }
                                    else if (slot == CALLBACK_SLOT_ITEM_11)
                                    {
                                        FUN_100143f0(slot, 0);
                                        if (DAT_10c66cd0 != 0)
                                            FUN_100143f0(slot, "G_013E.it3");
                                    }
                                    DAT_10db6ee8[slot].root->M_10194630(slot, CALLBACK_NODE_MASK_BODY);
                                    DAT_10db6ee8[slot].root->M_10194630(0, CALLBACK_NODE_MASK_SHADOW);
                                    DAT_10db6ee8[slot].root->M_101946b0(0xff, -1);
                                    DAT_10db6ee8[slot].root->M_10194730(CALLBACK_NODE_STATE_8F, -1);
                                    for (int word = 0; word < 0x10; word++)
                                        DAT_1020b430[slot].words[word] = 0;
                                    DAT_10210b30[slot] = 0;
                                    DAT_10254090[slot] = 1;
                                    DAT_10db6ee8[slot].SetDropCollider(DAT_10255f08);
                                    DAT_10804bf8.AddLitModel(&DAT_10db6ee8[slot]);
                                }
                                if (DAT_1021f3c0[slot] != CALLBACK_SLOT_KIND_ENEMY)
                                {
                                    DAT_10c68000.M_10166910(code, slot, -1);
                                    DAT_10db6ee8[slot].m_022c.Fill(0.0f);
                                    DAT_10db6ee8[slot].alpha = 1.0f;
                                    return 1;
                                }
                            }
                        }
                    }
                }
                for (slot = CALLBACK_ENEMY_SLOT_FIRST; slot < CALLBACK_ENEMY_SLOT_END; slot++)
                {
                    if (DAT_10daaae8[slot].m_0165[0] != 0)
                    {
                        if (strlen(name) - CALLBACK_FILE_EXTENSION_LENGTH == strlen(DAT_10daaae8[slot].m_0165))
                        {
                            if (_strnicmp(DAT_10daaae8[slot].m_0165, name, strlen(name) - CALLBACK_FILE_EXTENSION_LENGTH) == 0 &&
                                DAT_1021f3c0[slot] == CALLBACK_SLOT_KIND_ENEMY)
                            {
                                if (DAT_10daaae8[slot].m_00a8 != 0)
                                    DAT_10c68000.M_10166910(code, slot, DAT_10daaae8[slot].m_00a4);
                                else
                                    DAT_10c68000.M_10166910(code, slot, -1);
                                DAT_10db6ee8[slot].m_022c.Fill(0.0f);
                                DAT_10db6ee8[slot].alpha = 1.0f;
                                return 1;
                            }
                        }
                    }
                }
                slot = CALLBACK_ENEMY_SLOT_FIRST;
                do
                {
                    if (DAT_1021f3c0[slot] == 0)
                        break;
                    if (DAT_1021f3c0[slot + 1] == 0)
                    {
                        slot++;
                        break;
                    }
                    slot += 2;
                } while (slot < CALLBACK_ENEMY_SLOT_END);
                if (slot == CALLBACK_ENEMY_SLOT_END)
                    return 1;
                DAT_1021f3c0[slot] = CALLBACK_SLOT_KIND_NEW;
                DAT_10db6ee8[slot].LoadFile(FUN_1013f8a0(name, 0));
                DAT_10db6ee8[slot].root->ReleaseUniqueMeshData();
                DAT_10db6ee8[slot].SetDropCollider(DAT_10255f08);
                DAT_10804bf8.AddLitModel(&DAT_10db6ee8[slot]);
                DAT_10c68000.M_10166910(code, slot, -1);
                DAT_10db6ee8[slot].m_022c.Fill(0.0f);
                DAT_10db6ee8[slot].alpha = 1.0f;
                DAT_10db6ee8[slot].SetDropCollider(DAT_10255f08);
                DAT_10daaae8[slot].stats[STAT_HEALTH] = CALLBACK_FULL_HEALTH;
                DAT_10daaae8[slot].stats[STAT_MAX_HEALTH] = CALLBACK_FULL_HEALTH;
                for (int word = 0; word < 0x10; word++)
                    DAT_1020b430[slot].words[word] = 0;
                DAT_10467ba0[slot] = 0;
                DAT_102239d0[slot] = 0;
                DAT_10210d30[slot] = 0;
                DAT_104686b0[slot] = 0;
                DAT_1020d4c0[slot] = 0;
                DAT_102550e0[slot] = 0;
                DAT_10254090[slot] = 1;
                DAT_10218fc0[slot] = 0;
                DAT_10210b30[slot] = 0;
                DAT_10802bf8[slot].position.Fill(0.0f);
                DAT_10802bf8[slot].step.Fill(0.0f);
                DAT_10315350[slot] = 0;
                DAT_1021f6c0[slot] = 0;
                DAT_10db6ee8[slot].root->M_10194630(slot, CALLBACK_NODE_MASK_BODY);
                DAT_10db6ee8[slot].root->M_101946b0(CALLBACK_NODE_STATE_FE, CALLBACK_NODE_MASK_BODY);
                DAT_10db6ee8[slot].root->M_10194730(CALLBACK_NODE_STATE_2F, CALLBACK_NODE_MASK_BODY);
                DAT_10db6ee8[slot].root->M_10194630(0, CALLBACK_NODE_MASK_SHADOW);
                DAT_10db6ee8[slot].root->M_101946b0(CALLBACK_NODE_STATE_FF, CALLBACK_NODE_MASK_SHADOW);
                DAT_10db6ee8[slot].root->M_10194730(CALLBACK_NODE_STATE_2F, CALLBACK_NODE_MASK_SHADOW);
                DAT_10db6ee8[slot].root->M_101a8760();
                DAT_10db6ee8[slot].locked = 0;
                if (DAT_10daaae8[slot].m_00ac == CALLBACK_CAMERA_FOLLOW_KIND)
                {
                    DAT_1032d5b8[DAT_1049f0f0] = &DAT_10db6ee8[slot].matrix.translation;
                    DAT_1021bbc0[DAT_1049f0f0] = 0;
                    DAT_1049f0f0++;
                }
                DAT_1021f9c0[slot] = 0;
                DAT_10255a88[slot] = 1;
                DAT_10daaae8[slot].m_00a8 = 0;
            }
            return 1;
        }
        if (code == SCENE_CB_MMV_OPEN)
        {
            DAT_10f08ab0.Open(FUN_1013f8a0((const char *)args, 0));
            DAT_10f08ab0.M_10161050();
            DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
            FUN_1013faf0();
            return (DAT_10202478 << CALLBACK_MOVIE_SLOT_SHIFT | DAT_10f08ab0.width) << CALLBACK_MOVIE_SLOT_SHIFT | DAT_10f08ab0.height;
        }
        if (code == SCENE_CB_MMV_NEXT_FRAME)
        {
            if ((DAT_104bf890 & 1) == 0)
            {
                DAT_10f08ab0.M_10161050();
                DAT_10f08ab0.DrawToTexture(&DAT_118d0ba8[DAT_10202478]);
            }
            return DAT_10f08ab0.size > DAT_10f08ab0.frameIndex;
        }
        if (code == SCENE_CB_MOVIE_OPEN)
        {
            FUN_101b2580();
            FUN_101b27d0(FUN_1013f8a0((const char *)args, 0));
            DAT_118d0ba8[DAT_10202478].Write16(0, 0, DAT_11b1bb58, DAT_11b1bb64, DAT_11b1bb58 * 2, DAT_11b1bb54);
            FUN_1013faf0();
            return (DAT_10202478 << CALLBACK_MOVIE_SLOT_SHIFT | DAT_11b1bb58) << CALLBACK_MOVIE_SLOT_SHIFT | DAT_11b1bb64;
        }
        if (code == SCENE_CB_MOVIE_NEXT_FRAME)
        {
            if ((DAT_104bf890 & 1) != 0)
                return 1;
            if (FUN_101b25a0(0) != 0)
            {
                DAT_118d0ba8[DAT_10202478].Write16(0, 0, DAT_11b1bb58, DAT_11b1bb64, DAT_11b1bb58 * 2, DAT_11b1bb54);
                return 1;
            }
        }
        else
        {
            if (code == SCENE_CB_MOVIE_CLOSE)
            {
                FUN_101b2610();
                return 1;
            }
            if (code == SCENE_CB_DSHOW_OPEN)
            {
                DAT_10836e98.Open(FUN_1013f8a0((const char *)args, 0));
                DAT_10836e98.M_10148910();
                DAT_10836e98.M_1015c6a0(&DAT_118d0ba8[DAT_10202478]);
                FUN_1013faf0();
                return (DAT_10202478 << CALLBACK_MOVIE_SLOT_SHIFT | DAT_10836e98.width) << CALLBACK_MOVIE_SLOT_SHIFT | DAT_10836e98.height;
            }
            if (code == SCENE_CB_DSHOW_NEXT_FRAME)
            {
                if ((DAT_104bf890 & 1) != 0)
                    return 1;
                if (DAT_10836e98.M_10148930() == 0)
                {
                    DAT_10836e98.M_1015c6a0(&DAT_118d0ba8[DAT_10202478]);
                    return 1;
                }
            }
            else
            {
                if (code == SCENE_CB_DSHOW_CLOSE)
                {
                    DAT_10836e98.M_10148920();
                    DAT_10836e98.Close();
                    return 1;
                }
                if (code == SCENE_CB_LOAD_MAP)
                {
                    if (DAT_104bfeb0 == 0)
                    {
                        int map = DAT_10874848.FindConstant((const char *)args);
                        if (map < 0)
                            return 1;
                        Vec4 start = DAT_108029b8;
                        DAT_10c62ed0 = map;
                        DAT_104c0096 = 1;
                        FUN_100fc4a0(map, 0, 0);
                        DAT_104c0096 = 0;
                        DAT_108029b8.Copy(start);
                        return 1;
                    }
                }
                else if (code == SCENE_CB_RESTART_COLOUR_PLUGS)
                {
                    Node *node = DAT_10255f08->Find((const char *)args, 0);
                    if (node != 0)
                        node->RestartColourPlugs((const char *)args + 0xfa, DAT_104bf8a4);
                }
                else if (code == SCENE_CB_PLAY_EFFECT_SOUND)
                    FUN_1003ad20(*args, 0, 1.0f);
                else if (code == SCENE_CB_PLAY_WAV)
                    FUN_1003ac20((const char *)args, 0, 1.0f);
                else if (code == SCENE_CB_PLAY_SOUND_AT_MODEL)
                {
                    if (args[1] < 0)
                        FUN_1003ad20(*args, 0, (float)args[2] / SCENE_CB_VOLUME_DIVISOR);
                    else
                    {
                        Vec4 position = DAT_10db6ee8[args[1]].matrix.GetTranslation();
                        position.z = position.z + SCENE_CB_SOUND_HEIGHT;
                        FUN_1003ad20(*args, &position, (float)args[2] / SCENE_CB_VOLUME_DIVISOR);
                    }
                }
                else if (code == SCENE_CB_PLAY_WAV_AT_MODEL)
                {
                    if (args[1] < 0)
                        FUN_1003ac20((const char *)*args, 0, (float)args[2] / SCENE_CB_VOLUME_DIVISOR);
                    else
                    {
                        Vec4 position = DAT_10db6ee8[args[1]].matrix.GetTranslation();
                        position.z = position.z + SCENE_CB_SOUND_HEIGHT;
                        FUN_1003ac20((const char *)*args, &position, (float)args[2] / SCENE_CB_VOLUME_DIVISOR);
                    }
                }
                else if (code == SCENE_CB_PLAY_SOUND_AT_POINT)
                {
                    Vec4 position;
                    position.x = (float)args[3];
                    position.y = (float)args[4];
                    position.z = (float)args[5];
                    FUN_1003ad20(*args, &position, (float)args[2] / SCENE_CB_VOLUME_DIVISOR);
                }
                else if (code == SCENE_CB_PLAY_WAV_AT_POINT)
                {
                    Vec4 position;
                    position.x = (float)args[3];
                    position.y = (float)args[4];
                    position.z = (float)args[5];
                    FUN_1003ac20((const char *)*args, &position, (float)args[2] / SCENE_CB_VOLUME_DIVISOR);
                }
                else if (code == SCENE_CB_PLAY_BGM)
                    Bgm_Play(*args, 0);
                else if (code == SCENE_CB_PLACE_EFFECT_MODEL)
                {
                    FUN_1003a7d0((const char *)*args, (float)args[1], (float)args[2], (float)args[3], -1, 0, 0, (float)args[4]);
                    FUN_1013faf0();
                }
                else if (code == SCENE_CB_START_EFFECT)
                {
                    if (args[4] != 0)
                    {
                        Matrix4 placement;
                        placement = RotationZ((float)args[5] * CALLBACK_PI / CALLBACK_HALF_TURN);
                        placement._41 = (float)args[1];
                        placement._42 = (float)args[2];
                        placement._43 = (float)args[3];
                        Vec4 position;
                        position.Fill(0.0f);
                        DAT_10daaa80.Start(*args, position, -1, -1, 0, 0, 0, &placement, 0, 0);
                    }
                    else
                    {
                        Vec4 position;
                        position.Set((float)args[1], (float)args[2], (float)args[3], 0);
                        DAT_10daaa80.Start(*args, position, -1, -1, 0, 0, 0, 0, 0, 0);
                    }
                }
                else if (code == SCENE_CB_START_EFFECT_AT_NODE)
                {
                    Vec4 position;
                    position.Fill(0.0f);
                    Matrix4 placement;
                    if (DAT_10db6ee8[args[1]].root != 0)
                    {
                        placement = RotationZ(DAT_10db6ee8[args[1]].heading);
                        Node *node = DAT_10db6ee8[args[1]].root->Find((const char *)args[2], 0);
                        if (node != 0)
                            DAT_10daaa80.Start(*args, position, -1, -1, &node->worldMatrix, 0, 0, &placement, 0, 0);
                    }
                }
                else if (code == SCENE_CB_RUN_VALUE_COMMAND)
                    FUN_101011b0(args[0], args[1]);
                else if (code == SCENE_CB_SET_GAME_FLAG)
                {
                    if (args[0] == CALLBACK_FLAG_OUTLINE_SLOT)
                    {
                        if (args[1] == 0 || DAT_10c68000.m_eac58[args[1]] == -1)
                            DAT_104c02c5 = 0;
                        else
                        {
                            DAT_104c02c5 = (unsigned char)DAT_10c68000.m_eac58[args[1]];
                            DAT_104c02c7 = CALLBACK_OUTLINE_FULL;
                        }
                    }
                    if (args[0] == CALLBACK_FLAG_SOUND_MUTE)
                        DAT_11b044e0 = args[1];
                    if (args[0] == CALLBACK_FLAG_OUTLINE_MODE)
                        DAT_104c02c6 = (unsigned char)args[1];
                    if (args[0] == CALLBACK_FLAG_OUTLINE_OPACITY)
                        DAT_104c02c7 = (unsigned char)args[1];
                    if (args[0] == CALLBACK_FLAG_HAND_TOOL)
                        FUN_10012780(args[1], 0);
                    if (args[0] == CALLBACK_FLAG_SWITCH_ON)
                    {
                        int kind = args[1];
                        if (FUN_10017aa0(kind) > 0)
                            FUN_1005f530(FUN_10017aa0(kind));
                    }
                    if (args[0] == CALLBACK_FLAG_SWITCH_OFF)
                    {
                        int kind = args[1];
                        if (FUN_10017aa0(kind) > 0)
                            FUN_1005f640(FUN_10017aa0(kind));
                    }
                    if (args[0] == CALLBACK_FLAG_CUTSCENE_RESET)
                    {
                        DAT_10c652cc = 1;
                        FUN_10012510(1, 0);
                        DAT_10c6356c = 0;
                        DAT_10c63570 = CALLBACK_FAR_REGISTER;
                        FUN_101011b0(CALLBACK_VALUE_HIDE_NODE, 0);
                        FUN_101011b0(CALLBACK_VALUE_COUNT_NODES, 0);
                        FUN_101011b0(CALLBACK_VALUE_NODE_GROUP, CALLBACK_GROUP_BELOW);
                        FUN_101011b0(CALLBACK_VALUE_4E2B, 100);
                        FUN_101011b0(CALLBACK_VALUE_4E2C, 1);
                    }
                    if (args[0] == CALLBACK_FLAG_FADE_ON_TOP)
                        DAT_10c68000.fadeOnTop = args[1];
                    if (args[0] == CALLBACK_FLAG_VALUE_DA7DD8)
                        DAT_10da7dd8 = (unsigned char)args[1];
                    if (args[0] == CALLBACK_FLAG_LIMITS)
                    {
                        if (args[1] != 0)
                        {
                            DAT_1083631c.x = CALLBACK_LIMIT_MIN;
                            DAT_1083631c.y = CALLBACK_LIMIT_MIN;
                            DAT_104c0224 = CALLBACK_LIMITS_ON;
                            DAT_1083631c.z = CALLBACK_LIMIT_MAX;
                            DAT_1083631c.w = CALLBACK_LIMIT_MAX;
                        }
                        else
                            DAT_104c0224 = 0;
                    }
                    if (args[0] == CALLBACK_FLAG_E74 && args[1] != 0)
                        DAT_104c0318 = 1;
                }
                else if (code == SCENE_CB_COPY_NODE_TO_PICTURES)
                {
                    for (int i = 0; i < CALLBACK_COPY_PARAMETER_COUNT; i++)
                    {
                        Node *node = DAT_10255f08->Find((const char *)args, i);
                        if (node == 0)
                            return 0;
                        node->ApplyCommandText((const char *)(args + 0x10));
                    }
                }
                else if (code == SCENE_CB_DETACH_NODE)
                {
                    Node *node = DAT_10255f08->Find((const char *)args, 0);
                    if (node != 0 && node->parent != 0)
                        node->parent->RemoveChild(node, 0);
                }
            }
        }
    }
    return 0;
}
