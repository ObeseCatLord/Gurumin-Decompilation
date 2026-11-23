// The effect manager (DAT_10daaa80): starts, steps, draws and removes the running particle and sprite effects.
// Methods: char_effects.cpp (Reset, Init, Update, Remove*, ClearTables, Draw), effect_start.cpp (Start),
// effect_sprites.cpp (DrawSpriteA/B); EffectOptions' constructor is in model_state.cpp.
// This is the one declaration of these types: include this header, never redeclare them.
#ifndef GURUMIN_EFFECT_MANAGER_H
#define GURUMIN_EFFECT_MANAGER_H

#include "screen_vertex.h"

#include "vec4.h"
#include "matrix4.h"

// Number of effect lists (render groups) the manager keeps.
#define EFFECT_GROUP_COUNT 6
// Number of effect lists the running records are linked into (DAT_1101d820 / DAT_11018604).
#define EFFECT_LIST_COUNT 5
// Number of effect definitions (DAT_1101dfdc), running effect records (DAT_1115cfe8), sprite records in each
// sprite table (DAT_11134cc8, DAT_118a8888) and effect lights (DAT_118e99f8).
#define EFFECT_DEFINITION_COUNT 0x47e
#define EFFECT_RECORD_COUNT 0x348
#define EFFECT_SPRITE_COUNT 0x348
#define EFFECT_LIGHT_COUNT 0x80
// A free definition, record or sprite has id (or type) -1; -1 also ends a record list.
#define EFFECT_NONE (-1)
// EffectDef::kind values EffectManager::DrawLight tells apart (1 and 0x12 are sprites). Kinds 7, 9, 0x16 and 0x17
// are drawn by their own functions; kind 0x11 is the trail made of two interleaved ribbons; any other kind with a
// light is drawn as a plain ribbon strip.
#define EFFECT_KIND_LIGHT_7 7
#define EFFECT_KIND_LIGHT_9 9
#define EFFECT_KIND_LIGHT_INTERLEAVED 0x11
#define EFFECT_KIND_LIGHT_22 0x16
#define EFFECT_KIND_LIGHT_23 0x17
// Bits of EffectOptions::flags as the running record's update pass reads them (char_effects.cpp).
#define EFFECT_FLAG_RESTRICTED_PASS 1   // also updated in Update's restricted pass (the followAttachment bit)
#define EFFECT_FLAG_HIDDEN 2            // only updated when Update includes hidden effects (the followOwner bit)

// Options a caller may pass to EffectManager::Start (0x44 bytes, copied into the effect's record). The constructor
// (0x101668b0, model_state.cpp) fills in the defaults.
struct EffectOptions
{
    int m_0000;
    int m_0004;                                      // 0x04: 0xa2c2a by default
    float m_0008;                                    // 0x08: 1.0 by default
    unsigned char list;                              // 0x0c: effect list when the definition leaves it open
    union                                            // 0x0d
    {
        unsigned char flags;                         // all flags (switch_effects.cpp stores 1 = follow attachment)
        struct
        {
            unsigned char followAttachment : 1;      // bit 0: the effect follows the matrix it was attached to
            unsigned char followOwner : 1;           // bit 1: the effect follows its owner (npc_frozen.cpp)
            unsigned char m_000d_2 : 6;
        };
    };
    char m_000e;
    char pad_000f;
    union                                            // 0x10
    {
        int m_0010;
        struct
        {
            char m_0010_0;
            char m_0010_1;
            char drawn;                              // 0x12: in a running record: 1 once drawn this frame (list 1)
            char m_0010_3;
        };
    };
    int m_0014;
    int m_0018;
    int m_001c;
    int m_0020;
    char settings[8];                                // 0x24..0x2b: byte-sized settings (settings[2] = 1 by default).
                                                     // An array: locals of this type get a /GS cookie
                                                     // (npc_frozen.cpp, char_move.cpp, switch_effects.cpp)
    int owner;                                       // 0x2c: owner (character index + 100), 0 = none
    int m_0030;
    int m_0034;
    int m_0038;
    int m_003c;
    int m_0040;
    EffectOptions() throw();                         // 0x101668b0 (model_state.cpp); callers know it cannot throw
};

// An effect definition (0x3d8 bytes; EFFECT_DEFINITION_COUNT of them at DAT_1101dfdc), one per effect type the
// scripts can create. effect_defs.cpp makes new ones, effect_start.cpp starts them, char_effects.cpp and
// effect_sprites.cpp read their settings.
struct EffectDef
{
    int id;                                          // 0x00: script id, EFFECT_NONE = free
    int kind;                                        // 0x04: 1 and 0x12 are sprites; 6, 7, 0x11 and 0x16 have lights
    char pad_0008[0x64];
    int depthTest;                                   // 0x6c: non-zero = drawn with render state 9 on
    char pad_0070[0x8];
    int startFrame;                                  // 0x78: frame the effect starts at
    char pad_007c[0x4];
    int lightFrames;                                 // 0x80: how long its light lasts (fades out over lightFrames + 10)
    char pad_0084[0x4];
    int fullAlphaB;                                  // 0x88: sprite table B: non-zero = starts at full alpha (else half)
    char pad_008c[0xc];
    int fullAlphaA;                                  // 0x98: the same for sprite table A
    char pad_009c[0x5c];
    int fixedList;                                   // 0xf8: non-zero = always started in the definition's list
    int texture;                                     // 0xfc: texture slot, -1 = not looked up yet (-1 after Init)
    int blend;                                       // 0x100: sprite blend mode 0-6
    char pad_0104[0x50];
    int m_0154;
    char pad_0158[0xc];
    int m_0164;                                      // 0x164: 50 for a new definition
    char pad_0168[0x8c];
    int spawnKind;                                   // 0x1f4: 2 and 4 are effects whose Start returns 1
    char pad_01f8[0x7];
    char name[0x20];                                 // 0x1ff: display name
    char textureName[0x40];                          // 0x21f: texture looked up into `texture`
    unsigned char list;                              // 0x25f: effect list (0 = the caller's choice)
    char flat;                                       // 0x260: sprite table A: drawn at depth 0
    char pad_0261[0x177];
};

// A running effect (0x1dc bytes; EFFECT_RECORD_COUNT of them at DAT_1115cfe8), linked into one of the
// EFFECT_LIST_COUNT lists by prev/next. EffectManager::Start (effect_start.cpp) fills it in; char_effects.cpp
// updates, draws and removes it.
struct EffectRecord
{
    int prev;                                        // 0x00: previous record of its list, -1 = first
    int next;                                        // 0x04: next record of its list, -1 = last
    int id;                                          // 0x08: effect definition, EFFECT_NONE = free
    int parent;                                      // 0x0c: record this effect is attached to, -1 = none
    int m_0010;                                      // 0x10: -1 at the start
    int frame;                                       // 0x14: start frame (the definition's plus the caller's offset)
    int m_0018;
    int m_001c;
    int serial;                                      // 0x20: value of the start counter when it started
    Vec4 partPositions[2];                           // 0x24: positions of its two parts, indexed by `part`
    char pad_0044[0x50];
    Vec4 velocity;                                   // 0x94
    char pad_00a4[0x10];
    Vec4 position;                                   // 0xb4
    char pad_00c4[0x40];
    Matrix4 matrix;                                  // 0x104: the caller's matrix, identity without one
    char pad_0144[0x40];
    const Matrix4 *matrixOwner;                      // 0x184: matrix it follows, 0 = none
    int m_0188;                                      // 0x188: the caller's value
    short m_018c;
    char part;                                       // 0x18e: 0 or 1, the part whose position decides visibility
    char pad_018f[3];
    short light;                                     // 0x192: entry of DAT_118e99f8 it uses, 0 = none
    EffectOptions options;                           // 0x194: the start options (options.owner = starting character)
    int *pause;                                      // 0x1d8: paused while it points to a non-zero value
};

// A sprite record (0xc4 bytes; EFFECT_SPRITE_COUNT in each of the tables DAT_11134cc8 and DAT_118a8888), drawn by
// effect_sprites.cpp.
struct EffectSprite
{
    int type;                                        // 0x00: effect definition of the sprite, EFFECT_NONE = free
    Vec4 position;                                   // 0x04: world position (table A) or screen position (table B)
    float colour[3];                                 // 0x14
    float alpha;                                     // 0x20
    float colour2[3];                                // 0x24
    char pad_0030[0x44];
    float size;                                      // 0x74: half the width of the square
    float m_0078;
    float u0;                                        // 0x7c
    float u1;                                        // 0x80
    char pad_0084[0x18];
    float v0;                                        // 0x9c
    float v1;                                        // 0xa0
    char pad_00a4[0x18];
    short lifetime;                                  // 0xbc: frames the record lives
    short age;                                       // 0xbe: frames it has lived
    int pad_00c0;
};

// Number of points a light's trail can hold, and of vertices of the trail strip DAT_110187c0 (two per point).
#define EFFECT_LIGHT_POINT_COUNT 0x100
#define EFFECT_TRAIL_VERTEX_COUNT 0x200

// The light of an effect (0x4014 bytes; EFFECT_LIGHT_COUNT of them at DAT_118e99f8): a trail of up to
// EFFECT_LIGHT_POINT_COUNT points, each with a width and two colours, drawn as a strip by EffectManager::DrawLight
// (effect_light.cpp). The frame counter shortens the trail once the light stops being fed.
struct EffectLight
{
    union
    {
        int active;                                  // 0x00: non-zero while in use
        int pointCount;                              // 0x00: points of the trail so far (3 are needed to draw it)
    };
    int type;                                        // 0x04: effect definition (DAT_1101dfdc) the light belongs to
    int pad_0008;
    int frame;                                       // 0x0c: frames since the light started (0 = not counting)
    float fade;                                      // 0x10: 1 at the start, falling to 0
    Vec4 points[EFFECT_LIGHT_POINT_COUNT];           // 0x14: world position of each point
    Vec4 widths[EFFECT_LIGHT_POINT_COUNT];           // 0x1014: x = width of the trail at the point, in 1/1000
    Vec4 colours[EFFECT_LIGHT_POINT_COUNT];          // 0x2014: r, g, b, a of each point (1.0 = full)
    Vec4 specular[EFFECT_LIGHT_POINT_COUNT];         // 0x3014: second colour r, g, b of each point
};

extern "C" EffectDef DAT_1101dfdc[EFFECT_DEFINITION_COUNT];      // effect definitions
extern "C" EffectRecord DAT_1115cfe8[EFFECT_RECORD_COUNT];       // running effects
extern "C" EffectSprite DAT_11134cc8[EFFECT_SPRITE_COUNT];       // sprite table A: sprites placed in the world
extern "C" EffectSprite DAT_118a8888[EFFECT_SPRITE_COUNT];       // sprite table B: sprites placed on the screen
extern "C" EffectLight DAT_118e99f8[EFFECT_LIGHT_COUNT];         // effect lights
extern "C" ScreenVertex DAT_110187c0[EFFECT_TRAIL_VERTEX_COUNT];  // vertices of the trail strip being drawn
extern "C" int DAT_1101d820[EFFECT_LIST_COUNT];                  // first record of each effect list
extern "C" int DAT_11018604[EFFECT_LIST_COUNT];                  // last record of each effect list

struct EffectManager
{
    Vec4 colour;                                     // 0x00: overall effect colour (white after Init)
    int m_0010[EFFECT_GROUP_COUNT];                  // 0x10: start offset of each of the six effect groups
    int pad_0028[2];
    int m_0030[EFFECT_GROUP_COUNT];                  // 0x30: current offset of each group (reset to the start)
    int m_0048;
    int m_004c;
    int m_0050;                                      // 0x50: number of running effects

    int Reset();                                     // 0x10184170: removes every running effect
    int Init();                                      // 0x101842a0: Reset plus the effect definitions
    void M_1016fab0();                               // 0x1016fab0 (called by Init)
    int ClearTables();                               // 0x1016fb10
    int RemoveOwnedBy(int owner);                    // 0x101883b0
    int RemoveChildren(int parent);                  // 0x10172260
    int RemoveOfType(int type);                      // 0x10188490
    void Step(int record, int list);                 // 0x10184440: advances one effect
    int Update(int restricted, int includeHidden, int ignorePause);  // 0x10188580
    void LoadDefinition(int definition);             // 0x1016fa40
    // 0x10172350 (effect_start.cpp): starts the effect with script id `effect` at `position`. `owner` is the matrix
    // it follows (0 = none), `matrix` the matrix it is placed with, `options` the start options (0 = defaults) and
    // `pause` a flag that pauses it while non-zero. Returns the effect handle.
    int Start(int effect, Vec4 position, int parent, int unused, const Matrix4 *owner, int value, int frame,
              const Matrix4 *matrix, const EffectOptions *options, int *pause);
    // 0x10190e30: draws the effects of render pass `pass`; `camera` is passed through to the draw helpers.
    int Draw(const Matrix4 &view, const Matrix4 *camera, unsigned char pass);
    int DrawRecord(int record, const Matrix4 &view, const Matrix4 *camera, int pass);   // 0x10189530
    int DrawSpriteA(int index, const Matrix4 &view, const Matrix4 *camera);            // 0x101890f0
    int DrawSpriteB(int index, const Matrix4 &view, const Matrix4 *camera);            // 0x10189340
    // 0x10187520 (effect_light.cpp): draws effect light `index` as a textured trail; the light kinds 7, 9, 0x16 and
    // 0x17 have their own versions.
    int DrawLight(int index, const Matrix4 &view, const Matrix4 *camera);
    int DrawLightKind7(int index, const Matrix4 &view, const Matrix4 *camera);         // 0x101713f0 (kinds 7 and 9)
    int DrawLightKind22(int index, const Matrix4 &view, const Matrix4 *camera);        // 0x1016fb80 (kind 0x16)
    int DrawLightKind23(int index, const Matrix4 &view, const Matrix4 *camera);        // 0x10170580 (kind 0x17)
};

// The effect manager.
extern "C" EffectManager DAT_10daaa80;

typedef char check_effect_def[sizeof(EffectDef) == 0x3d8 ? 1 : -1];
typedef char check_effect_record[sizeof(EffectRecord) == 0x1dc ? 1 : -1];
typedef char check_effect_sprite[sizeof(EffectSprite) == 0xc4 ? 1 : -1];

#endif
