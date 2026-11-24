// The wisp caster, an enemy of kind 0x10 (enemy_wisp_caster.cpp, a per-kind behaviour run from 0x100f9670): it
// hides until it is hit, then stalks, dashes and lunges at the player, and casts flame effects and two drifting wisps.
#ifndef GURUMIN_WISP_CASTER_ENEMY_H
#define GURUMIN_WISP_CASTER_ENEMY_H

#include "vec4.h"

// 0x10071ee0 (Enemy_WispCaster; C++ linkage: the original has an EH frame)
int Enemy_WispCaster(int index);

// 0x1001bec0 (actor_scan.cpp): slows `velocity` down when it would take `position` closer to another object.
extern "C" int FUN_1001bec0(const Vec4 *position, Vec4 *velocity);

// 0x1003a7d0 (EffectModel_Place): places the effect model `path` at (x, y, z).
extern "C" int FUN_1003a7d0(const char *path, float x, float y, float z, int start, float ox, float oy, float oz);

#endif
