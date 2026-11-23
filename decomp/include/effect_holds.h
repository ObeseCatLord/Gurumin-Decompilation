// The effect hold table DAT_1020dcf8: effects the map keeps requesting (fires, lava glow, flashes), each in its own
// slot until the hazard code starts the effect. This is the one declaration of this type: include this header.
#ifndef GURUMIN_EFFECT_HOLDS_H
#define GURUMIN_EFFECT_HOLDS_H

// Number of slots of the table.
#define EFFECT_HOLD_COUNT 0x20

struct EffectHold
{
    int effect;           // 0x00: effect id the slot waits to start (0 = free)
    int position[3];      // 0x04: where the effect is wanted (x, y, z)
    int m_0010;           // 0x10
    int age;              // 0x14: frames the slot has waited; the hazard code starts the effect once it is high enough
    int value;            // 0x18: handed to the effect's options
    int m_001c;           // 0x1c
};

#endif
