// The script effect slots (EN build): effects started by scripts (gimmick effects), 0x40 slots of 0xfc bytes.
// This is the one declaration of EffectSlot and its table in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_EFFECT_SLOTS_H
#define GURUMIN_EN_EFFECT_SLOTS_H

// Number of effect slots.
const int EFFECT_SLOT_COUNT = 0x40;
// Bytes of EffectSlot::name.
const int EFFECT_SLOT_NAME_SIZE = 0xc0;
// EffectSlot::type of the effects that hold a value.
const int EFFECT_SLOT_TYPE_VALUE = 10;
// EffectSlot::type of the model effects started by EffectSlot_StartModel.
const int EFFECT_SLOT_TYPE_MODEL = 14;
// Returned when no slot matches.
const int EFFECT_SLOT_NONE = -1;

#include "vec4.h"

struct EffectSlot
{
    int id;                     // 0x00: id of the effect (script id)
    int value;                  // 0x04: value of the effect
    int m_0008;                 // 0x08: not recovered
    int type;                   // 0x0c: kind of effect (EFFECT_SLOT_TYPE_VALUE)
    int modelValue;             // 0x10: value of a model effect (EffectSlot_StartModel's first argument)
    char m_0014[0x1c - 0x14];   // 0x14: not used by EN code yet
    char name[EFFECT_SLOT_NAME_SIZE];   // 0x1c: name of the effect
    Vec4 position;              // 0xdc: where the effect plays
    int m_00ec;                 // 0xec: not recovered (non-zero keeps the slot taken)
    int active;                 // 0xf0: non-zero while the effect runs
    char m_00f4[0xfc - 0xf4];   // 0xf4: not used by EN code yet
};

extern "C" EffectSlot g_effectSlots[EFFECT_SLOT_COUNT];   // EN 0x015b9688 (JP DAT_10860df0)

// Returns the first free effect slot, or -1. EN 0x5ee440 (JP FindFreeEffectSlot).
extern "C" int EffectSlots_FindFree(void);
// Starts the model effect `name` (type EFFECT_SLOT_TYPE_MODEL) with `id` and `value` at (x, y, z) in a free slot. Returns 1. EN 0x5c99e0 (JP 0x1003e650).
extern "C" int EffectSlot_StartModel(int value, int x, int y, int z, const char *name, int id);
// Returns the value of the active type-10 effect with `id`, or -1. EN 0x64db60.
extern "C" int EffectSlots_GetValue(int id);

#endif
