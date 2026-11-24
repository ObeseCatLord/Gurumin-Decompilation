// The game's script built-ins: one handler runs every built-in the script parser registers for the game (messages,
// shops and selection windows, waits, map and character commands, movies, the equipment list, the kusa nodes, the
// wallpaper). The parser passes the built-in's id, an array with a pointer to each argument and itself, which
// receives the result.

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game.h"
#include "game_vars.h"
#include "game_load_data.h"
#include "title_screen.h"
#include "map_vars.h"
#include "map_setup.h"
#include "menu_save_load.h"
#include "camera_vars.h"
#include "render_vars.h"
#include "script_vars.h"
#include "script_system.h"
#include "scene_command.h"
#include "scene_callback.h"
#include "effect_manager.h"
#include "effect_model.h"
#include "world_renderer.h"
#include "tracker.h"
#include "texture_object.h"
#include "texture_load_bmp.h"
#include "gimmick_slot.h"
#include "item_info.h"
#include "bgm_track.h"
#include "ui_vars.h"
#include "script_game_builtin.h"

extern "C" int FUN_1000ebf0(void);                     // opens the list window
extern "C" int FUN_10137b10(void);                     // Screenshot_Capture
extern "C" int FUN_100114a0(void);
extern "C" int FUN_1013f880(void);                     // the game's random number
extern "C" int FUN_1000db30(int id, int value, int frames);
extern "C" int FUN_1000da40(int id, int value, int frames);
extern "C" int FUN_1000da20(int text);
extern "C" int FUN_1000ed70(const char *text, const char *subtext, int frames);
extern "C" int FUN_1000c020(char *out, const char *text);     // Text_Substitute
extern "C" void FUN_1000ec70(int frames, float r, float g, float b, float a);   // ColourFade_Start
extern "C" int FUN_10056260(int slot, const char *line1, const char *line2, const char *line3, const char *line4);
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);    // Sound_PlayEffect
extern "C" int FUN_1003a880(const char *path, float x, float y, float z, int start, float ox, float oy, float oz,
                            const Vec4 &colour);
extern "C" int FUN_1000dd90(float x, float y, float z, int type);              // Debris_Spawn
extern "C" void FUN_1003bc60(int x, int y, int mode, int radius);                                    // Ground_Dig
extern "C" int FUN_1016afa0(int mode, int r, int g, int b);
extern "C" int FUN_1016b230(const char *text);
extern "C" int FUN_10012130(int *item, int *count);                            // Inventory_Add
extern "C" int FUN_10012450(int index, int attach, const char *name);
extern "C" int FUN_10012510(int mode, int index);
extern "C" void FUN_100123d0(void);
extern "C" void FUN_100102e0(void);
extern "C" int FUN_10012100(int value);
extern "C" int FUN_10014d30(int item);
extern "C" int FUN_100154c0(int item);
extern "C" int FUN_10016580(const char *name);                                 // Movie_PlayToTexture
extern "C" int FUN_10017aa0(int id);                                           // Enemy_FindByKind
extern "C" int FUN_10017ad0(int id);                                           // Enemy_FindById
extern "C" int FUN_10017bb0(int id);
#include "value_command.h"
extern "C" int FUN_1013faf0(void);                                             // FrameTimer_ResetNext2
extern "C" int FUN_1013fb10(int active, int target, int frames);
extern "C" const char *FUN_1013f8a0(const char *name, int folder);             // builds the data path of `name`
extern "C" int FUN_10092af0(int code, int value);                              // scene callback
extern "C" int FUN_1015c9e0(int mode);                                         // Screen_StartTransition
extern "C" int FUN_1015d500(int value);
extern "C" int FUN_1015f310(char *text);
extern "C" int FUN_1015fd30(void);
extern "C" int FUN_10161120(char *out, const char *path);                      // Path_GetBaseName
extern "C" void FUN_101b2580(void);                                            // starts the movie library
extern "C" void FUN_101b2610(void);                                            // closes it
extern "C" void FUN_101b27d0(const char *path);                                // opens a movie file in it
extern "C" int FUN_1000e750(void);                                             // AntiTamper_Store
extern "C" void FUN_1000e810(void);
extern "C" void FUN_1003b6e0(int index, int variant);                          // Costume_LoadPlayerModel
extern "C" unsigned int FUN_100144c0(unsigned int flags, char *text);          // Ability_GetDescription
extern "C" int FUN_10017a60(const void *first, const void *second);            // compares two item numbers
extern "C" int FUN_100fc4a0(int map, const char *startMarker, int heightOffset);   // Map_SetupByNumber
extern "C" int FUN_101008e0(GameTrigger *trigger);                            // queued event callback

// Adds item `itemNumber` to the equipment list when the player has it, it has a name and is a helmet (type 1).
#define LIST_HELMET(itemNumber) \
    if (DAT_103150c0[itemNumber] != 0 && DAT_1088e650[itemNumber].name[0] != 0 && DAT_1088e6b0[itemNumber].type == 1) \
        DAT_1042e2a8[count++] = (itemNumber);

// Sets `cost` to what the next level of a helmet that is at level `level` costs.
#define LEVEL_COST(level, cost) \
    cost = 0x18696; \
    if ((level) < 1) \
        cost = 10; \
    if ((level) == 1) \
        cost = 0x32; \
    else if ((level) == 2) \
        cost = 0xfa; \
    else if ((level) == 3) \
        cost = 999; \
    else if ((level) > 3) \
        cost = 0;

// Spawns one piece of the kusa firework of type `type` over `position`.
#define KSK_PIECE(position, type) FUN_1000dd90((position).x, (position).y, (position).z + 50.0f, type)

// 0x10138280 (Script_GameBuiltin). Runs the game built-in `id` (a GAME_BUILTIN_* value) with the arguments `args`
// (one pointer to the value of each argument, or the text itself for a string); the result is left in `bank` with
// its type. Ids this does not know do nothing.
void Script_GameBuiltin(int id, ScriptArgument *args, ListBank *bank)
{
    switch (id)
    {
    case GAME_BUILTIN_MESSAGE:
        FUN_10056260(*args[0].i, args[1].text, args[2].text, args[3].text, args[4].text);
        DAT_104bfb08[*args[0].i] = 0xb;
        DAT_104bfd10 = 1;
        while (DAT_104bfd10 != 0 && FUN_10137b10() != 0)
            ;
        break;
    case GAME_BUILTIN_SHOP_START:
        DAT_10202350 = 0;
        DAT_104bfd24 = 0;
        break;
    case GAME_BUILTIN_SHOP_ITEM:
        {
            char itemName[0x100];
            int *item = args[0].i;
            strcpy(itemName, DAT_1088e650[*item].name);
            sprintf(DAT_10450808[DAT_104bfd24], "%ss112\x81\x90%d", itemName, DAT_1088e6b0[*item].price);
            DAT_1042e2a8[DAT_104bfd24] = *args[0].i;
            DAT_104bfd24++;
        }
        break;
    case GAME_BUILTIN_SHOP_END:
        DAT_10254590 = 1;
        DAT_104bfd20 = 1;
        FUN_1000ebf0();
        DAT_10467f20 = 0;
        DAT_104bfd10 = 1;
        while (DAT_104bfd10 != 0 && FUN_10137b10() != 0)
            ;
        break;
    case GAME_BUILTIN_SEL_START:
        DAT_104bfd24 = 0;
        DAT_10202350 = *args[0].i;
        break;
    case GAME_BUILTIN_SEL_TEXT:
        FUN_1000c020(DAT_10450808[DAT_104bfd24], args[0].text);
        DAT_1042e2a8[DAT_104bfd24] = *args[1].i;
        DAT_104bfd24++;
        break;
    case GAME_BUILTIN_SHOP_END2:
        DAT_10254590 = 1;
        DAT_104bfd20 = 2;
        FUN_1000ebf0();
        DAT_10467f20 = 0;
        DAT_104bfd10 = 1;
        while (DAT_104bfd10 != 0 && FUN_10137b10() != 0)
            ;
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = DAT_10202350;
        break;
    case GAME_BUILTIN_SEL_END:
        DAT_10254590 = 1;
        DAT_104bfd20 = 0;
        FUN_1000ebf0();
        DAT_10467f20 = 0;
        DAT_104bfd10 = 1;
        while (DAT_104bfd10 != 0 && FUN_10137b10() != 0)
            ;
        break;
    case GAME_BUILTIN_RECOVER:
        {
            const Vec4 &position = DAT_10db9210.GetTranslation();
            int sound = 0x83 + FUN_1013f880() % 2;
            if (DAT_104c0332 == 0)
                FUN_1003ad20(sound, &position, 1.0f);
            int amount = *args[1].i;
            int minimum = DAT_10daaae8[0].stats[STAT_MAX_HEALTH] * amount / 100;
            if (amount < minimum)
                amount = minimum;
            DAT_10daaae8[0].stats[STAT_HEALTH] += amount;
            if (DAT_10daaae8[0].stats[STAT_HEALTH] > DAT_10daaae8[0].stats[STAT_MAX_HEALTH])
                DAT_10daaae8[0].stats[STAT_HEALTH] = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
            FUN_1000e750();
            rand();
        }
        break;
    case GAME_BUILTIN_WAIT_DISARMED:
        {
            DAT_104c02ed = 0;
            DAT_10db6f78.Fill(0.0f);
            DAT_10db6ee8[0].Teleport(DAT_10db9240, DAT_10db9244, DAT_10db9248);
            FUN_100114a0();
            unsigned int until = *args[0].i + DAT_104bf890;
            while (until > DAT_104bf890 && FUN_10137b10() != 0)
                ;
        }
        break;
    case GAME_BUILTIN_WAIT:
        {
            FUN_100114a0();
            unsigned int until = *args[0].i + DAT_104bf890;
            while (until > DAT_104bf890 && FUN_10137b10() != 0)
                ;
        }
        break;
    case GAME_BUILTIN_GET_COSTUME_UNLOCKED:
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = (unsigned short)DAT_104c0230 >> (char)*args[0].i & 1;
        break;
    case GAME_BUILTIN_CROSS_FADE_ALIAS:
    case GAME_BUILTIN_CROSS_FADE:
        FUN_1015c9e0(GAME_BUILTIN_CROSS_FADE_MODE);
        break;
    case GAME_BUILTIN_SE:
        FUN_1003ad20(*args[0].i, 0, 1.0f);
        break;
    case GAME_BUILTIN_LOAD_MAP:
        DAT_10c62ed0 = *args[0].i;
        FUN_100fc4a0(DAT_10c62ed0, 0, 0);
        DAT_1101d8a8 = 0xf;
        DAT_1101de30 = 1;
        DAT_11018734 = 1;
        DAT_104c0092 = 0;
        DAT_104c0093 = 0;
        break;
    case GAME_BUILTIN_CHAR_POS:
        DAT_10db6ee8[*args[0].i].Teleport((float)*args[1].i, (float)*args[2].i, (float)(*args[3].i - 5));
        break;
    case GAME_BUILTIN_MAP_ENABLE:
        DAT_104bfd50 = *args[0].i;
        break;
    case GAME_BUILTIN_LOCK:
        DAT_104bfdb8 = 1;
        break;
    case GAME_BUILTIN_UNLOCK:
        DAT_104bfdb8 = 0;
        if (DAT_104c0088 != 0)
            DAT_104c0088 = 0;
        break;
    case GAME_BUILTIN_WALK:
        DAT_104c0024 = 0;
        DAT_102559f0 = *args[0].i;
        DAT_1042d9e4 = *args[1].i;
        DAT_104bfd0c = *args[2].i;
        break;
    case GAME_BUILTIN_BLACK:
        FUN_1000da40(*args[0].i, *args[1].i, *args[2].i);
        break;
    case GAME_BUILTIN_LOOK:
        {
            int character = *args[0].i;
            int target = *args[1].i;
            Vec4 targetPosition;
            Vec4 characterPosition;
            if (target > 0)
                targetPosition.Copy(DAT_10db6ee8[target].matrix.GetTranslation());
            else
                targetPosition.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            characterPosition.Copy(DAT_10db6ee8[character].matrix.GetTranslation());
            targetPosition.Copy(targetPosition - characterPosition);
            targetPosition.Copy(Vec3_Normalized(&targetPosition));
            targetPosition.Copy(targetPosition * 6.0f);
            DAT_10db6ee8[character].targetHeading = (float)atan2(targetPosition.x, targetPosition.y) + 3.1415f;
        }
        break;
    case GAME_BUILTIN_BGM:
        DAT_104c012c = *args[0].i;
        Bgm_Play(*args[0].i, 0);
        break;
    case GAME_BUILTIN_EVENT:
        DAT_10874848.RegisterEvent(*args[0].i);
        DAT_10874848.RunQueuedEvents(FUN_101008e0);
        break;
    case GAME_BUILTIN_VISIBLE:
        DAT_10db6ee8[*args[0].i].shadowHidden = *args[1].i == 0;
        break;
    case GAME_BUILTIN_MAP_EVENT:
        DAT_1020235c = *args[0].i;
        break;
    case GAME_BUILTIN_WINDOW_MESSAGE:
        FUN_1000ed70(args[0].text, args[1].text, GAME_BUILTIN_WINDOW_FRAMES);
        break;
    case GAME_BUILTIN_SCOP:
        FUN_10012510(*args[0].i, 0);
        break;
    case GAME_BUILTIN_PLAY_MOVIE:
        FUN_100114a0();
        DAT_104bfeb0 = 0;
        DAT_104c0088 = 1;
        DAT_104bfe6c = 1;
        DAT_104bfeb4 = *args[1].i;
        DAT_104bfe20 = 0;
        DAT_104c0309 = 0;
        DAT_104bfe70 = 0;
        DAT_104c02c5 = 0;
        DAT_10f654f8 = 0;
        DAT_10f654fc = 0;
        DAT_10c68000.Bind(DAT_10db6ee8, DAT_10255f08, &DAT_104bfe68, FUN_10092af0, &DAT_1020e538);
        DAT_10203420 = 1.0f;
        DAT_10c68000.LoadCharacter(FUN_1013f8a0(args[0].text, 0));
        if (DAT_10c655f0 == 6)
            DAT_10c68974 = 1;
        DAT_10c66d00 = 0;
        DAT_10c68000.Seek(0);
        FUN_1013faf0();
        FUN_1000db30(0, 0, 0);
        DAT_1020d9e0.M_100014a0();
        {
            int savedLock = DAT_104bfdb8;
            DAT_104bfdb8 = 0;
            while (DAT_104bfe6c != 0 && FUN_10137b10() != 0)
                ;
            FUN_1013faf0();
            FUN_100102e0();
            DAT_10203420 = 1.0f;
            DAT_104c0318 = 0;
            DAT_104bfdb8 = savedLock;
        }
        FUN_100123d0();
        rand();
        for (int i = 0; i < CHAR_SLOT_COUNT; i++)
        {
            memset(&DAT_1020b430[i], 0, sizeof(CharState));
            DAT_10210b30[i] = 0;
            DAT_10254090[i] = 1;
            DAT_10db6ee8[i].m_0294 = 0;
            DAT_10db6ee8[i].M_10161be0(0, 0);
            DAT_10db6ee8[i].M_10161f60(0, 0);
        }
        DAT_1101de30 = 1;
        DAT_11018734 = 1;
        if (DAT_104bfdb8 == 0)
            DAT_104c0088 = 0;
        DAT_1032d868 = DAT_10219bb8;
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = DAT_104c0309 == 0;
        FUN_1013faf0();
        break;
    case GAME_BUILTIN_RECOVER_ALL:
        DAT_10daaae8[0].stats[STAT_HEALTH] = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
        FUN_1000e750();
        break;
    case GAME_BUILTIN_CHAR_EFFECT:
        {
            Vec4 position = DAT_10db6ee8[*args[0].i].matrix.GetTranslation();
            DAT_10daaa80.Start(*args[1].i, position, -1, 0, 0, 0, 0, 0, 0, 0);
        }
        break;
    case GAME_BUILTIN_BLUR:
        {
            // the original reads the character's position here and does not use it
            Vec4 position = DAT_10db6ee8[*args[0].i].matrix.GetTranslation();
            FUN_1013fb10(*args[0].i, *args[1].i, *args[2].i);
        }
        break;
    case GAME_BUILTIN_MMV:
        FUN_10016580(args[0].text);
        break;
    case GAME_BUILTIN_EQUIP:
        {
            int item = *args[0].i;
            if (DAT_1088e6b0[item].type != 1)
                DAT_10daabb8 = DAT_10daabb8 == item ? 0 : item;
            else if (DAT_10daabb4 == item)
            {
                DAT_10daabb4 = 0;
                FUN_100154c0(0);
            }
            else
            {
                DAT_10daabb4 = item;
                FUN_100154c0(*args[0].i);
            }
        }
        break;
    case GAME_BUILTIN_CAMERA:
        FUN_1016afa0(*args[0].i, *args[1].i, *args[2].i, *args[3].i);
        break;
    case GAME_BUILTIN_WALK2:
        DAT_104c0024 = *args[0].i;
        DAT_102559f0 = *args[1].i;
        DAT_1042d9e4 = *args[2].i;
        DAT_104bfd0c = *args[3].i;
        break;
    case GAME_BUILTIN_SET_WALK_TARGET:
        DAT_104bfe20 = *args[0].i >= 0;
        DAT_102024ac = *args[0].i;
        break;
    case GAME_BUILTIN_GET_ITEM:
        {
            int item = *args[0].i;
            int count = 1;
            FUN_10012130(&item, &count);
        }
        break;
    case GAME_BUILTIN_MES:
        FUN_1000ed70(args[0].text, 0, GAME_BUILTIN_MES_FRAMES);
        break;
    case GAME_BUILTIN_SET_MAP_COLOR:
        FUN_1000ec70(*args[0].i, *args[1].f, *args[2].f, *args[3].f, 1.0f);
        break;
    case GAME_BUILTIN_POST_SCR:
        FUN_1000da20(*args[0].i);
        DAT_104c0334 = 5;
        break;
    case GAME_BUILTIN_LOCK_COMMAND:
        DAT_104c0028 = *args[0].i;
    case GAME_BUILTIN_SET_COMMAND_LOCK:
        DAT_104c002c = *args[0].i;
        break;
    case GAME_BUILTIN_SET_CHARACTER_KIND:
        if (*args[1].i != 0)
        {
            DAT_10daaae8[*args[0].i].stats[STAT_MAX_HEALTH] = 100;
            DAT_10daaae8[*args[0].i].stats[STAT_HEALTH] = 100;
            DAT_10daaae8[*args[0].i].held = 1;
            DAT_10daaae8[*args[0].i].m_0141 = 0;
            DAT_1021f3c0[*args[0].i] = 99;
        }
        else
            DAT_1021f3c0[*args[0].i] = 2;
        break;
    case GAME_BUILTIN_GET_MP:
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = DAT_10daaae8[0].stats[STAT_PICKUPS];
        break;
    case GAME_BUILTIN_SAVE_MAP:
        DAT_104c0034 = DAT_10c62ed0;
        for (int i = 0; i < CHAR_SLOT_COUNT; i++)
        {
            DAT_10837fc8[i].Copy(DAT_10db6ee8[i].matrix.GetTranslation());
            DAT_10837fc8[i].w = DAT_10db6ee8[i].targetHeading;
        }
        break;
    case GAME_BUILTIN_RESTORE_MAP:
        DAT_10c62ed0 = DAT_104c0034;
        DAT_1021cff8 = 1;
        DAT_1042de20 = DAT_104c0034;
        DAT_1030c900 = 0;
        DAT_103086a8 = 0;
        DAT_1101d8a8 = 0xf;
        DAT_1101de30 = 1;
        DAT_11018734 = 1;
        for (int i = 0; i < CHAR_SLOT_COUNT; i++)
        {
            if (i == 0 || DAT_1021f3c0[i] == 2 || DAT_1021f3c0[i] == 3)
            {
                DAT_10db6ee8[i].SetPosition(DAT_10837fc8[i].x, DAT_10837fc8[i].y, DAT_10837fc8[i].z);
                DAT_10db6ee8[i].heading = DAT_10837fc8[i].w;
                DAT_10db6ee8[i].targetHeading = DAT_10837fc8[i].w;
            }
        }
        FUN_1015c9e0(0);
        break;
    case GAME_BUILTIN_WAIT2:
        {
            unsigned int until = *args[0].i + DAT_104bf890;
            while (until > DAT_104bf890 && FUN_10137b10() != 0)
                ;
        }
        break;
    case GAME_BUILTIN_WAIT_COMMAND:
        if (DAT_104c0338 == 0)
        {
            DAT_104c0339 = 1;
            unsigned int until = *args[0].i + DAT_104bf890;
            while (until > DAT_104bf890 && DAT_104c0339 != 0 && FUN_10137b10() != 0)
                ;
        }
        break;
    case GAME_BUILTIN_PLAY_MOVIE2:
        FUN_100114a0();
        DAT_104bfeb0 = 0;
        DAT_104c0088 = 1;
        DAT_104bfe70 = 0;
        DAT_104bfe6c = 1;
        DAT_104bfeb4 = *args[1].i;
        DAT_104bfe20 = 0;
        DAT_104c0309 = 0;
        DAT_104c02c5 = 0;
        DAT_10f654f8 = 0;
        DAT_10f654fc = 0;
        DAT_10c68000.Bind(DAT_10db6ee8, DAT_10255f08, &DAT_104bfe68, FUN_10092af0, &DAT_1020e538);
        DAT_10203420 = 1.0f;
        DAT_10c68000.LoadCharacter(FUN_1013f8a0(args[0].text, 0));
        if (DAT_10c655f0 == 6)
            DAT_10c68974 = 1;
        DAT_10c66d00 = 0;
        DAT_10c68000.Seek(0);
        FUN_1013faf0();
        FUN_1000db30(0, 0, 0);
        DAT_1020d9e0.M_100014a0();
        {
            int savedLock = DAT_104bfdb8;
            DAT_104bfdb8 = 0;
            while (DAT_104bfe6c != 0 && FUN_10137b10() != 0)
                ;
            DAT_10203420 = 1.0f;
            FUN_100102e0();
            FUN_1013faf0();
            DAT_104c0318 = 0;
            DAT_104bfdb8 = savedLock;
        }
        FUN_100123d0();
        rand();
        for (int i = 0; i < CHAR_SLOT_COUNT; i++)
        {
            memset(&DAT_1020b430[i], 0, sizeof(CharState));
            DAT_10210b30[i] = 0;
            DAT_10254090[i] = 1;
            DAT_10db6ee8[i].m_0294 = 0;
            DAT_10db6ee8[i].M_10161be0(0, 0);
            DAT_10db6ee8[i].M_10161f60(0, 0);
        }
        DAT_1101de30 = 1;
        DAT_11018734 = 1;
        if (DAT_104bfdb8 == 0)
            DAT_104c0088 = 0;
        DAT_1032d868 = DAT_10219bb8;
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = DAT_104c0309 == 0;
        FUN_1013faf0();
        break;
    case GAME_BUILTIN_LEVEL_UP:
        DAT_102024a8 = *args[0].i;
        break;
    case GAME_BUILTIN_SET_SAVE_POS:
        DAT_1030b660 = DAT_10c62ed0;
        DAT_108029b8.Copy(DAT_10db9210.GetTranslation());
        DAT_108029b8.w = DAT_10db9364;
        break;
    case GAME_BUILTIN_RESET_FADE:
        FUN_1015c9e0(0);
        break;
    case GAME_BUILTIN_GET_GP:
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = DAT_10daaae8[0].coins;
        break;
    case GAME_BUILTIN_SET_GP:
        DAT_10daaae8[0].coins = *args[0].i;
        FUN_1000e750();
        break;
    case GAME_BUILTIN_KAGU_TEST:
        Script_ValueCommand(*args[0].i, *args[1].i);
        break;
    case GAME_BUILTIN_KAGU_TEST2:
        Scene_RunCommand(*args[0].i, *args[1].i, args[2].text);
        break;
    case GAME_BUILTIN_WHITE:
        FUN_1000db30(*args[0].i, *args[1].i, *args[2].i);
        break;
    case GAME_BUILTIN_ERASE_FLOOR:
        for (int x = GAME_BUILTIN_DIG_START; x < GAME_BUILTIN_DIG_END; x += GAME_BUILTIN_DIG_STEP)
            for (int y = GAME_BUILTIN_DIG_START; y < GAME_BUILTIN_DIG_END; y += GAME_BUILTIN_DIG_STEP)
                FUN_1003bc60(*args[0].i + x, *args[1].i + y, 0, 0x3e8);
        break;
    case GAME_BUILTIN_REFFECT:
        {
            Vec4 position;
            position.Set((float)*args[1].i, (float)*args[2].i, (float)*args[3].i, 0);
            DAT_10daaa80.Start(*args[0].i, position, -1, 0, 0, 0, 0, 0, 0, 0);
        }
        break;
    case GAME_BUILTIN_SEND_EVENT:
        DAT_10874848.RegisterEvent(*args[0].i);
        while (DAT_10874848.RunQueuedEvents(FUN_101008e0) != 0)
            ;
        break;
    case GAME_BUILTIN_LIFT_ENABLE:
        DAT_102024b4 = (char)*args[0].i;
        break;
    case GAME_BUILTIN_SPECIAL:
        if (*args[0].i == 1)
        {
            if (DAT_104c00b6 == 0)
                DAT_104c0093 = DAT_104c00b4 * 5;
            else if (DAT_104c00b6 == 1)
                DAT_104c0093 = DAT_104c00b4 << 3;
            else if (DAT_104c00b6 == 2)
                DAT_104c0093 = DAT_104c00b4 * 10;
        }
        if (*args[0].i == 2)
        {
            if (DAT_104c00b6 == 0)
                DAT_104c0092 = DAT_104c00b4;
            else if (DAT_104c00b6 == 1)
                DAT_104c0092 = DAT_104c00b4 * 3;
            else if (DAT_104c00b6 == 2)
                DAT_104c0092 = DAT_104c00b4 * 10;
        }
        break;
    case GAME_BUILTIN_LIFT_2C:
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = FUN_10017ad0(*args[0].i);
        break;
    case GAME_BUILTIN_DOOR_2C:
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = FUN_10017aa0(*args[0].i);
        break;
    case GAME_BUILTIN_TAKARA_2C:
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = FUN_10017bb0(*args[0].i);
        break;
    case GAME_BUILTIN_ENEMY_STOP:
        DAT_104c0094 = (char)*args[0].i;
        break;
    case GAME_BUILTIN_GET_CHAR_POS:
        {
            Vec4 position;
            position.Copy(DAT_10db6ee8[*args[0].i].matrix.GetTranslation());
            *args[1].i = (int)position.x;
            *args[2].i = (int)position.y;
            *args[3].i = (int)position.z;
        }
        break;
    case GAME_BUILTIN_LIFT_ENABLE2:
        DAT_1020b430[*args[0].i].sawPlayer = *args[1].i == 0;
        break;
    case GAME_BUILTIN_GET_BT15:
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = DAT_1020b430[*args[0].i].behaviour;
        break;
    case GAME_BUILTIN_SCOP2:
        FUN_10012450(*args[0].i, *args[1].i, args[2].text);
        break;
    case GAME_BUILTIN_SAVE_CAMERA:
        DAT_10da9150.Copy(DAT_118e69e8);
        DAT_10868818.Copy(DAT_118e6cb8);
        break;
    case GAME_BUILTIN_RESTORE_CAMERA:
        DAT_118e69e8.Copy(DAT_10da9150);
        DAT_118e6cb8.Copy(DAT_10868818);
        DAT_11af3e28.Copy(DAT_10da9150);
        DAT_118e6c78.Copy(DAT_10868818);
        break;
    case GAME_BUILTIN_MISS:
        bank->resultType = SCRIPT_TYPE_INT;
        *bank->result = (unsigned char)DAT_104c00a8;
        break;
    case GAME_BUILTIN_RESET:
        DAT_104bfd84 = 1;
        break;
    case GAME_BUILTIN_SET_SAVE_POS2:
        DAT_108029b8.x = (float)*args[1].i;
        DAT_108029b8.y = (float)*args[2].i;
        DAT_108029b8.z = (float)*args[3].i;
        DAT_1030b660 = *args[0].i;
        break;
    case GAME_BUILTIN_GAME_OVER:
        DAT_104c00cc = (char)*args[0].i;
        break;
    case GAME_BUILTIN_DEMO:
        DAT_104c0200 = 2;
        strcpy(DAT_102026d8, args[0].text);
        break;
    case GAME_BUILTIN_LOAD_MAP_B:
        DAT_10c62ed0 = *args[0].i;
        DAT_104c0096 = 1;
        FUN_100fc4a0(DAT_10c62ed0, 0, 0);
        DAT_104c0096 = 0;
        DAT_1101d8a8 = 0xf;
        DAT_11018734 = 1;
        DAT_1101de30 = 1;
        DAT_104c0092 = 0;
        DAT_104c0093 = 0;
        break;
    case GAME_BUILTIN_STAGE_NAME:
        {
            char text[0x124];
            sprintf(text, "\"%s\"", DAT_10874e48[*args[0].i]);
            SetTextResult(bank, text);
        }
        break;
    case GAME_BUILTIN_STAGE_MMV:
        {
            char text[0x124];
            sprintf(text, "\"%s\"", DAT_10875e48[*args[0].i]);
            SetTextResult(bank, text);
        }
        break;
    case GAME_BUILTIN_RELOAD_CHAR:
        {
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
        }
        break;
    case GAME_BUILTIN_PLAY_MMV3:
        {
            FUN_100114a0();
            DAT_104bfeb0 = 0;
            DAT_104c0088 = 1;
            DAT_104bfe6c = 2;
            DAT_104bfeb4 = *args[1].i;
            DAT_104bfe20 = 0;
            FUN_101b2580();
            FUN_101b27d0(FUN_1013f8a0(args[0].text, 0));
            DAT_118d0ba8[DAT_10202478].Write16(0, 0, DAT_11b1bb58, DAT_11b1bb64, DAT_11b1bb58 * 2, DAT_11b1bb54);
            DAT_1020d9e0.M_100014a0();
            int savedLock = DAT_104bfdb8;
            DAT_104bfdb8 = 0;
            while (DAT_104bfe6c != 0 && FUN_10137b10() != 0)
                ;
            FUN_101b2610();
            DAT_104bfdb8 = savedLock;
            if (savedLock == 0)
                DAT_104c0088 = 0;
        }
        break;
    case GAME_BUILTIN_PLAY_AVI:
        FUN_100114a0();
        DAT_104bfeb0 = 0;
        DAT_104bfe20 = 0;
        if (DAT_104c0340 == 0)
        {
            if (DAT_10836e98.Open(FUN_1013f8a0(args[0].text, 0)) == 0)
            {
                DAT_104c0088 = 1;
                DAT_104bfe6c = 3;
                DAT_104bfeb4 = *args[1].i;
                DAT_10836e98.M_10148910();
                Bgm_Play(1, 0);
                if (DAT_10836e98.M_10148930() == 0)
                {
                    DAT_10836e98.M_10148960();
                    DAT_10836e98.M_1015c6a0(&DAT_118d0ba8[DAT_10202478]);
                }
                DAT_1020d9e0.M_100014a0();
                int savedLock = DAT_104bfdb8;
                DAT_104bfdb8 = 0;
                DAT_10f654f0 = 1;
                while (DAT_104bfe6c != 0 && FUN_10137b10() != 0)
                    ;
                DAT_10f654f0 = 0;
                DAT_10836e98.Close();
                if (savedLock == 0)
                    DAT_104c0088 = 0;
                DAT_104bfdb8 = savedLock;
                rand();
            }
        }
        break;
    case GAME_BUILTIN_FILL:
        DAT_10f654f8 = *args[0].i;
        DAT_10f654fc = *args[1].f;
        break;
    case GAME_BUILTIN_CUSTOM:
        {
            DAT_10254590 = 1;
            DAT_104bfd20 = 3;
            DAT_104c02d0 = 0;
            int count = 0;
            for (int item = 0; item < GAME_BUILTIN_MAX_ITEMS; item += GAME_BUILTIN_ITEMS_PER_BLOCK)
            {
                LIST_HELMET(item)
                LIST_HELMET(item + 1)
                LIST_HELMET(item + 2)
                LIST_HELMET(item + 3)
                LIST_HELMET(item + 4)
                LIST_HELMET(item + 5)
                LIST_HELMET(item + 6)
                LIST_HELMET(item + 7)
            }
            qsort(DAT_1042e2a8, count, 4, FUN_10017a60);
            DAT_104bfd24 = count;
            FUN_1000ebf0();
            DAT_10467f20 = 0;
            DAT_104bfd10 = 1;
            while (DAT_104bfd10 != 0 && FUN_10137b10() != 0)
                ;
            bank->resultType = SCRIPT_TYPE_INT;
            *bank->result = DAT_104c02d0;
        }
        break;
    case GAME_BUILTIN_CUSTOM2:
        {
            char itemName[0x100];
            char text[0x124];
            int item = DAT_104c02d0;
            int level = DAT_10874848.TestScriptFlag(DAT_1088e6b0[item].levelFlag + GAME_BUILTIN_LEVEL_FLAG_BASE);
            int cost;
            LEVEL_COST(level, cost)
            if (*args[0].i == 0)
            {
                strcpy(itemName, DAT_1088e650[item].name);
                sprintf(text, "\"%s\"", itemName);
                SetTextResult(bank, text);
            }
            else if (*args[0].i == 1)
            {
                sprintf(itemName, "%d", cost);
                sprintf(text, "\"%s\"", itemName);
                SetTextResult(bank, text);
            }
        }
        break;
    case GAME_BUILTIN_CUSTOM3:
        if (*args[0].i == 0)
        {
            int count = 0;
            for (int item = 0; item < GAME_BUILTIN_MAX_ITEMS; item += GAME_BUILTIN_ITEMS_PER_BLOCK)
            {
                LIST_HELMET(item)
                LIST_HELMET(item + 1)
                LIST_HELMET(item + 2)
                LIST_HELMET(item + 3)
                LIST_HELMET(item + 4)
                LIST_HELMET(item + 5)
                LIST_HELMET(item + 6)
                LIST_HELMET(item + 7)
            }
            bank->resultType = SCRIPT_TYPE_INT;
            *bank->result = count;
        }
        if (*args[0].i == 1)
        {
            int item = DAT_104c02d0;
            DAT_104c02d4 = FUN_10014d30(item);
            int flag = DAT_1088e6b0[item].levelFlag + GAME_BUILTIN_LEVEL_FLAG_BASE;
            int level = DAT_10874848.TestScriptFlag(flag);
            int cost;
            LEVEL_COST(level, cost)
            level++;
            if (level > 3)
                level = GAME_BUILTIN_MAX_LEVEL;
            DAT_10874848.SetScriptFlag(flag, level);
            DAT_10c6670c -= cost;
            if (DAT_10daabb4 == item)
                DAT_104c02c8 = FUN_10014d30(item);
            DAT_104c02d8 = FUN_10014d30(item);
            FUN_1000e750();
            FUN_1000e810();
        }
        if (*args[0].i == 2)
        {
            bank->resultType = SCRIPT_TYPE_INT;
            *bank->result = 0;
            if (DAT_104c02d8 != 0)
            {
                unsigned int learned = DAT_104c02d4;
                unsigned int pending = DAT_104c02d8;
                char abilityText[0x124];
                char message[0x100];
                do
                {
                    unsigned int ability = FUN_100144c0(pending, abilityText);
                    if (ability == 0)
                        break;
                    if ((ability & learned) != ability)
                    {
                        DAT_104c02d4 = learned;
                        DAT_104c02d8 = pending;
                        rand();
                        sprintf(message, "%s\x82\xcc\x8c\xf8\x89\xca\x82\xaa\x92\xc7\x89\xc1\x82\xb3\x82\xea\x82\xbd\x81I\x81I", abilityText);
                        FUN_10012100(DAT_104c02d0);
                        FUN_1000ed70(message, "", GAME_BUILTIN_WINDOW_FRAMES);
                        bank->resultType = SCRIPT_TYPE_INT;
                        *bank->result = 1;
                        DAT_104c02d4 &= ~ability;
                        DAT_104c02d8 &= ~ability;
                        return;
                    }
                    pending &= ~ability;
                    learned &= ~ability;
                } while (pending != 0);
                DAT_104c02d4 = learned;
                DAT_104c02d8 = pending;
            }
        }
        break;
    case GAME_BUILTIN_KSK:
        {
            int number = *args[0].i;
            if (DAT_1021fa40[number] == 0)
                return;
            DAT_10219970[number] = 0;
            DAT_10255a48[number] = 0;
            if (DAT_10253e58[number] == 0)
                FUN_1016b230("656565");
            FUN_1003ad20(0x33a, &DAT_1020af20[number]->matrix.GetTranslation(), 1.0f);
            Vec4 position = DAT_1020af20[number]->matrix.GetTranslation();
            float heading = DAT_1020af20[number]->matrix.M_101b0a80();
            char path[0x40];
            sprintf(path, "ksk%02d.it3", DAT_1021d0b8[number]);
            Vec4 colour;
            colour.Fill(1.0f);
            if (DAT_10c655e0 == 0 && (DAT_10c62ed0 == 0x66 || DAT_10c62ed0 == 0x67 || DAT_10c62ed0 == 0x68))
                colour.Set(0.99145198f, 1.0f, 0.83760703f, 1.0f);
            if (DAT_10c655e0 != 0 && (DAT_10c62ed0 == 0x6f || DAT_10c62ed0 == 0x70 || DAT_10c62ed0 == 0x71))
                colour.Set(0.80626798f, 1.0f, 1.0f, 1.0f);
            if (DAT_10c655e0 == 0 && (DAT_10c62ed0 == 0xcb || DAT_10c62ed0 == 0xcc || DAT_10c62ed0 == 0xcd))
                colour.Set(0.80056900f, 1.0f, 0.96296197f, 1.0f);
            if (DAT_10c655e0 != 0 && (DAT_10c62ed0 == 0xc9 || DAT_10c62ed0 == 0xca))
                colour.Set(0.55270702f, 1.0f, 1.0f, 1.0f);
            if (DAT_10c655e0 == 0 && (DAT_10c62ed0 == 0x1f7 || DAT_10c62ed0 == 0x1f8))
                colour.Set(0.85470098f, 1.0f, 0.85470098f, 1.0f);
            if (DAT_10c655e0 != 0 && (DAT_10c62ed0 == 0x1f5 || DAT_10c62ed0 == 0x1f6))
                colour.Set(1.0f, 0.75213700f, 1.0f, 1.0f);
            int model = FUN_1003a880(path, position.x, position.y, position.z, 0, 0, 0,
                                     (heading - 1.57075f) * 180.0f / 3.1415f, colour);
            if (model != -1)
            {
                int last = DAT_10edbe08[model].end - 1;
                if (last < 2)
                    last = 1;
                DAT_10467718[number] = (short)last;
            }
            Vec4 effectPosition = DAT_1020af20[number]->matrix.GetTranslation();
            effectPosition.z = effectPosition.z + 10.0f;
            DAT_10daaa80.Start(DAT_10253e58[number] == 1 ? 0x2d4 : 0x2e4, effectPosition, -1, 0, 0, 0, 0, 0, 0, 0);
            char name[0x40];
            sprintf(name, "@ksk%02d", number);
            for (int i = 0; i < 12; i++)
            {
                Node *node = (Node *)DAT_10255f08->M_1019ed10(name, i);
                if (node == 0)
                    break;
                node->SetFlags(0x20);
                node->ClearFlags(4);
            }
            sprintf(name, "@ksk%02d+", number);
            for (int i = 0; i < 6; i++)
            {
                Node *node = (Node *)DAT_10255f08->M_1019ed10(name, i);
                if (node == 0)
                    break;
                if (node->name[strlen(node->name) - 1] == '0')
                    node->M_10191d90();
            }
            if (DAT_104678a0[number] != 0)
                DAT_10874848.SetScriptFlag(DAT_104678a0[number], 1);
            Vec4 top = DAT_1020af20[number]->worldMatrix.GetTranslation();
            int pieces = DAT_1042da88[number];
            while (pieces != 0)
            {
                if (pieces >= 500)
                {
                    KSK_PIECE(top, 0x56);
                    pieces -= 500;
                }
                else if (pieces >= 100)
                {
                    KSK_PIECE(top, 0x55);
                    pieces -= 100;
                }
                else if (pieces >= 0x32)
                {
                    KSK_PIECE(top, 0x54);
                    pieces -= 0x32;
                }
                else if (pieces >= 10)
                {
                    KSK_PIECE(top, 0x53);
                    pieces -= 10;
                }
                else if (pieces >= 5)
                {
                    KSK_PIECE(top, 0x52);
                    pieces -= 5;
                }
                else if (pieces >= 1)
                {
                    KSK_PIECE(top, 0x51);
                    pieces--;
                }
            }
            DAT_10874848.RaiseKskonEvents(number);
            DAT_1020af20[number] = 0;
            DAT_1021fa40[number] = 0;
        }
        break;
    case GAME_BUILTIN_SET_FRAME:
        DAT_104c02e6 = (char)*args[0].i;
        break;
    case GAME_BUILTIN_WALLPAPER:
        {
            char itmPath[0x104];
            char bmpPath[0x124];
            char wallPath[0x104];
            char baseName[0x100];
            char wallpaper[0x104];
            sprintf(itmPath, "%s.itm", FUN_1013f8a0(args[0].text, 0));
            sprintf(bmpPath, "%s%s.bmp", DAT_10f60590, args[0].text);
            sprintf(wallPath, "%swall.txt", DAT_10f60590);
            FILE *file = fopen(itmPath, "rb");
            if (file != 0)
            {
                fseek(file, 0, SEEK_END);
                long fileSize = ftell(file);
                fseek(file, 0, SEEK_SET);
                int dataSize = fileSize - 0xc;
                FUN_10161120(baseName, itmPath);
                unsigned char *data = (unsigned char *)operator new(dataSize);
                fread(data, 1, dataSize, file);
                int header;
                int key;
                unsigned int checksum;
                fread(&header, 1, 4, file);
                fread(&key, 1, 4, file);
                fread(&checksum, 1, 4, file);
                fclose(file);
                FILE *bitmap = fopen(bmpPath, "wb");
                if (bitmap != 0)
                {
                    FUN_101615d0(data, dataSize, baseName, key, checksum);
                    fwrite(data, 1, dataSize, bitmap);
                    fclose(bitmap);
                    SystemParametersInfoA(SPI_GETDESKWALLPAPER, 0x100, wallpaper, 1);
                    FILE *wall = fopen(wallPath, "at");
                    if (wall != 0)
                    {
                        fprintf(wall, "%s\n", wallpaper);
                        fclose(wall);
                        SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, bmpPath, SPIF_UPDATEINIFILE);
                        SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, bmpPath, SPIF_SENDCHANGE);
                    }
                }
                free(data);
            }
        }
        break;
    case GAME_BUILTIN_MOUSE:
        DAT_10202800 = (char)*args[0].i;
        break;
    case GAME_BUILTIN_TYPE:
        {
            char typed[GAME_BUILTIN_TYPE_BUFFER_SIZE];
            char text[0x124];
            int frames = *args[0].i;
            memset(typed, 0, sizeof(typed));
            DAT_104c030c = frames + DAT_104bf890;
            DAT_10f68110 = 1;
            FUN_1015f310(typed);
            DAT_10f66dd4 = 1;
            FUN_1015d500(GAME_BUILTIN_TYPE_FRAMES);
            DAT_104c030a = 1;
            DAT_104c0310 = 0;
            while (DAT_104c030c > DAT_104bf890 && FUN_10137b10() != 0)
                ;
            DAT_104c030a = 0;
            FUN_1015fd30();
            if (typed[0] == 0)
                strcpy(typed, "\x96\xa2\x93\xfc\x97\xcd");
            sprintf(text, "\"%s\"", typed);
            SetTextResult(bank, text);
        }
        break;

    default:
        break;
    }
}
