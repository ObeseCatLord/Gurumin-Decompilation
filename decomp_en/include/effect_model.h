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
    ~EffectModel();     // EN 0x5a1140: members destroy themselves (the matrix has the only real destructor)
    int Free();     // EN 0x63f320 (JP 0x1000be10): releases the model, the slot is free again
    void AddKey(int frame, float x, float y, float z);        // inline: adds a position key
    void SetRotation(float x, float y, float z);              // inline: sets the rotation in degrees
    int Rotate(float x, float y, float z);                    // EN 0x63f740 (EN-only, effect_model_rotate.cpp): the out-of-line twin of SetRotation that returns 0 (reached through a pointer, no direct caller)
    int AddPositionKey(int frame, float x, float y, float z);  // EN 0x5a2670 (EN-only, effect_model_rotate.cpp): the out-of-line twin of AddKey that returns 0 (no direct caller)
    int DrawKeyed(Matrix4 &view, Matrix4 &camera);            // EN 0x5a8790 (JP 0x10037b40): plays one frame with the key frames applied
    int Setup(const char *path, int a, int start, int end);   // EN 0x602e10 (JP 0x1000b9f0): loads model `path` into the slot
};

// Adds a position key at `frame` and marks the keys changed.
inline void EffectModel::AddKey(int frame, float x, float y, float z)
{
    changed = 1;
    keys.Insert(frame, x, y, z, 0);
}

// Sets the rotation (degrees about x, y, z) and marks the model changed.
inline void EffectModel::SetRotation(float x, float y, float z)
{
    changed = 1;
    rotation.Set(x, y, z, 0);
}

// Slots of the effect model table.
const int EFFECT_MODEL_COUNT = 0x40;
// The effect model table. EN 0x015bd790 (JP DAT_10edbe08).
extern "C" EffectModel g_effectModels[EFFECT_MODEL_COUNT];

// Places effect model `path` at (x, y, z) in a free slot, rotated by (ox, oy, oz). Returns the slot or -1. EN 0x5a1d30.
extern "C" int EffectModel_Place(const char *path, float x, float y, float z, int start, float ox, float oy, float oz);
// Same with a colour. EN 0x5a1e10.
extern "C" int EffectModel_PlaceColoured(const char *path, float x, float y, float z, int start, float ox, float oy,
                                         float oz, const Vec4 &colour);

// Queues every visible effect model slot for drawing: through the camera view, or through its own fixed camera. Returns 0.
// EN 0x5cbd30 (JP EffectModel::Draw 0x1000baf0, called for every slot).
int MeshSlots_DrawAll(void);

#endif
