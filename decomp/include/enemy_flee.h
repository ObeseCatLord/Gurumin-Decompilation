// Enemy moves that keep away from the player (enemy_flee.cpp), used by the AI updaters in enemy_ai.cpp.
#ifndef GURUMIN_ENEMY_FLEE_H
#define GURUMIN_ENEMY_FLEE_H

#include "vec4.h"

// 0x10024e80 (Char_OutsideLimit): non-zero = the step `move` of enemy `index` leaves its range.
extern "C" int FUN_10024e80(int index, Vec4 move);

// 0x100279b0 (Enemy_BackAwayFromPlayer): the enemy walks away from the player. `stopAfter` is the lowest
// 1-in-400 random roll that keeps it going when blocked; `shootBelow` is the 1-in-400 roll below which it throws a
// weapon (when its mode is 1). Returns 1 when the move ends, else 0.
extern "C" int FUN_100279b0(int index, int stopAfter, int shootBelow);

#endif
