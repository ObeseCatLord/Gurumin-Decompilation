// Effect model: an animated model (node tree + key frames) played at a position, e.g. hit sparks or magic effects.

#include "vec4.h"

#include "matrix4.h"

#include "key_track.h"

#include "node.h"

#include "effect_model.h"

extern "C" EffectModel DAT_10edbe08[64];  // effect models placed by scripts (slot free while node == 0)

extern "C" Matrix4 DAT_10db9210;
extern "C" void *(*DAT_11b0b938)(int size, const char *tag);
extern "C" void (*DAT_11b1ba40)(void *);
extern "C" const char *FUN_1013f8a0(const char *format, ...);

// Constructor: only the members with constructors do anything (Vec4/Matrix4/KeyBank); the rest is set by Setup.
EffectModel::EffectModel()
{
}

// Setup: loads the effect model `path` (folder 0 of FUN_1013f8a0) and plays frames start..end.
// A negative start counts back from the end (stored in reverseStart, start becomes 0); a negative end means
// "up to the model's last frame" (Node::M_10199340 returns the frame count). Releases any previous model first.
int EffectModel::M_1000b9f0(const char *path, int value, int start, int end)
{
    color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    this->end = end;
    m_0030 = 0;
    this->start = start;
    reverseStart = 0;
    matrix = DAT_10db9210;
    offset.Set(0.0f, 0.0f, 0.0f, 0.0f);
    m_003c = 0;
    if (start < 0)
    {
        reverseStart = -start;
        this->start = 0;
    }
    m_0038 = value;
    if (node != 0)
    {
        node->Release();
        DAT_11b1ba40(node);
    }
    keys.Clear();
    node = (Node *)DAT_11b0b938(0x18a0, "m_frm");
    node->Init();
    node->Load(FUN_1013f8a0(path, 0), 0, 0, 0);
    if (end < 0)
        this->end = node->M_10199340();
    node->Rebuild();
    node->BuildMesh(1, 0);
    changed = 0;
    return 1;
}

// Returns the first free effect model slot, or -1.
inline int FindFreeEffectModel()
{
    for (int i = 0; i < 64; i++)
    {
        if (DAT_10edbe08[i].node == 0)
            return i;
    }
    return -1;
}

// Places effect model `path` at (x, y, z) in a free slot, starting at frame `start` and playing to the end, moved
// by (ox, oy, oz). Returns the slot, or -1 when all 64 are in use.
extern "C" int FUN_1003a7d0(const char *path, float x, float y, float z, int start, float ox, float oy, float oz)
{
    int slot = FindFreeEffectModel();
    if (slot >= 0)
    {
        DAT_10edbe08[slot].M_1000b9f0(path, 0, start, -1);
        DAT_10edbe08[slot].AddKey(0, x, y, z);
        DAT_10edbe08[slot].SetOffset(ox, oy, oz);
        DAT_10edbe08[slot].m_0030 = 1;
        return slot;
    }
    return -1;
}

// Same as FUN_1003a7d0 with a colour (r, g, b, a) for the effect.
extern "C" int FUN_1003a880(const char *path, float x, float y, float z, int start, float ox, float oy, float oz,
                            const Vec4 &colour)
{
    int slot = FindFreeEffectModel();
    if (slot >= 0)
    {
        DAT_10edbe08[slot].M_1000b9f0(path, 0, start, -1);
        DAT_10edbe08[slot].color.Copy(colour);
        DAT_10edbe08[slot].AddKey(0, x, y, z);
        DAT_10edbe08[slot].SetOffset(ox, oy, oz);
        DAT_10edbe08[slot].m_0030 = 1;
        return slot;
    }
    return -1;
}
