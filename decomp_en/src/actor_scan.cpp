// Scans of the character slots (EN build). Ported from the matched JP source decomp/src/actor_scan.cpp.
#include "char.h"

// Enemy_FindReusableSlot: returns the first enemy slot that has a model and is either out of the world or finished
// (health <= 0 while its phase is 2), or -1. Called when a new enemy needs a slot.
extern "C" int Enemy_FindReusableSlot(void)
{
    for (int i = ENEMY_SLOT_FIRST; i < ENEMY_SLOT_END; i++)
    {
        if (g_charKind[i] == CHAR_KIND_ENEMY_SLOT && i >= ENEMY_SLOT_FIRST)
        {
            if ((g_characterModels[i].matrix.m[MATRIX_TRANSLATION_X] < CHAR_OFFWORLD_X ||
                 (g_charRecords[i].stats[STAT_HEALTH] <= 0 && g_charPhase[i] == CHAR_PHASE_FINISHED)) &&
                g_characterModels[i].root != 0)
                return i;
        }
    }
    return -1;
}
