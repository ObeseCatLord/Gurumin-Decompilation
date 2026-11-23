// Enemy home-range test (enemy_moves.cpp), called with the step as a Vec4 by the enemy moves.
#ifndef GURUMIN_ENEMY_HOME_H
#define GURUMIN_ENEMY_HOME_H

#include "vec4.h"

// 0x10024d00 (Enemy_CanReturnHome): 1 when enemy `index` may head for its start position, given the distance
// `distance` after the step and `homeDistance` now; `step` is the step (x/y are the direction), `minimum` the
// distance below which it always may.
extern "C" int FUN_10024d00(int index, float distance, float homeDistance, Vec4 step, float minimum);

#endif
