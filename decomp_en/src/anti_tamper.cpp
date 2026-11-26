// Anti-tamper guard copies (EN build). JP twin: anti_tamper.cpp.
#include <stdlib.h>

#include "anti_tamper.h"
#include "char.h"
#include "frame_step.h"
#include "game_vars.h"
#include "switches.h"

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

// AntiTamper_StoreArrays: refreshes the guard copies of the stock of every item (slots 10..89) and of the 15 extra
// values (slots 100..114). Returns 0. Called after every inventory change.
extern "C" int AntiTamper_StoreArrays(void)
{
    for (int i = 0; i < GUARD_ITEM_COUNT; i++)
        g_guardCopies[GUARD_ITEM_FIRST_SLOT + i] = ~g_itemStock[i] + GUARD_STOCK_BASE + i * GUARD_SLOT_STEP;
    for (int i = 0; i < GUARD_EXTRA_COUNT; i++)
        g_guardCopies[GUARD_EXTRA_FIRST_SLOT + i] = ~g_guardedExtras[i] + GUARD_EXTRA_BASE + i * GUARD_EXTRA_STEP;
    return 0;
}

// The penalty of a failed check: writes a random value into a random entry of the noise table.
inline void TamperPenalty()
{
    int noise = rand();
    g_tamperNoise[rand() % TAMPER_NOISE_COUNT] = noise;
}

// Frames between the checks of the item stock copies; they run on frame multiples of it and the extra values
// copies half a period later (14 of 30).
const unsigned int TAMPER_ARRAY_PERIOD = 30;
const unsigned int TAMPER_EXTRA_PHASE = 14;

// AntiTamper_Check: counts frames, penalises an active debugger flag, compares every guard copy with its value (the
// health and maximum health mismatches are also counted in g_healthClampCountA) and every 30 frames checks the array
// copies. Returns 0. Called by the frame effect update (EN 0x60cb10).
extern "C" int AntiTamper_Check(void)
{
    g_tamperCheckDepth++;
    if (g_debuggerDetected != 0)
        TamperPenalty();
    if (g_guardCopies[0] != GUARD_VALUE(0, g_charRecords[0].stats[STAT_PICKUPS]))
        TamperPenalty();
    if (g_guardCopies[1] != GUARD_VALUE(1, g_charRecords[0].stats[STAT_GUARDED_B]))
        TamperPenalty();
    if (g_guardCopies[2] != GUARD_VALUE(2, g_charRecords[0].stats[STAT_HEALTH]))
    {
        TamperPenalty();
        g_healthClampCountA++;
    }
    if (g_guardCopies[3] != GUARD_VALUE(3, g_charRecords[0].stats[STAT_MAX_HEALTH]))
    {
        TamperPenalty();
        g_healthClampCountA++;
    }
    if (g_guardCopies[4] != GUARD_VALUE(4, g_charRecords[0].money))
        TamperPenalty();
    if (g_guardCopies[5] != GUARD_VALUE(5, g_guardedValue5))
        TamperPenalty();
    if (g_guardCopies[6] != GUARD_VALUE(6, g_guardedValue6))
        TamperPenalty();
    if (g_guardCopies[7] != GUARD_VALUE(7, g_guardedValue7))
        TamperPenalty();
    if (g_guardCopies[8] != GUARD_VALUE(8, g_guardedValue8))
        TamperPenalty();
    if (g_guardCopies[9] != GUARD_VALUE(9, g_guardedValue9))
        TamperPenalty();
    if (g_gameFrames % TAMPER_ARRAY_PERIOD == 0)
    {
        for (int i = 0; i < GUARD_ITEM_COUNT; i++)
        {
            if (g_guardCopies[GUARD_ITEM_FIRST_SLOT + i] != ~g_itemStock[i] + GUARD_STOCK_BASE + i * GUARD_SLOT_STEP)
                TamperPenalty();
        }
    }
    if (g_gameFrames % TAMPER_ARRAY_PERIOD == TAMPER_EXTRA_PHASE)
    {
        for (int i = 0; i < GUARD_EXTRA_COUNT; i++)
        {
            if (g_guardCopies[GUARD_EXTRA_FIRST_SLOT + i] != ~g_guardedExtras[i] + GUARD_EXTRA_BASE + i * GUARD_EXTRA_STEP)
                TamperPenalty();
        }
    }
    g_tamperCheckDepth--;
    return 0;
}
