// The fire-breathing stalker (EN build; JP: decomp/include/breath_enemy.h). Declared once here. Used by src/enemy_breather.cpp.
#ifndef GURUMIN_EN_BREATH_ENEMY_H
#define GURUMIN_EN_BREATH_ENEMY_H

#include "char.h"

// Per-slot flag: non-zero = the "@efc_Null" node must be found again (the model was reloaded); cleared once done.
// EN 0x0091e6c0 (JP DAT_10468730).
extern "C" char g_charRebindNode[CHAR_SLOT_COUNT];

// One frame of the fire-breathing stalker `index`: stalks, circles, retreats and breathes. Returns 0. C++ linkage (the
// original has an exception frame). EN 0x6a4e20 (JP 0x100ac440).
int Enemy_StalkAndBreathe(int index);

#endif
