// EffectModel (EN build): one slot of the table of models placed by scripts (hit sparks, magic effects) and attached
// models such as held items. A slot is free while node == 0.
// This is the one declaration of EffectModel in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_EFFECT_MODEL_H
#define GURUMIN_EN_EFFECT_MODEL_H

#include "key_bank.h"
#include "node.h"
#include "vec4.h"

// Bytes of an EffectModel.
const int EFFECT_MODEL_SIZE = 0xb08;

struct EffectModel
{
    int m_0000;         // 0x00: not recovered
    Node *node;         // 0x04: the loaded model (0 = slot free)
    int start;          // 0x08: first frame played
    int end;            // 0x0c: last frame played
    Vec4 color;         // 0x10: colour
    Vec4 rotation;      // 0x20: rotation about x, y and z in degrees
    int visible;        // 0x30: 1 = drawn
    int frame;          // 0x34: frame counter
    int value;          // 0x38: the `value` argument of the setup
    int ownCamera;      // 0x3c: non-zero = drawn with its own fixed camera
    Matrix4 matrix;     // 0x40: placement
    KeyBank keys;       // 0x80: key frames
    int changed;        // 0xb04: 1 after a key or the rotation was changed

    EffectModel();      // EN 0x5a0fa0 (JP 0x1000be40): members construct themselves
    int Free();       // EN 0x63f320 (JP 0x1000be10): releases the model, the slot is free again
};

#endif
