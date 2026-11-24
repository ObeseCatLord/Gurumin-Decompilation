// Enemy moves that keep a distance to the player (EN build). Declared once here.
#ifndef GURUMIN_EN_ENEMY_FLEE_H
#define GURUMIN_EN_ENEMY_FLEE_H

#include "vec4.h"

// Non-zero = enemies do not move. EN 0x00a21304 (JP DAT_104bfd08).
extern "C" int g_enemiesFrozen;

// Non-zero = the step `move` of enemy `index` leaves its range. EN 0x5c1770 (JP Char_OutsideLimit).
extern "C" int Char_OutsideLimit(int index, Vec4 move);

// Backs enemy `index` away from the player. EN 0x5beb30 (JP Enemy_BackAwayFromPlayer).
extern "C" int Enemy_BackAwayFromPlayer(int index, int stopAfter, int shootBelow);

#endif
