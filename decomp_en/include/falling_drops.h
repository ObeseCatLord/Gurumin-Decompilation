// Falling drops (EN build; JP: DROP_COUNT in decomp/include, DAT_10254c40, DAT_104c0438). This is the one declaration of
// the drop tables in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_FALLING_DROPS_H
#define GURUMIN_EN_FALLING_DROPS_H

#include "vec4.h"

// Entries of the drop tables.
const int DROP_COUNT = 0x400;

extern "C" unsigned char g_dropState[DROP_COUNT];   // EN 0x00a13520 (JP DAT_10254c40): 0 = unused, 1 = falling, 2..30 = fading out
extern "C" Vec4 g_dropPositions[DROP_COUNT];        // EN 0x00bca1a8 (JP DAT_104c0438): position of each drop

extern "C" unsigned int g_dropNext;                          // EN 0x009e351c: table index the last Drop_Spawn search stopped at (the next search starts after it)
// Number of free-slot probes Drop_Spawn makes before it gives up.
const int DROP_SPAWN_TRIES = 100;
// State of a drop that has just been spawned (falling).
const unsigned char DROP_FALLING = 1;
// Starts a drop at `position` in the next free slot after g_dropNext (at most DROP_SPAWN_TRIES probes). Returns 1, or 0
// when no slot was free. EN 0x5a27e0 (see falling_drops.cpp).
extern "C" int Drop_Spawn(const Vec4 &position);

// Moves, draws and advances every drop. Returns 0. EN 0x5b4760 (JP 0x1000c390).
extern "C" int Drops_Update(void);

#endif
