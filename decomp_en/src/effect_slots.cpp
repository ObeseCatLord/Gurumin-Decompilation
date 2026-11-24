// Script effect slots (EN build). Ported from the matched JP source decomp/src/app_0011.cpp.
#include "effect_slots.h"

// EffectSlots_FindFree: returns the first free effect slot (not taken and not active), or -1. Called by the effect
// scripts.
extern "C" int EffectSlots_FindFree(void)
{
    for (int i = 0; i < EFFECT_SLOT_COUNT; i++)
    {
        if (g_effectSlots[i].m_00ec == 0 && g_effectSlots[i].active == 0)
            return i;
    }
    return EFFECT_SLOT_NONE;
}

// EffectSlots_GetValue: returns the value of the active type-10 effect with `id`, or -1. Called by the effect scripts.
extern "C" int EffectSlots_GetValue(int id)
{
    for (int i = 0; i < EFFECT_SLOT_COUNT; i++)
    {
        if (g_effectSlots[i].active != 0 && g_effectSlots[i].type == EFFECT_SLOT_TYPE_VALUE && g_effectSlots[i].id == id)
            return g_effectSlots[i].value;
    }
    return EFFECT_SLOT_NONE;
}
