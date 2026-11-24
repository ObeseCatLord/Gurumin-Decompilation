// Start options of an effect (EN build; JP: EffectOptions in decomp/include/effect_manager.h). A caller of
// EffectManager::Start may pass one; the constructor fills in the defaults. This is the one declaration of
// EffectOptions in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_EFFECT_OPTIONS_H
#define GURUMIN_EN_EFFECT_OPTIONS_H

#include "vec4.h"

// Defaults of the constructor.
const int EFFECT_OPTIONS_DEFAULT_COLOUR = 0xa2c2a;   // EffectOptions::colour
const float EFFECT_OPTIONS_DEFAULT_SCALE = 1.0f;     // EffectOptions::scale

// Index of the setting that is 1 by default.
const int EFFECT_SETTING_DEFAULT_ON = 2;
// Number of byte-sized settings (an array, so locals of this type get a /GS cookie in JP code).
const int EFFECT_SETTING_COUNT = 8;

// Options a caller may pass to EffectManager::Start (0x44 bytes, copied into the effect's record).
struct EffectOptions
{
    int m_0000;                            // 0x00: not recovered
    int colour;                            // 0x04: EFFECT_OPTIONS_DEFAULT_COLOUR by default
    float scale;                           // 0x08: EFFECT_OPTIONS_DEFAULT_SCALE by default
    unsigned char list;                    // 0x0c: effect list when the definition leaves it open
    unsigned char flags;                   // 0x0d: bit 0 = follows the matrix it was attached to, bit 1 = follows its owner
    char m_000e;                           // 0x0e: not recovered
    char pad_000f;                         // 0x0f
    int m_0010;                            // 0x10: roll angle of some quad kinds (read as a float by the record)
    int m_0014;                            // 0x14: not recovered
    int m_0018;                            // 0x18: not recovered
    int m_001c;                            // 0x1c: not recovered
    int m_0020;                            // 0x20: not recovered
    char settings[EFFECT_SETTING_COUNT];   // 0x24: byte-sized settings (settings[2] = 1 by default)
    int owner;                             // 0x2c: owner (character index + 100), 0 = none
    Vec4 *steering;                        // 0x30: where a hook writes a direction
    const Vec4 *boxMin;                    // 0x34: first end of the box a hook tests
    const Vec4 *boxMax;                    // 0x38: second end of the box
    int m_003c;                            // 0x3c: not recovered
    int m_0040;                            // 0x40: not recovered

    EffectOptions() throw();               // EN 0x709ca0 (JP 0x101668b0): fills in the defaults
};

#endif
