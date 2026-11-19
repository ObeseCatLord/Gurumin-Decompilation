// Per-character effect spawning: the effect manager (DAT_10daaa80, EffectManager) starts an effect and writes
// its handle into the character's entry of DAT_10219770.

#include <string.h>

#include <stdio.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    int InRect(const struct Box2 *box);          // 0x101ac7c0 (vec_math.cpp)
};

// Four floats with the Vec4 setter (0x101ac9c0).
struct Colour4
{
    float r;
    float g;
    float b;
    float a;
    void Set(float r, float g, float b, float a);
};

struct EffectManager
{
    Colour4 colour;           // 0x00: overall effect colour (white after Init)
    int m_0010[6];            // 0x10: start offset of each of the six effect groups
    int pad_0028[2];
    int m_0030[6];            // 0x30: current offset of each group (reset to the start)
    int m_0048;
    int m_004c;
    int m_0050;               // 0x50: number of running effects
    int Reset();
    int Init();
    void M_1016fab0();
    int ClearTables();
    int RemoveOwnedBy(int owner);
    int RemoveChildren(int parent);
    int RemoveOfType(int type);
    void Step(int record, int list);  // 0x10184440: advances one effect
    int Update(int restricted, int includeHidden, int ignorePause);
    // Starts effect `effect` at `position`; the seven values are its options (colour, follow target, flags, ...;
    // callers mostly pass -1 and 0). Stores the effect handle in *handle.
    int M_10172350(int effect, Vec4 position, int a, int b, int c, int d, int e, int f, int g, int *handle);
    int Draw(const struct Matrix4 &view, int unused, unsigned char pass);
    int DrawRecord(int record, const struct Matrix4 &view, int unused, int pass);   // 0x10189530
    int DrawSpriteA(int index, const struct Matrix4 &view, int unused);            // 0x101890f0
    int DrawSpriteB(int index, const struct Matrix4 &view, int unused);            // 0x10189340
    int DrawLight(int index, const struct Matrix4 &view, int unused);              // 0x10187520
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
    int Reset();                                // 0x10172aa0
    void Flush();                               // 0x101733f0
};

// Effect slot tables.
struct EffectRecordA
{
    int prev;                 // 0x00: previous record of its list, -1 = first
    int next;                 // 0x04: next record of its list, -1 = last
    int id;                   // 0x08: -1 = free
    int parent;               // 0x0c: record this effect is attached to, -1 = none
    char pad_0010[0x14];
    Vec4 positions[0x16];     // 0x24: positions of the effect's parts
    char pad_0184[0xa];
    char part;                // 0x18e: the part whose position decides visibility
    char pad_018f[3];
    short light;              // 0x192: entry of DAT_118e99f8 (0x4014-byte records) this effect uses, 0 = none
    char pad_0194[0xd];
    unsigned char flags;      // 0x1a1: 1 = also updated in the restricted pass, 2 = only updated when hidden effects are
    char pad_01a2[4];
    char drawn;               // 0x1a6: 1 once drawn this frame (list 1)
    char pad_01a7[0x19];
    int owner;                // 0x1c0: character slot that started the effect
    char pad_01c4[0x14];
    int *pause;               // 0x1d8: when it points to a non-zero value the effect is paused
};

struct EffectRecordB
{
    int id;                   // -1 = free
    char pad_0004[0xb8];
    short lifetime;           // 0xbc: frames the record lives
    short age;                // 0xbe: frames it has lived
    int pad_00c0;
};

// Effect definition (effect_defs.cpp EffectDef): `id` is the effect type a script refers to.
struct EffectType
{
    int id;
    char pad_0004[0x7c];
    int lightFrames;          // 0x80: how long the effect's light lasts (it fades out over lightFrames + 10)
    char pad_0084[0x78];
    int m_00fc;               // 0xfc: -1 after Init
    char pad_0100[0x2d8];
};

extern "C" EffectType DAT_1101dfdc[0x47e];
extern "C" TriangleBatch DAT_1101c804;
extern "C" char DAT_10fc6a90[];                  // vertex buffer of the effect batch (0x400 entries)
// Light of an effect, 0x4014 bytes.
struct EffectLight
{
    int active;
    int type;                 // 0x04: effect definition (DAT_1101dfdc) the light belongs to
    int pad_0008;
    int frame;                // 0x0c: frames since the light started (0 = not counting)
    float fade;               // 0x10: 1 at the start, falling to 0
    char pad_0014[0x4000];
};

extern "C" EffectLight DAT_118e99f8[0x80];
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

// 0x101883b0. Stops every running effect started by character slot `owner`: each one is marked free, unlinked from
// its list (five lists with heads DAT_1101d820 and tails DAT_11018604) and its light entry is cleared; the
// effects attached to it are stopped too. Returns 0.
int EffectManager::RemoveOwnedBy(int owner)
{
    for (int list = 0; list < 5; list++)
    {
        for (int i = DAT_1101d820[list]; i > -1; i = DAT_1115cfe8[i].next)
        {
            if (DAT_1115cfe8[i].id > -1 && DAT_1115cfe8[i].owner == owner)
            {
                DAT_1115cfe8[i].id = -1;
                if (DAT_1115cfe8[i].prev == -1)
                    DAT_1101d820[list] = DAT_1115cfe8[i].next;
                else
                    DAT_1115cfe8[DAT_1115cfe8[i].prev].next = DAT_1115cfe8[i].next;
                if (DAT_1115cfe8[i].next != -1)
                    DAT_1115cfe8[DAT_1115cfe8[i].next].prev = DAT_1115cfe8[i].prev;
                else
                    DAT_11018604[list] = DAT_1115cfe8[i].prev;
                if (DAT_1115cfe8[i].light != 0)
                    DAT_118e99f8[DAT_1115cfe8[i].light].active = 0;
                RemoveChildren(i);
            }
        }
    }
    return 0;
}

// 0x10172260. Stops every running effect attached to record `parent`, and the effects attached to those in turn:
// each one is marked free, unlinked from its list, detached and its light entry cleared. Returns 0.
int EffectManager::RemoveChildren(int parent)
{
    for (int list = 0; list < 5; list++)
    {
        for (int i = DAT_1101d820[list]; i > -1; i = DAT_1115cfe8[i].next)
        {
            if (DAT_1115cfe8[i].id != -1 && DAT_1115cfe8[i].parent == parent)
            {
                DAT_1115cfe8[i].id = -1;
                if (DAT_1115cfe8[i].prev == -1)
                    DAT_1101d820[list] = DAT_1115cfe8[i].next;
                else
                    DAT_1115cfe8[DAT_1115cfe8[i].prev].next = DAT_1115cfe8[i].next;
                if (DAT_1115cfe8[i].next != -1)
                    DAT_1115cfe8[DAT_1115cfe8[i].next].prev = DAT_1115cfe8[i].prev;
                else
                    DAT_11018604[list] = DAT_1115cfe8[i].prev;
                DAT_1115cfe8[i].parent = -1;
                if (DAT_1115cfe8[i].light != 0)
                    DAT_118e99f8[DAT_1115cfe8[i].light].active = 0;
                RemoveChildren(i);
            }
        }
    }
    return 0;
}

// 0x10188490. Stops every running effect whose definition has type `type` (the record's id indexes the definition
// table DAT_1101dfdc), including the effects attached to it. Returns 0.
int EffectManager::RemoveOfType(int type)
{
    for (int list = 0; list < 5; list++)
    {
        for (int i = DAT_1101d820[list]; i > -1; i = DAT_1115cfe8[i].next)
        {
            if (DAT_1115cfe8[i].id > -1 && DAT_1101dfdc[DAT_1115cfe8[i].id].id == type)
            {
                DAT_1115cfe8[i].id = -1;
                if (DAT_1115cfe8[i].prev == -1)
                    DAT_1101d820[list] = DAT_1115cfe8[i].next;
                else
                    DAT_1115cfe8[DAT_1115cfe8[i].prev].next = DAT_1115cfe8[i].next;
                if (DAT_1115cfe8[i].next != -1)
                    DAT_1115cfe8[DAT_1115cfe8[i].next].prev = DAT_1115cfe8[i].prev;
                else
                    DAT_11018604[list] = DAT_1115cfe8[i].prev;
                if (DAT_1115cfe8[i].light != 0)
                    DAT_118e99f8[DAT_1115cfe8[i].light].active = 0;
                RemoveChildren(i);
            }
        }
    }
    return 0;
}

// 0x10188580. Advances all effects by one frame: every running effect is stepped unless it is hidden (flag 2,
// skipped unless `includeHidden`), outside a restricted pass (`restricted` steps only effects with flag 1) or paused
// (unless `ignorePause`). The short-lived records of the two other tables expire when their age reaches their
// lifetime, and effect lights fade out over the effect's light frames plus 10. Returns 0.
int EffectManager::Update(int restricted, int includeHidden, int ignorePause)
{
    for (int list = 0; list < 5; list++)
    {
        for (int i = DAT_1101d820[list]; i > -1; i = DAT_1115cfe8[i].next)
        {
            if (DAT_1115cfe8[i].id != -1 && (includeHidden != 0 || (DAT_1115cfe8[i].flags & 2) == 0) &&
                (restricted == 0 || (DAT_1115cfe8[i].flags & 1) != 0) &&
                (DAT_1115cfe8[i].pause == 0 || *DAT_1115cfe8[i].pause == 0 || ignorePause != 0))
                Step(i, list);
        }
    }
    int j;
    for (j = 0; j < 840; j++)
    {
        if (DAT_11134cc8[j].id != -1)
        {
            DAT_11134cc8[j].age++;
            if (DAT_11134cc8[j].age >= DAT_11134cc8[j].lifetime)
                DAT_11134cc8[j].id = -1;
        }
    }
    for (j = 0; j < 840; j++)
    {
        if (DAT_118a8888[j].id != -1)
        {
            DAT_118a8888[j].age++;
            if (DAT_118a8888[j].age >= DAT_118a8888[j].lifetime)
                DAT_118a8888[j].id = -1;
        }
    }
    for (int k = 0; k < 0x80; k++)
    {
        if (DAT_118e99f8[k].active != 0 && DAT_118e99f8[k].frame != 0)
        {
            int frame = DAT_118e99f8[k].frame + 1;
            int frames = DAT_1101dfdc[DAT_118e99f8[k].type].lightFrames;
            DAT_118e99f8[k].frame = frame;
            DAT_118e99f8[k].fade = 1.0f - (float)frame / (float)(frames + 10);
            if (frame >= frames + 10)
            {
                DAT_118e99f8[k].fade = 0;
                DAT_118e99f8[k].frame = 0;
                DAT_118e99f8[k].active = 0;
            }
        }
    }
    return 0;
}

// 0x1016fb10. Marks the effect definitions (field 0xfc) and both short-lived record tables free (-1). Defined
// before Init: Init relies on this function leaving edx alone. Returns 0.
int EffectManager::ClearTables()
{
    int i;
    for (i = 0; i < 0x47e; i++)
        DAT_1101dfdc[i].m_00fc = -1;
    for (i = 0; i < 840; i++)
        DAT_11134cc8[i].id = -1;
    for (i = 0; i < 840; i++)
        DAT_118a8888[i].id = -1;
    return 0;
}

extern "C" char DAT_1101de68[0x114450];   // effect definitions with the data before them (cleared by Init)
extern "C" int DAT_110186a8[32];
extern "C" char DAT_1101de64;
extern "C" int FUN_1016f900(void);        // adds the default effect definition (effect_defs.cpp)

// 0x101842a0. Full initialisation at load time: Reset plus the effect definitions. The colour becomes white, every
// table is cleared and marked free, the lists emptied, the six group offsets set and the default definition added.
// The sixth offset is uninitialised in the original, as in Reset. Returns 0.
int EffectManager::Init()
{
    int offsets[6];
    colour.Set(1.0f, 1.0f, 1.0f, 1.0f);
    DAT_1101c804.SetBuffer(DAT_10fc6a90, 0x400);
    M_1016fab0();
    m_0050 = 0;
    memset(DAT_118e99f8, 0, sizeof(DAT_118e99f8));
    memset(DAT_1115cfe8, 0, 840 * sizeof(EffectRecordA));
    memset(DAT_1101de68, 0, sizeof(DAT_1101de68));
    memset(DAT_11134cc8, 0, 840 * sizeof(EffectRecordB));
    memset(DAT_118a8888, 0, 840 * sizeof(EffectRecordB));
    int i;
    for (i = 0; i < 32; i++)
        DAT_110186a8[i] = -1;
    DAT_1101de64 = 0;
    for (i = 0; i < 840; i++)
        DAT_1115cfe8[i].id = -1;
    for (i = 0; i < 840; i++)
        DAT_11134cc8[i].id = -1;
    for (i = 0; i < 840; i++)
        DAT_118a8888[i].id = -1;
    for (i = 0; i < 0x47e; i++)
    {
        DAT_1101dfdc[i].id = -1;
        DAT_1101dfdc[i].m_00fc = -1;
    }
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
    ClearTables();
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
    FUN_1016f900();
    return 0;
}

extern "C" struct Box2 DAT_10f668a8;                 // the visible area
extern "C" float DAT_10f668c0;                       // its depth limit
extern "C" void FUN_1013f070(int mode, int a, int b);   // sets a 2D/3D render state

// Effect records whose slot is free must not be in a list: crash on purpose (fread on a null file).
#define CHECK_RECORD(i)     if (DAT_1115cfe8[i].id == -1)         fread(0, 1, 1, 0)

// 0x10190e30 (EffectManager::Draw). Draws the effects of render pass `pass`: pass 1 draws the list-1 effects whose
// part is in the visible area and in front of its depth limit (+20), marking them drawn; other passes draw their own
// list, pass 0 also the list-1 effects not drawn yet. Then the sprites of both tables in the pass's range, and in
// pass 0 the effect lights. Returns 0.
int EffectManager::Draw(const Matrix4 &view, int unused, unsigned char pass)
{
    FUN_1013f070(3, 1, 0xf);
    FUN_1013f070(7, 1, 0);
    FUN_1013f070(9, 0, 0);
    int ranges[10] = {0, 0x1cc, 0x244, 0x2a8, 0x2f8, 0x1cc, 0x244, 0x2a8, 0x2f8, 0x348};   // first, then end per pass
    DAT_1101c804.Reset();
    int i;
    if (pass == 1)
    {
        for (i = DAT_1101d820[1]; i > -1; i = DAT_1115cfe8[i].next)
        {
            if (DAT_1115cfe8[i].id == -1)
                fread(0, 1, 1, 0);
            else
            {
                if (DAT_1115cfe8[i].positions[DAT_1115cfe8[i].part].InRect(&DAT_10f668a8) != 0 &&
                    DAT_10f668c0 + 20.0f > DAT_1115cfe8[i].positions[DAT_1115cfe8[i].part].z)
                {
                    FUN_1013f070(10, 1, 0);
                    DrawRecord(i, view, unused, 1);
                    DAT_1115cfe8[i].drawn = 1;
                }
                else
                    DAT_1115cfe8[i].drawn = 0;
                CHECK_RECORD(i);
            }
        }
    }
    else
    {
        for (i = DAT_1101d820[pass]; i > -1; i = DAT_1115cfe8[i].next)
        {
            if (DAT_1115cfe8[i].id == -1)
                fread(0, 1, 1, 0);
            else
            {
                FUN_1013f070(10, 1, 0);
                DrawRecord(i, view, unused, pass);
                CHECK_RECORD(i);
            }
        }
        if (pass == 0)
        {
            for (i = DAT_1101d820[1]; i > -1; i = DAT_1115cfe8[i].next)
            {
                if (DAT_1115cfe8[i].id == -1)
                    fread(0, 1, 1, 0);
                else
                {
                    if (DAT_1115cfe8[i].drawn == 0)
                    {
                        FUN_1013f070(10, 1, 0);
                        DrawRecord(i, view, unused, 0);
                        DAT_1115cfe8[i].drawn = 1;
                    }
                    CHECK_RECORD(i);
                }
            }
        }
    }
    FUN_1013f070(3, 1, 0xf);
    FUN_1013f070(7, 1, 0);
    FUN_1013f070(10, 1, 0);
    int first = ranges[pass];
    int end = ranges[pass + 5];
    int sprite;
    for (sprite = first; sprite < end; sprite++)
    {
        if (DAT_11134cc8[sprite].id != -1)
            DrawSpriteA(sprite, view, unused);
    }
    FUN_1013f070(10, 1, 0);
    for (sprite = first; sprite < end; sprite++)
    {
        if (DAT_118a8888[sprite].id != -1)
            DrawSpriteB(sprite, view, unused);
    }
    if (pass == 0)
    {
        for (i = 0; i < 0x80; i++)
        {
            if (DAT_118e99f8[i].active != 0)
                DrawLight(i, view, unused);
        }
    }
    DAT_1101c804.Flush();
    FUN_1013f070(0, 0, 0);
    FUN_1013f070(9, 1, 0);
    FUN_1013f070(0xc, 1, 0);
    FUN_1013f070(10, 0, 0);
    return 0;
}
