// The ground enemy that spits a fan of effects and charges (enemy_spit_charge.cpp, a per-kind behaviour run from
// 0x100f9670).
#ifndef GURUMIN_SPIT_CHARGE_ENEMY_H
#define GURUMIN_SPIT_CHARGE_ENEMY_H

#include "vec4.h"

// 0x1006be40 (Enemy_SpitAndCharge; C++ linkage: the original has an EH frame)
int Enemy_SpitAndCharge(int index);

// 0x10050df0 (Char_ProbeSideLedge, ledge_probe.cpp): looks for the ground below character `index` and for a wall
// and ledge beside it.
extern "C" int FUN_10050df0(int index, Vec4 *wall, Vec4 *ground, float *wallDistance, float *height, float *drop,
                            float reach);

#endif
