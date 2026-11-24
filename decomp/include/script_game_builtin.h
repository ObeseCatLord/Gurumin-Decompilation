// The game's script built-ins (script_game_builtin.cpp): the handler registered for every built-in of the game
// (Message, Wait, LoadMap, CharPos, PlayMovie, Custom, Ksk, Kabe ...), the ids they are registered with and the
// globals only they use. This is the one declaration of these: include this header, never redeclare them.
#ifndef GURUMIN_SCRIPT_GAME_BUILTIN_H
#define GURUMIN_SCRIPT_GAME_BUILTIN_H

#include <string.h>

#include "listbank.h"
#include "vec4.h"

// Ids of the game built-ins (the id is the second argument of ListBank::RegisterBuiltin, listbank.cpp).
enum GameBuiltin
{
    GAME_BUILTIN_GET_COSTUME_UNLOCKED = 0x2fbe,    // int GetCf(int bit)
    GAME_BUILTIN_SHOP_START = 0x2b5c,              // int ShopStart()
    GAME_BUILTIN_SHOP_ITEM = 0x2b5d,               // int ShopItem(int item): adds the item to the list window
    GAME_BUILTIN_SHOP_END = 0x2b5e,                // int ShopEnd(): runs the shop window
    GAME_BUILTIN_SEL_START = 0x2b5f,               // int SelStart(int)
    GAME_BUILTIN_SEL_TEXT = 0x2b60,                // int SelText(string, int)
    GAME_BUILTIN_SEL_END = 0x2b61,                 // int SelEnd(): runs the selection window
    GAME_BUILTIN_SHOP_END2 = 0x2b62,               // int ShopEnd2(): runs the window and returns the choice
    GAME_BUILTIN_MESSAGE = 11000,                  // int Message(int, string x4)
    GAME_BUILTIN_RECOVER = 12000,                  // int Recover(int, int, float, float, float)
    GAME_BUILTIN_WAIT = 0x2ee3,                    // int Wait(int frames)
    GAME_BUILTIN_WAIT_DISARMED = 0x2ee4,           // int WaitD(int frames)
    GAME_BUILTIN_CROSS_FADE_ALIAS = 0x319f,        // runs the same code as CrossFade
    GAME_BUILTIN_SE = 0x3200,                      // int SE(int sound)
    GAME_BUILTIN_LOAD_MAP = 0x3264,                // int LoadMap(int map)
    GAME_BUILTIN_CHAR_POS = 0x3265,                // int CharPos(int character, int x, int y, int z)
    GAME_BUILTIN_MAP_ENABLE = 0x3266,              // int MapEnable(int)
    GAME_BUILTIN_LOCK = 0x3267,                    // int Lock()
    GAME_BUILTIN_UNLOCK = 0x3268,                  // int Unlock()
    GAME_BUILTIN_WALK = 0x3269,                    // int Walk(int, int, int)
    GAME_BUILTIN_BLACK = 0x326a,                   // int Black(int, int, int)
    GAME_BUILTIN_LOOK = 0x326b,                    // int Look(int character, int target)
    GAME_BUILTIN_BGM = 0x326c,                     // int BGM(int track)
    GAME_BUILTIN_EVENT = 0x326d,                   // int Event(int id)
    GAME_BUILTIN_VISIBLE = 0x326e,                 // int Visible(int character, int visible)
    GAME_BUILTIN_MAP_EVENT = 0x326f,               // int MapEvent(int)
    GAME_BUILTIN_WINDOW_MESSAGE = 0x3270,          // int WMessage(string, string)
    GAME_BUILTIN_SCOP = 0x3271,                    // int Scop(int)
    GAME_BUILTIN_PLAY_MOVIE = 0x3272,              // int PlayMovie(string, int)
    GAME_BUILTIN_CROSS_FADE = 0x3273,              // int CrossFade()
    GAME_BUILTIN_RECOVER_ALL = 0x3274,             // int Kaifuku(): refills the player's health
    GAME_BUILTIN_CHAR_EFFECT = 0x3275,             // int RCharEffect(int effect, int character)
    GAME_BUILTIN_BLUR = 0x3276,                    // int Blur(int, int, int)
    GAME_BUILTIN_MMV = 0x3277,                     // int MMV(string)
    GAME_BUILTIN_EQUIP = 0x3278,                   // int Soubi(int item)
    GAME_BUILTIN_STATUS_UP = 0x3279,               // int StatusUp(int): does nothing
    GAME_BUILTIN_CAMERA = 0x327a,                  // int Camera(int, int, int, int)
    GAME_BUILTIN_SET_ROLL = 0x327b,                // int SetRoll(int, int): does nothing
    GAME_BUILTIN_GET_ITEM = 0x327c,                // int GetItem(int item)
    GAME_BUILTIN_SET_WALK_TARGET = 0x327d,         // set by scripts only (no name)
    GAME_BUILTIN_WALK2 = 0x3282,                   // int Walk2(int, int, int, int)
    GAME_BUILTIN_MES = 0x3283,                     // int Mes(string)
    GAME_BUILTIN_MENU = 0x3284,                    // int Menu(int): does nothing
    GAME_BUILTIN_SET_MAP_COLOR = 0x3285,           // int SetMapColor(int, float, float, float)
    GAME_BUILTIN_POST_SCR = 0x3286,                // int PostScr(int)
    GAME_BUILTIN_LOCK_COMMAND = 0x3287,            // int LockCommand(int)
    GAME_BUILTIN_SET_COMMAND_LOCK = 0x3288,        // sets the second command lock only
    GAME_BUILTIN_SET_CHARACTER_KIND = 0x3289,      // sets a character's slot kind (no name)
    GAME_BUILTIN_GET_MP = 0x328a,                  // int GetMP()
    GAME_BUILTIN_SAVE_MAP = 0x328b,                // int SaveMap()
    GAME_BUILTIN_RESTORE_MAP = 0x328c,             // int RestoreMap()
    GAME_BUILTIN_LEVEL_UP = 0x328d,                // int LVUP(int)
    GAME_BUILTIN_SET_SAVE_POS = 0x328e,            // int SetSavePos()
    GAME_BUILTIN_RESET_FADE = 0x3291,              // int ResetFade()
    GAME_BUILTIN_GET_GP = 0x3292,                  // int GetGP()
    GAME_BUILTIN_SET_GP = 0x3293,                  // int SetGP(int)
    GAME_BUILTIN_KAGU_TEST = 0x3296,               // int KaguTest(int, int)
    GAME_BUILTIN_KAGU_TEST2 = 0x3297,              // int KaguTest2(int, int, string)
    GAME_BUILTIN_WHITE = 0x3298,                   // int White(int, int, int)
    GAME_BUILTIN_ERASE_FLOOR = 0x3299,             // int YukaKesi(int x, int y)
    GAME_BUILTIN_REFFECT = 0x329a,                 // int REffect(int effect, int x, int y, int z)
    GAME_BUILTIN_SEND_EVENT = 0x329b,              // int SendEvent(int id)
    GAME_BUILTIN_LIFT_ENABLE = 0x329c,             // int LiftEnable(int)
    GAME_BUILTIN_SPECIAL = 0x329d,                 // int Special(int kind)
    GAME_BUILTIN_LIFT_2C = 0x329e,                 // int Lift2C(int id)
    GAME_BUILTIN_DOOR_2C = 0x329f,                 // int Door2C(int kind)
    GAME_BUILTIN_ENEMY_STOP = 0x32a0,              // int TekiStop(int)
    GAME_BUILTIN_GET_CHAR_POS = 0x32a1,            // int GetCharPos(int character, int &x, int &y, int &z)
    GAME_BUILTIN_LIFT_ENABLE2 = 0x32a2,            // int LiftEnable2(int character, int)
    GAME_BUILTIN_GET_BT15 = 0x32a3,                // int Getbt15(int character)
    GAME_BUILTIN_SCOP2 = 0x32a4,                   // int Scop2(int, int, string)
    GAME_BUILTIN_SAVE_CAMERA = 0x32a5,             // int SaveCamera()
    GAME_BUILTIN_RESTORE_CAMERA = 0x32a6,          // int RestoreCamera()
    GAME_BUILTIN_MISS = 0x32a7,                    // int Miss()
    GAME_BUILTIN_RESET = 0x32a8,                   // int Reset()
    GAME_BUILTIN_SET_SAVE_POS2 = 0x32a9,           // int SetSavePos2(int map, int x, int y, int z)
    GAME_BUILTIN_GAME_OVER = 0x32be,               // int GAMEOVER(int)
    GAME_BUILTIN_WAIT2 = 13000,                    // int Wait2(int frames)
    GAME_BUILTIN_TAKARA_2C = 0x32c9,               // int Takara2C(int id)
    GAME_BUILTIN_WAIT_COMMAND = 0x32ca,            // int WaitC(int frames)
    GAME_BUILTIN_DEMO = 0x332c,                    // int Demo(string)
    GAME_BUILTIN_LOAD_MAP_B = 0x332d,              // int LoadMapB(int map)
    GAME_BUILTIN_STAGE_NAME = 0x332e,              // string StageName(int)
    GAME_BUILTIN_STAGE_MMV = 0x332f,               // string StageMMV(int)
    GAME_BUILTIN_RELOAD_CHAR = 0x3330,             // int ReloadChar()
    GAME_BUILTIN_PLAY_MMV3 = 0x3336,               // int PlayMMV3(string, int)
    GAME_BUILTIN_PLAY_AVI = 0x3337,                // int PlayAVI(string, int)
    GAME_BUILTIN_FILL = 0x3338,                    // int Fill(int, float)
    GAME_BUILTIN_CUSTOM = 0x3339,                  // int Custom(): the equipment list
    GAME_BUILTIN_CUSTOM2 = 0x333a,                 // string Custom2(int)
    GAME_BUILTIN_CUSTOM3 = 0x333b,                 // int Custom3(int)
    GAME_BUILTIN_KSK = 0x333c,                     // int Ksk(int number)
    GAME_BUILTIN_SET_FRAME = 0x333d,               // int SetWaku(int)
    GAME_BUILTIN_WALLPAPER = 0x333e,               // int Kabe(string)
    GAME_BUILTIN_MOUSE = 0x333f,                   // int Mouse(int)
    GAME_BUILTIN_PLAY_MOVIE2 = 0x3340,             // int PlayMovie2(string, int)
    GAME_BUILTIN_TYPE = 0x3341                     // string Type(int)
};

// Result types written to ListBank::resultType.
#define GAME_BUILTIN_MAX_ITEMS 0x60            // items in the item table (12 blocks of 8)
#define GAME_BUILTIN_ITEMS_PER_BLOCK 8

// Frames Message windows use, tick of the text of the custom window and so on.
#define GAME_BUILTIN_WINDOW_FRAMES 0x5a
#define GAME_BUILTIN_MES_FRAMES 0x28
#define GAME_BUILTIN_TYPE_BUFFER_SIZE 0x100
#define GAME_BUILTIN_TYPE_FRAMES 0x10
// Level flags of the equipment items start at script flag 3205.
#define GAME_BUILTIN_LEVEL_FLAG_BASE 0xc85
// The number of the highest level of a helmet's abilities.
#define GAME_BUILTIN_MAX_LEVEL 4
// Half the size of the cell grid YukaKesi digs: 8 x 8 points 0x78 apart starting at -0x168.
#define GAME_BUILTIN_DIG_START (-0x168)
#define GAME_BUILTIN_DIG_STEP 0x78
#define GAME_BUILTIN_DIG_END 0x1e0
#define GAME_BUILTIN_DIG_TYPE 1000
// Effect ids of the equipment fireworks drawn above a Ksk node (1, 5, 10, 50, 100 and 500 pieces).
#define GAME_BUILTIN_KSK_PIECE_COUNT 6
// Camera shot values of the ending of the CrossFade transition.
#define GAME_BUILTIN_CROSS_FADE_MODE 0xf

extern "C" char DAT_102026d8[0x80];               // the file name Demo(string) stores
extern "C" char DAT_10874e48[0x40][0x40];         // the stage names, StageName(int) returns one
extern "C" Vec4 DAT_10837fc8[0x80];               // per character: where SaveMap saved it (w = its heading)
extern "C" char DAT_10c68974;                     // set when a movie is played while the costume 6 is worn
extern "C" Vec4 DAT_10db6f78;                    // filled with zero by WaitD
extern "C" int DAT_104c0340;                      // non-zero = PlayAVI plays nothing
extern "C" int DAT_104c02d0;                      // the item the equipment window works on
extern "C" unsigned int DAT_104c02d4;             // ability bits of that item still to be learned (Custom3)
extern "C" unsigned int DAT_104c02d8;             // ability bits that item had when it was picked

// 0x10138280 (Script_GameBuiltin), script_game_builtin.cpp (C++ linkage: the saved matrix has a destructor).
// Runs the game built-in `id` with its arguments `args`; leaves the result of the call in `bank`.
void Script_GameBuiltin(int id, ScriptArgument *args, ListBank *bank);

// Leaves the text `text` as the result of the built-in that runs.
__forceinline void SetTextResult(ListBank *bank, const char *text)
{
    char *destination = bank->resultText;
    bank->resultType = SCRIPT_TYPE_TEXT;
    strcpy(destination, text);
}

#endif
