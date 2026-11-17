// Per-character effect spawning: the effect manager (DAT_10daaa80, EffectManager) starts an effect and writes
// its handle into the character's entry of DAT_10219770.

#include <string.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
};

struct EffectManager
{
    char pad_0000[0x10];
    int m_0010[6];            // 0x10: start offset of each of the six effect groups
    int pad_0028[2];
    int m_0030[6];            // 0x30: current offset of each group (reset to the start)
    int m_0048;
    int m_004c;
    int m_0050;               // 0x50: number of running effects
    int Reset();
    // Starts effect `effect` at `position`; the seven values are its options (colour, follow target, flags, ...;
    // callers mostly pass -1 and 0). Stores the effect handle in *handle.
    int M_10172350(int effect, Vec4 position, int a, int b, int c, int d, int e, int f, int g, int *handle);
};

extern "C" EffectManager DAT_10daaa80;
extern "C" int DAT_10219770[0x80];  // effect handle of each character slot

// Starts effect `effect` at `position` for character slot `index` and records its handle in that slot.
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int a, int b, int c, int d, int e, int f, int g)
{
    DAT_10daaa80.M_10172350(effect, position, a, b, c, d, e, f, g, &DAT_10219770[index]);
    return 0;
}

// Triangle batch used for effect sprites (texture_mgr.cpp TriangleBatch).
struct TriangleBatch
{
    int SetBuffer(void *buffer, int capacity);  // 0x10172a80
};

// Effect slot tables.
struct EffectRecordA
{
    int pad_0000[2];
    int id;                   // 0x08: -1 = free
    char pad_000c[0x1d0];
};

struct EffectRecordB
{
    int id;                   // -1 = free
    char pad_0004[0xc0];
};

extern "C" TriangleBatch DAT_1101c804;
extern "C" char DAT_10fc6a90[];                  // vertex buffer of the effect batch (0x400 entries)
extern "C" int DAT_118e99f8[0x80280];
extern "C" EffectRecordA DAT_1115cfe8[];
extern "C" EffectRecordB DAT_11134cc8[];
extern "C" EffectRecordB DAT_118a8888[];
extern "C" int DAT_1101d820[5];
extern "C" int DAT_11018604[5];

// Stops every effect: rebinds the effect batch to its buffer, clears all effect tables and marks their slots free
// (-1), and resets the six group offsets. The offset table has only five values in the original; the sixth group
// gets whatever was on the stack (kept as is).
int EffectManager::Reset()
{
    int offsets[6];
    DAT_1101c804.SetBuffer(DAT_10fc6a90, 0x400);
    m_0050 = 0;
    memset(DAT_118e99f8, 0, sizeof(DAT_118e99f8));
    memset(DAT_1115cfe8, 0, 840 * sizeof(EffectRecordA));
    memset(DAT_11134cc8, 0, 840 * sizeof(EffectRecordB));
    memset(DAT_118a8888, 0, 840 * sizeof(EffectRecordB));
    int i;
    for (i = 0; i < 840; i++)
        DAT_1115cfe8[i].id = -1;
    for (i = 0; i < 840; i++)
        DAT_11134cc8[i].id = -1;
    for (i = 0; i < 840; i++)
        DAT_118a8888[i].id = -1;
    // The two five-entry lookup caches.
    DAT_1101d820[0] = -1;
    DAT_1101d820[1] = -1;
    DAT_1101d820[2] = -1;
    DAT_1101d820[3] = -1;
    DAT_1101d820[4] = -1;
    DAT_11018604[0] = -1;
    DAT_11018604[1] = -1;
    DAT_11018604[2] = -1;
    DAT_11018604[3] = -1;
    DAT_11018604[4] = -1;
    offsets[0] = 0;
    offsets[1] = 0x1cc;
    offsets[2] = 0x244;
    offsets[3] = 0x2a8;
    offsets[4] = 0x2f8;
    for (i = 0; i < 6; i++)
    {
        m_0030[i] = offsets[i];
        m_0010[i] = offsets[i];
    }
    return 0;
}
