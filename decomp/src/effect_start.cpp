// Starting an effect: an effect definition (by script id) gets a free record in one of the five effect lists
// (char_effects.cpp updates and draws them). This unit lays out the record fields the start fills in.

#include "vec4.h"

#include "matrix4.h"

// Start options a caller may pass (0x44 bytes, copied into the record).
struct EffectOptions
{
    int m_0000;
    int m_0004;
    int m_0008;
    unsigned char list;                              // 0x0c: effect list when the definition leaves it open
    char m_000d;
    char m_000e;
    char pad_000f;
    int m_0010;
    char pad_0014[0x10];
    char m_0024;
    char m_0025;
    char m_0026;
    char m_0027;
    char m_0028;
    char m_0029;
    char m_002a;
    char pad_002b;
    int m_002c;
    char pad_0030[0x14];
};

// An effect record (char_effects.cpp EffectRecordA), 0x1dc bytes.
struct EffectRecord
{
    int prev;                 // 0x00: previous record of its list, -1 = first
    int next;                 // 0x04: next record of its list, -1 = last
    int id;                   // 0x08: effect definition, -1 = free
    int parent;               // 0x0c: record this effect is attached to, -1 = none
    int m_0010;               // 0x10: -1 at the start
    int frame;                // 0x14: start frame (the definition's plus the caller's offset)
    int m_0018;
    int m_001c;
    int serial;               // 0x20: start counter
    char pad_0024[0x70];
    Vec4 velocity;            // 0x94
    char pad_00a4[0x10];
    Vec4 position;            // 0xb4
    char pad_00c4[0x40];
    Matrix4 matrix;           // 0x104: the caller's matrix, identity without one
    char pad_0144[0x40];
    int matrixOwner;          // 0x184
    int m_0188;
    short m_018c;
    char part;                // 0x18e: 1 at the start
    char pad_018f[3];
    short light;              // 0x192: 0 = no light
    EffectOptions options;    // 0x194
    int *pause;               // 0x1d8: paused while it points to a non-zero value
};

// An effect definition (effect_defs.cpp EffectDef), 0x3d8 bytes.
struct EffectDef
{
    int id;                   // script id, -1 = free
    char pad_0004[0x74];
    int startFrame;           // 0x78
    char pad_007c[0x7c];
    int fixedList;            // 0xf8: non-zero = always the definition's list
    int texture;              // 0xfc: texture slot, -1 = not looked up yet
    char pad_0100[0xf4];
    int kind;                 // 0x1f4: 2 and 4 are effects the caller keeps
    char pad_01f8[0x27];
    char textureName[0x40];   // 0x21f
    unsigned char list;       // 0x25f: effect list (0 = the caller's choice)
    char pad_0260[0x178];
};

struct EffectManager
{
    void LoadDefinition(int definition);             // 0x1016fa40
    int Start(int effect, Vec4 position, int parent, int unused, int matrixOwner, int value, int frame,
              const Matrix4 *matrix, const EffectOptions *options, int *pause);
};

extern "C" EffectDef DAT_1101dfdc[0x47e];
extern "C" EffectRecord DAT_1115cfe8[0x348];
extern "C" int DAT_110186a8[32];                     // recently started script ids
extern "C" int DAT_10fa73f8[32];                     // their definitions
extern "C" unsigned char DAT_1101de64;               // next entry of that cache
extern "C" int DAT_1101d820[5];                      // first record of each list
extern "C" int DAT_11018604[5];                      // last record of each list
extern "C" int DAT_1101de60;                         // effects started so far
extern "C" int DAT_10ff1df8[0x47e];                  // 1 once a definition's resources are loaded
extern "C" int FUN_10161920(const char *name);       // Texture_FindByName

// 0x10172350 (EffectManager::Start). Starts the effect with script id `effect` at `position`: finds its definition
// (through a 32-entry cache), takes the first free record after the last one of the definition's list (wrapping
// once around the 0x348 records), links it at the list's end and fills it in: parent record, matrix owner, value,
// start frame, the caller's matrix and options (or defaults) and the pause flag. Looks up the texture and loads
// the definition's resources on first use. Returns 1 for effects of kind 2 and 4, 0 otherwise or when there is no
// definition or free record.
int EffectManager::Start(int effect, Vec4 position, int parent, int unused, int matrixOwner, int value, int frame,
                         const Matrix4 *matrix, const EffectOptions *options, int *pause)
{
    int i;
    for (i = 0; i < 32; i++)
    {
        if (DAT_110186a8[i] == effect)
            break;
    }
    int definition;
    if (i == 32 || (definition = DAT_10fa73f8[i]) < 0)
    {
        for (definition = 0; definition < 0x47e; definition++)
        {
            if (DAT_1101dfdc[definition].id == effect)
                break;
        }
        if (definition == 0x47e)
            return 0;
    }
    DAT_110186a8[DAT_1101de64] = effect;
    DAT_10fa73f8[DAT_1101de64++] = definition;
    if (DAT_1101de64 > 0x1f)
        DAT_1101de64 = 0;
    int list;
    if (options != 0)
    {
        unsigned char own = DAT_1101dfdc[definition].list;
        if (own != 0)
            list = own;
        else
            list = options->list;
        if (DAT_1101dfdc[definition].fixedList != 0)
            list = own;
    }
    else
        list = DAT_1101dfdc[definition].list;
    int last = DAT_11018604[list];
    int candidate = last + 1;
    if (candidate < 0)
        candidate = 0;
    int index;
    int tries = 0;
    for (;;)
    {
        index = candidate;
        if (candidate >= 0x348)
            index = candidate - 0x348;
        if (DAT_1115cfe8[index].id == -1)
            break;
        tries++;
        candidate++;
        if (tries >= 0x348)
            return 0;
    }
    DAT_1115cfe8[index].prev = last;
    if (DAT_1101d820[list] == -1)
        DAT_1101d820[list] = index;
    if (last != -1)
        DAT_1115cfe8[last].next = index;
    DAT_1115cfe8[index].pause = pause;
    DAT_1115cfe8[index].next = -1;
    DAT_11018604[list] = index;
    if (DAT_1101dfdc[definition].texture < 0)
        DAT_1101dfdc[definition].texture = FUN_10161920(DAT_1101dfdc[definition].textureName);
    if (DAT_10ff1df8[definition] == 0)
        LoadDefinition(definition);
    DAT_1115cfe8[index].m_0188 = value;
    DAT_1115cfe8[index].matrixOwner = matrixOwner;
    DAT_1115cfe8[index].serial = DAT_1101de60;
    DAT_1115cfe8[index].parent = parent;
    DAT_1115cfe8[index].id = definition;
    DAT_1115cfe8[index].m_0010 = -1;
    DAT_1115cfe8[index].frame = DAT_1101dfdc[definition].startFrame + frame;
    DAT_1115cfe8[index].position.Copy(position);
    DAT_1115cfe8[index].velocity.Fill(0.0f);
    DAT_1115cfe8[index].m_001c = 0;
    DAT_1115cfe8[index].m_0018 = 0;
    DAT_1115cfe8[index].light = 0;
    if (options != 0)
    {
        DAT_1115cfe8[index].options = *options;
        if (DAT_1101dfdc[definition].list != 0)
            DAT_1115cfe8[index].options.list = DAT_1101dfdc[definition].list;
        if (DAT_1101dfdc[definition].fixedList != 0)
            DAT_1115cfe8[index].options.list = DAT_1101dfdc[definition].list;
    }
    else
    {
        DAT_1115cfe8[index].options.m_0028 = 0;
        DAT_1115cfe8[index].options.m_0024 = 0;
        DAT_1115cfe8[index].options.m_0025 = 0;
        DAT_1115cfe8[index].options.m_0026 = 0;
        DAT_1115cfe8[index].options.m_000e = 0;
        DAT_1115cfe8[index].options.m_0029 = 0;
        DAT_1115cfe8[index].options.m_0008 = 0;
        DAT_1115cfe8[index].options.m_0027 = 0;
        DAT_1115cfe8[index].options.m_002a = 0;
        DAT_1115cfe8[index].options.m_0010 = 0;
        DAT_1115cfe8[index].options.m_0004 = 0;
        DAT_1115cfe8[index].options.m_0000 = 0;
        DAT_1115cfe8[index].options.m_002c = 0;
        DAT_1115cfe8[index].options.list = DAT_1101dfdc[definition].list;
    }
    DAT_1115cfe8[index].part = 1;
    DAT_1115cfe8[index].m_018c = 0;
    if (matrix != 0)
        DAT_1115cfe8[index].matrix = *matrix;
    else
        DAT_1115cfe8[index].matrix.SetIdentity();
    DAT_1101de60++;
    if (DAT_1101dfdc[definition].kind == 2 || DAT_1101dfdc[definition].kind == 4)
        return 1;
    return 0;
}
typedef char check_record[sizeof(EffectRecord) == 0x1dc ? 1 : -1];
typedef char check_def[sizeof(EffectDef) == 0x3d8 ? 1 : -1];
typedef char check_options[sizeof(EffectOptions) == 0x44 ? 1 : -1];
