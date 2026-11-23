// The effect manager (DAT_10daaa80): starts, steps, draws and removes the running particle and sprite effects.
// Methods: char_effects.cpp (Reset, Init, Update, Remove*, ClearTables, Draw), effect_start.cpp (Start),
// effect_sprites.cpp (DrawSpriteA/B); EffectOptions' constructor is in model_state.cpp.
// This is the one declaration of these types: include this header, never redeclare them.
#ifndef GURUMIN_EFFECT_MANAGER_H
#define GURUMIN_EFFECT_MANAGER_H

#include "vec4.h"
#include "matrix4.h"

// Number of effect lists (render groups) the manager keeps.
#define EFFECT_GROUP_COUNT 6

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
    int m_0010;
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
    EffectOptions();                                 // 0x101668b0 (model_state.cpp)
};

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
    int DrawLight(int index, const Matrix4 &view, const Matrix4 *camera);              // 0x10187520
};

// The effect manager.
extern "C" EffectManager DAT_10daaa80;

#endif
