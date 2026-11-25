// The effect manager (EN build): starts, steps and draws the running particle and sprite effects. Only the methods
// used so far are declared; the data layout is not recovered, so sizeof(EffectManager) is not the real size.
// This is the one declaration of EffectManager in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_EFFECT_MANAGER_H
#define GURUMIN_EN_EFFECT_MANAGER_H

#include "containers.h"
#include "vec4.h"
#include "effect_options.h"
#include "texture_mgr.h"

struct TriangleBatch;   // the vertex batch the effects draw into (texture_mgr.h)

// Number of effect definitions, and of sprite records in each of the two sprite tables.
const int EFFECT_DEFINITION_COUNT = 0x47e;
const int EFFECT_SPRITE_COUNT = 0x348;
// A free definition or sprite has type (or id) -1.
const int EFFECT_NONE = -1;
// EffectDef::kind values that are sprites (drawn by the sprite functions).
const int EFFECT_KIND_SPRITE = 1;
const int EFFECT_KIND_SPRITE_ALT = 0x12;

// Where an effect moves to (EffectDef::m_0114, 16 bytes each).
struct EffectTarget
{
    int type;               // 0x00: 1 follows the owner matrix, 2 the target matrix, 3 and 4 steer towards them, 5 steers towards a host direction, 6 sets the height
    int value;              // 0x04: turn per frame in hundredths for the steering types
    int m_0008[2];          // 0x08: not yet named
};

// An event of an effect (EffectDef::m_0174, 16 bytes each).
struct EffectTrigger
{
    int type;               // 0x00: what the event does (ends the effect, asks for a screen effect, ...)
    int frame;              // 0x04: frame it happens at
    int value;              // 0x08: screen effect id or height
    int m_000c;             // 0x0c: not yet named
};

// A child effect an effect starts (EffectDef::m_01b4, 16 bytes each).
struct EffectChild
{
    int type;               // 0x00: 1 when it ends, 2 when its hit test hits, 3 every `frame` frames, 4 at `frame`
    int frame;              // 0x04: frame or interval
    int effect;             // 0x08: effect definition started
    int count;              // 0x0c: number of effects started
};

// An effect definition (0x3d8 bytes; EFFECT_DEFINITION_COUNT of them in g_effectDefs), one per effect type the scripts
// can create. The layout equals the JP EffectDef (decomp/include/effect_manager.h): the EN sprite drawing reads kind,
// depthTest, fullAlphaA/B, texture, blend and flat at the same offsets.
struct EffectDef
{
    int id;                                          // 0x00: script id, EFFECT_NONE = free
    int kind;                                        // 0x04: 1 and 0x12 are sprites; 6, 7, 0x11 and 0x16 have lights
    int m_0008;                                      // 0x08: non-zero = the effect is parked (Step does nothing)
    int m_000c;                                      // 0x0c: not yet named
    int m_0010;                                      // 0x10: not yet named
    int m_0014;                                      // 0x14: not yet named
    int m_0018[16];                                  // 0x18: [0] non-zero = hit test with the host hook, [2], [4] its options
    int m_0058[4];                                   // 0x58: random turn of the direction: interval, then x/y/z amount
    int m_0068;                                      // 0x68: first of 0x10 bytes read from the definition file
    int depthTest;                                   // 0x6c: non-zero = drawn with the depth test on
    char m_0070[0x8];                                // 0x70: not yet named
    int startFrame;                                  // 0x78: frame the effect starts at
    int m_007c;                                      // 0x7c: first of 0x20 (old files) or 0x80 bytes of the file
    int lightFrames;                                 // 0x80: how long its light lasts (fades out over lightFrames + 10)
    char m_0084[0x4];                                // 0x84: not yet named
    int fullAlphaB;                                  // 0x88: sprite table B: non-zero = starts at full alpha (else half)
    int spawnWorldSprite;                            // 0x8c: non-zero = also leaves a sprite in table A
    int worldSpriteLifetime;                         // 0x90: frames that sprite lives
    char m_0094[4];                                  // 0x94: not yet named
    int fullAlphaA;                                  // 0x98: the same for sprite table A
    char m_009c[0x5c];                               // 0x9c: not yet named
    int fixedList;                                   // 0xf8: non-zero = always started in the definition's list
    int texture;                                     // 0xfc: texture slot, -1 = not looked up yet
    int blend;                                       // 0x100: sprite blend mode 0-6
    int m_0104[4];                                   // 0x104: position and turn track numbers used
    EffectTarget m_0114[4];                          // 0x114: where the effect moves (Step reads the first two)
    int m_0154;                                      // 0x154: not yet named
    char m_0158[0xc];                                // 0x158: not yet named
    int m_0164;                                      // 0x164: frames the effect lives (50 for a new definition)
    char m_0168[0xc];                                // 0x168: not yet named
    EffectTrigger m_0174[4];                         // 0x174: events at a frame or when something happens
    EffectChild m_01b4[4];                           // 0x1b4: effects it starts
    int spawnKind;                                   // 0x1f4: 2 and 4 are effects whose Start returns 1
    char trackFlags[7];                              // 0x1f8: one byte after each of tracks 3 to 9 in the file
    char name[0x20];                                 // 0x1ff: display name
    char textureName[0x40];                          // 0x21f: texture looked up into `texture`
    unsigned char list;                              // 0x25f: effect list (0 = the caller's choice)
    char flat;                                       // 0x260: sprite table A: drawn at depth 0
    char m_0261;                                     // 0x261: not yet named
    char m_0262[2];                                  // 0x262: not yet named
    Vec4 m_0264[8];                                  // 0x264: spawn offset, direction, turn ranges, accelerations
    KeyTrack tracks[12];                             // 0x2e4: key tracks of the effect
    float m_03d4;                                    // 0x3d4: random spread of the start position
};

// A sprite record (0xc4 bytes; EFFECT_SPRITE_COUNT in each of the tables g_effectSpritesA and g_effectSpritesB).
struct EffectSprite
{
    int type;                                        // 0x00: effect definition of the sprite, EFFECT_NONE = free
    Vec4 position;                                   // 0x04: world position (table A) or screen position (table B)
    float colour[3];                                 // 0x14: colour
    float alpha;                                     // 0x20: opacity
    float colour2[3];                                // 0x24: second colour
    char m_0030[0x44];                               // 0x30: not yet named
    float size;                                      // 0x74: half the width of the square
    float m_0078;                                    // 0x78: not yet named
    float u0;                                        // 0x7c: left texture coordinate
    float u1;                                        // 0x80: right texture coordinate
    char m_0084[0x18];                               // 0x84: not yet named
    float v0;                                        // 0x9c: top texture coordinate
    float v1;                                        // 0xa0: bottom texture coordinate
    char m_00a4[0x18];                               // 0xa4: not yet named
    short lifetime;                                  // 0xbc: frames the record lives
    short age;                                       // 0xbe: frames it has lived
    char list;                                       // 0xc0: effect list of the record that left it
    char m_00c1[3];                                  // 0xc1: padding
};

// Number of running effect records, effect lights, effect lists and effect groups.
const int EFFECT_RECORD_COUNT = 0x348;
const int EFFECT_LIGHT_COUNT = 0x80;
const int EFFECT_LIST_COUNT = 5;
const int EFFECT_GROUP_COUNT = 6;
// Capacity of the effect vertex batch and the first-record offset of each effect group.
const int EFFECT_BATCH_CAPACITY = 0x400;
const int EFFECT_GROUP_START_1 = 0x1cc;
const int EFFECT_GROUP_START_2 = 0x244;
const int EFFECT_GROUP_START_3 = 0x2a8;
const int EFFECT_GROUP_START_4 = 0x2f8;
// Points a light's trail holds.
const int EFFECT_LIGHT_POINT_COUNT = 0x100;
// A record, sprite or list head that is free or ends a list holds -1.
// EffectRecord::light value of a record without a light.
const int EFFECT_NO_LIGHT = 0;

// A running effect (0x1dc bytes; EFFECT_RECORD_COUNT of them in g_effectRecords), linked into one of the
// EFFECT_LIST_COUNT lists by prev/next. EffectManager::Start fills it in.
struct EffectRecord
{
    int prev;                                        // 0x00: previous record of its list, -1 = first
    int next;                                        // 0x04: next record of its list, -1 = last
    int id;                                          // 0x08: effect definition, EFFECT_NONE = free
    int parent;                                      // 0x0c: record this effect is attached to, -1 = none
    char m_0010[0x192 - 0x10];                       // 0x10: not yet named (timing, positions, matrices)
    short light;                                     // 0x192: entry of g_effectLights it uses, 0 = none
    EffectOptions options;                           // 0x194: the start options (options.owner = starting character)
    int *pause;                                      // 0x1d8: paused while it points to a non-zero value
};

// The light of an effect (0x4014 bytes; EFFECT_LIGHT_COUNT of them in g_effectLights): a trail of points.
struct EffectLight
{
    int active;                                      // 0x00: non-zero while in use
    char m_0004[0x4014 - 4];                         // 0x04: not yet named (trail points, widths and colours)
};

extern "C" EffectDef g_effectDefs[EFFECT_DEFINITION_COUNT];          // EN 0x01dd0354 (JP DAT_1101dfdc): effect definitions
extern "C" EffectSprite g_effectSpritesA[EFFECT_SPRITE_COUNT];       // EN 0x01da7ec0: world sprites (table A)
extern "C" EffectSprite g_effectSpritesB[EFFECT_SPRITE_COUNT];       // EN 0x020f17b0: screen sprites (table B)
extern "C" EffectRecord g_effectRecords[EFFECT_RECORD_COUNT];        // EN 0x02119bd0: running effects
extern "C" EffectLight g_effectLights[EFFECT_LIGHT_COUNT];           // EN 0x01ee7870: effect lights
extern "C" int g_effectListFirst[EFFECT_LIST_COUNT];                 // EN 0x01674770: first record of each effect list
extern "C" int g_effectListLast[EFFECT_LIST_COUNT];                  // EN 0x01674790: last record of each effect list
extern "C" TriangleBatch g_effectBatch;                              // EN 0x016747b0: vertex batch of the effects
extern "C" ScreenVertex g_effectBatchVertices[EFFECT_BATCH_CAPACITY];   // EN 0x016747d0: its buffer

typedef char check_effect_def[sizeof(EffectDef) == 0x3d8 ? 1 : -1];
typedef char check_effect_sprite[sizeof(EffectSprite) == 0xc4 ? 1 : -1];
typedef char check_effect_record[sizeof(EffectRecord) == 0x1dc ? 1 : -1];
typedef char check_effect_light[sizeof(EffectLight) == 0x4014 ? 1 : -1];

// Script ids of the most recently started effects (32 entries) and the next entry to overwrite.
const int EFFECT_RECENT_COUNT = 32;
extern "C" int g_recentEffectIds[EFFECT_RECENT_COUNT];   // EN 0x01674670 (JP DAT_110186a8): set to -1 by Init
extern "C" char g_recentEffectNext;                      // EN 0x01664469 (JP DAT_1101de64): next entry of that cache
// Takes the first free effect definition, names it "not set" and resets the lookup caches; returns its index or -1.
// EN 0x73ede0 (JP 0x1016f900). Called by EffectManager::Init.
extern "C" int EffectDefs_Allocate(void);
// The bytes of the effect definition table as Init clears them: EFFECT_DEFINITION_COUNT * sizeof(EffectDef) bytes
// starting 0x174 bytes before g_effectDefs (the same offset as in the JP build, DAT_1101de68).
extern "C" char g_effectDefStorage[EFFECT_DEFINITION_COUNT * sizeof(EffectDef)];   // EN 0x01dd01e0

struct EffectManager
{
    Vec4 colour;                                     // 0x00: effect colour (set to white by Init)
    int groupStart[EFFECT_GROUP_COUNT];              // 0x10: first record offset of each effect group
    char m_0028[8];                                  // 0x28: not yet named
    int groupCurrent[EFFECT_GROUP_COUNT];            // 0x30: current offset of each group (reset to the start)
    char m_0048[8];                                  // 0x48: not yet named
    int runningCount;                                // 0x50: number of running effects
    // Starts the effect with script id `effect` at `position`. `owner` is the matrix it follows (0 = none), `matrix` the
    // matrix it is placed with, `options` the start options (0 = defaults) and `pause` a flag that pauses it while
    // non-zero. Returns the effect handle. EN 0x70a7f0 (JP 0x10172350).
    // Draws the running effects with `view` and the camera matrix `camera`. EN 0x725f40 (JP 0x10190e30).
    void Draw(const Matrix4 &view, Matrix4 *camera, int flag);
    // Queues sprite `index` of table A (world position, projected with `view`) / table B (screen position) into the
    // effect batch. EN 0x725990 (JP 0x101890f0) / EN 0x725440 (JP 0x10189340).
    int DrawSpriteA(int index, const Matrix4 &view, const Matrix4 *camera);
    int DrawSpriteB(int index, const Matrix4 &view, const Matrix4 *camera);
    int RemoveChildren(int parent);   // EN 0x747950 (JP 0x10172260): stops every effect attached to record `parent`
    int RemoveOfType(int type);       // EN 0x747a50 (JP 0x10188490): stops every effect whose definition has type `type`
    int RemoveOwnedBy(int owner);     // EN 0x747b50 (JP 0x101883b0): stops every effect started by `owner` (character index + 100)
    // Full initialisation at load time: Reset plus the node pool release and the effect definitions. Returns 0. EN 0x734e60 (JP 0x101842a0).
    int Init();
    int AddDefaultDefinition();   // EN 0x73ede0 (JP 0x1016f900): adds the default effect definition to the table (not ported yet)
    // Marks the effect definitions' texture (field 0xfc) and both sprite tables free (-1); inline in the EN Init.
    int ClearTables()
    {
        int i;
        for (i = 0; i < EFFECT_DEFINITION_COUNT; i++)
            g_effectDefs[i].texture = EFFECT_NONE;
        for (i = 0; i < EFFECT_SPRITE_COUNT; i++)
            g_effectSpritesA[i].type = EFFECT_NONE;
        for (i = 0; i < EFFECT_SPRITE_COUNT; i++)
            g_effectSpritesB[i].type = EFFECT_NONE;
        return 0;
    }
    int Reset();   // EN 0x734ce0 (JP 0x10184170): removes every running effect
    int Start(int effect, Vec4 position, int parent, int unused, const Matrix4 *owner, int value, int frame,
              const Matrix4 *matrix, const EffectOptions *options, int *pause);
};

extern "C" EffectManager g_effectManager;   // EN 0x01487f98 (JP DAT_10daaa80)

#endif
