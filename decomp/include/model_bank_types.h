// ModelData, ShadowScene: declared here once (first used by src/model_bank.cpp).
#ifndef GURUMIN_MODEL_BANK_TYPES_H
#define GURUMIN_MODEL_BANK_TYPES_H

#include "model_bank.h"

// The model data a record's id2 points to, only the field used here.
struct ModelData
{
    char pad_0000[0x6e0];
    int m_06e0;
};
// The object the shadows are drawn by (its type is not known yet).
struct ShadowScene
{
    char pad_0000[0x24c];
    Matrix4 matrix;         // 0x24c
    int DrawShadow(int a, int b, int c, const Vec4 *p, const Vec4 *q, int texture, float scale, int size, int data,
                   AnimModel *model, float modelScale);  // 0x101a29e0
};

#endif
