// The in-game frame (game_frame.cpp, 0x1011d810, game mode 1): globals and routines only it uses, and its declaration.
#ifndef GURUMIN_GAME_FRAME_H
#define GURUMIN_GAME_FRAME_H

#include "vec4.h"

// 0x1011d810 (the in-game frame, called by the frame step in game mode 1). Runs one pass of the game logic for each
// engine tick that has elapsed since the last frame (the first frame also resets the mode's globals), then prepares
// and draws the scene. Returns 0 when the pass was cut short (the main menu opened or a queued event is due while
// the debug flag is set) and 1 when the frame was completed.
extern "C" int FUN_1011d810(void);

// 0x10057ee0 (drill_level.cpp): keeps the drill gauge in range and moves the drill level to match it. Returns 0.
int UpdateDrillLevel(void);

extern "C" unsigned char DAT_104c0233;           // counts down once per tick, next to DAT_104c0232
extern "C" Vec4 DAT_10868618[0x20];              // per fish: the direction it faces (unit vector)
extern "C" Vec4 DAT_10daa880[0x20];              // per fish: the point it swims to
extern "C" int FUN_1000c5c0(const Vec4 &position);   // 0x1000c5c0 (slot_alloc.cpp): stores a falling drop's position in a free slot
extern "C" int FUN_1000c390(void);               // 0x1000c390 (falling_drops.cpp, Drops_Update): moves and draws the falling drops
extern "C" Vec4 DAT_10864ce8;                    // world position of the sun: the lens flare point the frame adds
extern "C" int FUN_1000e870(void);               // 0x1000e870 (anti_tamper.cpp, AntiTamper_Check)
extern "C" void FUN_10189080(void);              // 0x10189080: runs once per tick in the mode where DAT_104bfe6c is 1
extern "C" void FUN_1013fa10(int index, int frames, float z, float y, float x, float w);   // 0x1013fa10 (colour_fades.cpp)
extern "C" int __cdecl FUN_1000c2f0(const void *first, const void *second);   // 0x1000c2f0: compares two model indices (qsort order of the model update)

#endif
