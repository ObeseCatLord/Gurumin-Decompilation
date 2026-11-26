// A placed model of the EN build (JP: ModelInstance in decomp/include/anim_channels.h). This is the one declaration of
// ModelInstance in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_MODEL_INSTANCE_H
#define GURUMIN_EN_MODEL_INSTANCE_H

#include "effect_options.h"
#include "vec4.h"

// Bounding vectors at the start of the instance.
const int MODEL_INSTANCE_BOUND_PAIR = 2;

// A placed model: bounding vectors, three matrices (world, local, previous) and its effect options.
struct ModelInstance
{
    char m_0000[0x24];                          // 0x00: not used by EN code yet
    Vec4 m_0024[MODEL_INSTANCE_BOUND_PAIR];     // 0x24: first pair of bounding vectors
    Vec4 m_0044;                                // 0x44: bounding vector
    Vec4 m_0054;                                // 0x54: bounding vector
    Vec4 m_0064;                                // 0x64: bounding vector
    Vec4 m_0074;                                // 0x74: bounding vector
    Vec4 m_0084;                                // 0x84: bounding vector
    Vec4 m_0094;                                // 0x94: bounding vector
    Vec4 m_00a4;                                // 0xa4: bounding vector
    Vec4 m_00b4;                                // 0xb4: bounding vector
    Matrix4 m_00c4;                             // 0xc4: world matrix
    Matrix4 m_0104;                             // 0x104: local matrix
    Matrix4 m_0144;                             // 0x144: previous matrix
    char m_0184[0x10];                          // 0x184: not used by EN code yet
    EffectOptions state;                        // 0x194: effect options of the model

    ModelInstance();                            // EN 0x709d80 (JP 0x101728c0)
    ~ModelInstance();                           // EN 0x70a240 (EN-only, anim_channels.cpp): destroys the three matrices, last to first
};

#endif
