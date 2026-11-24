// The hopping spitter of kind 4 (enemy_hop_spit.cpp, run from 0x100f9670): a flier-like enemy that turns about,
// hops at the player and spits a fan of effects at him.
#ifndef GURUMIN_HOP_SPIT_ENEMY_H
#define GURUMIN_HOP_SPIT_ENEMY_H

#include "vec4.h"
#include "matrix4.h"

// 0x10065bf0 (Enemy_WanderHopAndSpit; C++ linkage: the original has an EH frame)
int Enemy_WanderHopAndSpit(int index);

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_10015860(Vec4 position);                                   // Vis_IsVisible
extern "C" int FUN_100242c0(int index, Vec4 *ground, float depth, int fromHead, int testOnly, const Vec4 *offset);   // Char_ProbeAhead
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#endif
