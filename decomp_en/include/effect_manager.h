// The effect manager (EN build): starts, steps and draws the running particle and sprite effects. Only the methods
// used so far are declared; the data layout is not recovered, so sizeof(EffectManager) is not the real size.
// This is the one declaration of EffectManager in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_EFFECT_MANAGER_H
#define GURUMIN_EN_EFFECT_MANAGER_H

#include "vec4.h"

struct EffectOptions;   // start options of an effect

struct EffectManager
{
    // Starts the effect with script id `effect` at `position`. `owner` is the matrix it follows (0 = none), `matrix` the
    // matrix it is placed with, `options` the start options (0 = defaults) and `pause` a flag that pauses it while
    // non-zero. Returns the effect handle. EN 0x70a7f0 (JP 0x10172350).
    int Start(int effect, Vec4 position, int parent, int unused, const Matrix4 *owner, int value, int frame,
              const Matrix4 *matrix, const EffectOptions *options, int *pause);
};

extern "C" EffectManager g_effectManager;   // EN 0x01487f98 (JP DAT_10daaa80)

#endif
