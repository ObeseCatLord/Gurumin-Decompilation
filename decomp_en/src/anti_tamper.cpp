// Anti-tamper guard copies (EN build). JP twin: anti_tamper.cpp.
#include "anti_tamper.h"
#include "char.h"

// The guard copy of a value for `slot`: the inverted value plus the slot's step.
#define GUARD_VALUE(slot, value) (~(value) + (slot) * GUARD_SLOT_STEP)

// AntiTamper_StoreValues: refreshes the guard copies of the ten single values (slots 0..9): pickups, the second
// guarded stat, health, maximum health, money of slot 0 and five game values. Returns 0. Called whenever one of them
// changes.
extern "C" int AntiTamper_StoreValues(void)
{
    g_guardCopies[0] = GUARD_VALUE(0, g_charRecords[0].stats[STAT_PICKUPS]);
    g_guardCopies[1] = GUARD_VALUE(1, g_charRecords[0].stats[STAT_GUARDED_B]);
    g_guardCopies[2] = GUARD_VALUE(2, g_charRecords[0].stats[STAT_HEALTH]);
    g_guardCopies[3] = GUARD_VALUE(3, g_charRecords[0].stats[STAT_MAX_HEALTH]);
    g_guardCopies[4] = GUARD_VALUE(4, g_charRecords[0].money);
    g_guardCopies[5] = GUARD_VALUE(5, g_guardedValue5);
    g_guardCopies[6] = GUARD_VALUE(6, g_guardedValue6);
    g_guardCopies[7] = GUARD_VALUE(7, g_guardedValue7);
    g_guardCopies[8] = GUARD_VALUE(8, g_guardedValue8);
    g_guardCopies[9] = GUARD_VALUE(9, g_guardedValue9);
    return 0;
}
