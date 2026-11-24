// The script effect slots (EN build): effects started by scripts (gimmick effects), 0x40 slots of 0xfc bytes.
// This is the one declaration of EffectSlot and its table in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_EFFECT_SLOTS_H
#define GURUMIN_EN_EFFECT_SLOTS_H

// Number of effect slots.
const int EFFECT_SLOT_COUNT = 0x40;
// EffectSlot::type of the effects that hold a value.
const int EFFECT_SLOT_TYPE_VALUE = 10;
// Returned when no slot matches.
const int EFFECT_SLOT_NONE = -1;

struct EffectSlot
{
    int id;                     // 0x00: id of the effect (script id)
    int value;                  // 0x04: value of the effect
    int m_0008;                 // 0x08: not recovered
    int type;                   // 0x0c: kind of effect (EFFECT_SLOT_TYPE_VALUE)
    char m_0010[0xec - 0x10];   // 0x10: name, position (not used by EN code yet)
    int m_00ec;                 // 0xec: not recovered (non-zero keeps the slot taken)
    int active;                 // 0xf0: non-zero while the effect runs
    char m_00f4[0xfc - 0xf4];   // 0xf4: not used by EN code yet
};

extern "C" EffectSlot g_effectSlots[EFFECT_SLOT_COUNT];   // EN 0x015b9688 (JP DAT_10860df0)

// Returns the first free effect slot, or -1. EN 0x5ee440 (JP FindFreeEffectSlot).
extern "C" int EffectSlots_FindFree(void);
// Returns the value of the active type-10 effect with `id`, or -1. EN 0x64db60.
extern "C" int EffectSlots_GetValue(int id);

#endif
