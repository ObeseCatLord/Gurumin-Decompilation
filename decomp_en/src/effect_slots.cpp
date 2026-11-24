// Script effect slots (EN build). Ported from the matched JP source decomp/src/app_0011.cpp.
#include <string.h>
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

// Japanese text of the examine prompt as the data files spell it ("shiraberu"); the English game shows "Examine" (spelled out at its single use).
static const char EXAMINE_PROMPT_JP[] = "\x82\xb5\x82\xe7\x82\xd7\x82\xe9";

// EffectSlot_StartModel: starts a type-14 effect `name` with `id` and `value` at (x, y, z) in a free effect slot; the
// Japanese examine prompt is replaced by "Examine". Returns 1. The original does not check for a full table (slot -1
// would be written). Called by the effect scripts.
extern "C" int EffectSlot_StartModel(int value, int x, int y, int z, const char *name, int id)
{
    int slot = EffectSlots_FindFree();
    g_effectSlots[slot].modelValue = value;
    g_effectSlots[slot].id = id;
    g_effectSlots[slot].active = 1;
    g_effectSlots[slot].m_00ec = 0;
    g_effectSlots[slot].type = EFFECT_SLOT_TYPE_MODEL;
    if (strcmp(name, EXAMINE_PROMPT_JP) == 0)
        strcpy(g_effectSlots[slot].name, "Examine");   // the English text of the prompt
    else
        strcpy(g_effectSlots[slot].name, name);
    g_effectSlots[slot].position.Set((float)x, (float)y, (float)z, 0);
    return 1;
}
