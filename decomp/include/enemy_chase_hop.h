// The enemy that chases the player with obstacle steering, ledge probes and hops (enemy_chase_hop.cpp, step 2 of
// the enemy update 0x100e0320).
#ifndef GURUMIN_ENEMY_CHASE_HOP_H
#define GURUMIN_ENEMY_CHASE_HOP_H

#include "vec4.h"

// 0x100bcce0 (Enemy_ChaseAndHop): one frame of the chase of enemy `index` (`model` is the character whose idle flag
// and collision filter it also reads) towards `target`, from `self`. Returns 0.
extern "C" int FUN_100bcce0(int index, int model, Vec4 *target, Vec4 *self);

// 0x10078060: steering away from other enemies. Sets `found` when it has to avoid one and writes the step to take
// into `steer`; returns the motion to play (negative = none).
extern "C" int FUN_10078060(int index, Vec4 *position, int *found, Vec4 *steer);

// 0x100244b0 (Char_ProbeDirection), 0x10024750 (Char_ProbeGround), 0x100248e0 (Collision_ProbeDown), 0x100241e0
// (Character_SetMove): see collide_query.cpp and char_move.cpp.
extern "C" int FUN_100244b0(int index, float angle, Vec4 *ground, float depth, int fromHead, int testOnly,
                            const Vec4 *offset, const char *name);
extern "C" int FUN_10024750(int index, Vec4 *ground, float depth, int fromHead, const Vec4 *offset, int testOnly,
                            int smallObjects);
extern "C" int FUN_100248e0(Vec4 position, Vec4 *ground, float height);
extern "C" int FUN_100241e0(int index, float x, float y, float z);

#endif
