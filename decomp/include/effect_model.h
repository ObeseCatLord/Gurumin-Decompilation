// EffectModel: declared here once (first used by src/effect_model.cpp).
#ifndef GURUMIN_EFFECT_MODEL_H
#define GURUMIN_EFFECT_MODEL_H

#include "key_track.h"
#include "node.h"

struct EffectModel
{
    int pad_0000;
    Node *node;
    int start;
    int end;
    Vec4 color;
    Vec4 offset;
    int m_0030;
    int reverseStart;
    int m_0038;
    int m_003c;
    Matrix4 matrix;     // 0x40: placement, copied from the world matrix DAT_10db9210 at setup
    KeyBank keys;       // 0x80
    int changed;        // 0xb04: 1 after a key or the offset was changed, reset to 0 at setup
    EffectModel();
    int M_1000b9f0(const char *path, int value, int start, int end);

    // Adds a position key and marks the effect as changed.
    void AddKey(int frame, float x, float y, float z)
    {
        changed = 1;
        keys.Insert(frame, x, y, z, 0);
    }

    // Sets the offset and marks the effect as changed.
    void SetOffset(float x, float y, float z)
    {
        changed = 1;
        offset.Set(x, y, z, 0);
    }
};

#endif
