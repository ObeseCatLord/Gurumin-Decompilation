// The script system command dispatcher (script_system_command.cpp) and the globals it alone uses. The commands
// read their argument and leave results in the shot registers DAT_10c6356c.. (camera_vars.h, scene_command.h).
#ifndef GURUMIN_SCRIPT_SYSTEM_H
#define GURUMIN_SCRIPT_SYSTEM_H

#include "vec4.h"
#include "matrix4.h"

// Command ids of Script_SystemCommand.
enum SystemCommand
{
    SYSCMD_COUNT_FLAG_BITS = 0x9c40,        // counts the set bits of a block of script flag words
    SYSCMD_BEGIN_TIMED_RUN = 0x9c41,        // clears flags 1000-1199, starts motion 0 and the run timer
    SYSCMD_RECORD_ROOM_BITS = 0x9c42,       // stores which characters are in which state as flag bit masks
    SYSCMD_SET_ENTRY_FLAGS = 0x9c43,        // Map_SetEntryFlags for the given map
    SYSCMD_SPAWN_SHAGA = 0x9c44,            // starts a shaga effect (0 = a random one) at the shot point
    SYSCMD_GET_PLAYER_STATE = 0x9c45,       // result = DAT_104c00d0
    SYSCMD_TUTORIAL_OPEN_FIRST = 0xc350,
    SYSCMD_TUTORIAL_CLOSE = 0xc351,
    SYSCMD_TUTORIAL_OPEN_SECOND = 0xc352,
    SYSCMD_UNUSED_C353 = 0xc353,
    SYSCMD_UNUSED_C355 = 0xc355,
    SYSCMD_SET_BS003_VARIANT = 0xea61,
    SYSCMD_VOLUME_FADE = 0xea62,            // argument 0: one fade, otherwise the other
    SYSCMD_TOGGLE_LEVERS = 0xea63,          // switches every lever (type 0xb) whose id is the argument
    SYSCMD_GET_KSK_POSITION = 0xea64,       // result = position of the "@kskNN_" node
    SYSCMD_SET_SAVED_VALUE = 0x11173,
    SYSCMD_GET_SAVED_VALUE = 0x11174,
    SYSCMD_SAVE_OR_LOAD_FLAGS = 0x11175,    // argument 0: copy the script flags aside, otherwise restore them
    SYSCMD_NOP_1868B = 0x1868b,
    SYSCMD_PLACE_OBSERVER = 0x1868c,
    SYSCMD_MARK_PLAYER_START = 0x1868d,
    SYSCMD_RESET_GAME_FLAGS = 0x1868e,
    SYSCMD_FLOOD_LIGHTS = 0x1868f,
    SYSCMD_SET_CAMERA_FRAME = 0x18690,
    SYSCMD_RELOAD_PLAYER_A = 0x18691,
    SYSCMD_WAIT_FRAME_STEPS = 0x18692,
    SYSCMD_DRAIN_LIGHTS = 0x18693,
    SYSCMD_SET_LIGHT_POINT = 0x18694,
    SYSCMD_BUILD_CLEAR_MARKERS = 0x18695,
    SYSCMD_SHOW_FIELD_MAP = 0x18696,
    SYSCMD_CAMERA_PATH_POINT = 0x18697,
    SYSCMD_UNLOCK_COSTUME = 0x18698,
    SYSCMD_GET_COSTUME_BITS = 0x18699,
    SYSCMD_UNLOCK_CURRENT_COSTUME = 0x1869a,
    SYSCMD_GET_MAP_RECORD_POINT = 0x1869b,
    SYSCMD_SET_CAMERA_FRAME_OF_RECORD = 0x1869c,
    SYSCMD_GET_TARGET_MAP = 0x1869d,
    SYSCMD_ENTER_FIELD_MODE = 0x1869e,
    SYSCMD_RELOAD_PLAYER_B = 0x1869f,
    SYSCMD_RELOAD_PLAYER_MODEL = 0x186a0
};

// Shaga kinds spawned by SYSCMD_SPAWN_SHAGA.
#define SHAGA_KIND_MAX_RANDOM 0x1d      // a random argument is 1 + rand % 0x1d
#define SHAGA_KIND_FIRST_FAULTED 0x11   // random arguments above this get 0x20 added
#define SHAGA_KIND_PAIRED_FIRST 0x32    // kinds from here on record themselves in DAT_1042db08
#define SHAGA_PARTNER_KIND_OFFSET 1000

// Script flag blocks of SYSCMD_RECORD_ROOM_BITS.
#define ROOM_BITS_FLAG_BASE 0x40c       // + DAT_10c63ffc
#define ROOM_BITS_PAIR_FLAG_BASE 0x424  // + 2 * DAT_10c63ffc (two words)
#define ROOM_BITS_FIRST_SLOT 0x5c       // the 32 character slots 0x5c-0x7b map to the bits
#define ROOM_BITS_SLOT_COUNT 0x20
#define ROOM_BITS_BITS_PER_WORD 0x1f
#define FLAG_BITS_PER_WORD 0x1f

#ifndef SYSTEM_RECORD_COSTUMES
#define SYSTEM_RECORD_COSTUMES 0x80   // record of system.sav that holds the costume unlock bits
#endif

// SYSCMD_SET_LIGHT_POINT and SYSCMD_FLOOD_LIGHTS: light points DAT_10edbce8[0..5].
#define SCRIPT_LIGHT_POINT_MAX 5

extern "C" int DAT_10c63378;               // 1 while the camera path markers can be rebuilt
extern "C" int DAT_10c63584;               // costume bits as read by SYSCMD_GET_COSTUME_BITS (& 0xf7)
extern "C" int DAT_10c63588;               // & 0x40
extern "C" int DAT_10c63594;               // & 0x100
extern "C" int DAT_10c63598;               // & 0x200
extern "C" int DAT_10c63e9c;
extern "C" int DAT_10c63ff4;               // map when the timed run began
extern "C" int DAT_10c63ffc;               // slot of the timed run in the flag blocks
extern "C" unsigned long DAT_10c64004;     // tick count when the timed run began
extern "C" int DAT_10c64028;               // base of the timed run's start flag (+ 500)
extern "C" int DAT_10c64f1c[3];            // set once the matching action flag below was seen
extern "C" int DAT_10c64f2c[6];            // set once the matching action input was used
extern "C" char DAT_103150f8;
extern "C" char DAT_103150f9;
extern "C" char DAT_103150fd;
extern "C" int DAT_104c03f0;               // value kept by SYSCMD_SET_SAVED_VALUE
extern "C" int DAT_1021fb48[4000];         // the script flags put aside by SYSCMD_SAVE_OR_LOAD_FLAGS
extern "C" Vec4 DAT_108029b8;              // the player's start point (w = heading)
extern "C" Matrix4 DAT_1086df38;           // the observer's matrix
extern "C" Matrix4 DAT_10db9210;           // the player's world matrix
extern "C" int DAT_10daaaec;
extern "C" int DAT_10daaaf0;
extern "C" int DAT_10daabb4;               // the helmet worn
extern "C" int DAT_10db6f68;
extern "C" char DAT_10db86d0;
extern "C" int DAT_10db9380;
extern "C" Vec4 DAT_10edbce8[SCRIPT_LIGHT_POINT_MAX + 1];   // the light points started by the lights commands

extern "C" void FUN_1000fd50(void);         // the tracker's window callback
extern "C" void FUN_1013e6e0(void);         // the tracker's user value

// 0x100fedd0 (script_system_command.cpp; C++ linkage). Runs system command `command` with `argument`. Returns 0.
int Script_SystemCommand(int command, int argument);

#endif
