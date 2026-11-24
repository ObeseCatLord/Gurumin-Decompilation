// Effect definition table allocation (JP twin: effect_defs.cpp).

#include <string.h>

#include "effect_manager.h"

// Lives a new definition has by default, in frames.
const int NEW_DEFINITION_LIFETIME = 50;

// Takes the first free effect definition, names it "not set", gives it its defaults and clears the recent-effect
// cache and the effect list heads (-1). Called when a definition is added to the table. Returns its index, or -1
// when all are in use.
extern "C" int EffectDefs_Allocate(void)
{
    int i;
    for (i = 0; i < EFFECT_DEFINITION_COUNT; i++)
    {
        if (g_effectDefs[i].id == EFFECT_NONE)
            break;
    }
    if (i == EFFECT_DEFINITION_COUNT)
        return -1;
    g_effectDefs[i].id = EFFECT_NONE;
    strcpy(g_effectDefs[i].name, "\x96\xa2\x90\xdd\x92\xe8");   // cp932 for "mi-settei" (not set)
    g_effectDefs[i].m_0154 = 0;
    g_effectDefs[i].m_0164 = NEW_DEFINITION_LIFETIME;
    g_recentEffectNext = 0;
    int j;
    for (j = 0; j < EFFECT_RECENT_COUNT; j++)
        g_recentEffectIds[j] = EFFECT_NONE;
    // The two five-entry effect list heads.
    g_effectListFirst[0] = EFFECT_NONE;
    g_effectListFirst[1] = EFFECT_NONE;
    g_effectListFirst[2] = EFFECT_NONE;
    g_effectListFirst[3] = EFFECT_NONE;
    g_effectListFirst[4] = EFFECT_NONE;
    g_effectListLast[0] = EFFECT_NONE;
    g_effectListLast[1] = EFFECT_NONE;
    g_effectListLast[2] = EFFECT_NONE;
    g_effectListLast[3] = EFFECT_NONE;
    g_effectListLast[4] = EFFECT_NONE;
    return i;
}
