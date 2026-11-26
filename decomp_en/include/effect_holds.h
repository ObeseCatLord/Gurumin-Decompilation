// The effect hold table of the EN build (JP: decomp/include/effect_holds.h, DAT_1020dcf8): effects the map keeps
// requesting (fires, lava glow, flashes), each in its own slot until the hazard code starts the effect. This is the one
// declaration of this type and table in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_EFFECT_HOLDS_H
#define GURUMIN_EN_EFFECT_HOLDS_H

// Number of slots of the table.
const int EFFECT_HOLD_COUNT = 0x20;

struct EffectHold
{
    int effect;           // 0x00: effect id the slot waits to start (0 = free)
    int position[3];      // 0x04: where the effect is wanted (x, y, z)
    int m_0010;           // 0x10: not used by EN code yet
    int age;              // 0x14: frames the slot has waited; the hazard code starts the effect once it is high enough
    int value;            // 0x18: handed to the effect's options
    int m_001c;           // 0x1c: not used by EN code yet
};

extern "C" EffectHold g_effectHolds[EFFECT_HOLD_COUNT];   // EN 0x00920470 (JP DAT_1020dcf8): one slot per effect the map keeps requesting

#endif
