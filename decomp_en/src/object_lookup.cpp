// Looking up and ordering object slots (EN build, EN-only: no JP twin found).
#include "char.h"

// Object_FindByModelId: the first slot from OBJECT_SLOT_FIRST on whose kind letter is F, M or N and whose model has the
// id `objectId`; OBJECT_NOT_FOUND when there is none. Called by the script commands that name an object (EN 0x5caa80,
// 0x5e87d0).
extern "C" int Object_FindByModelId(int objectId)
{
    for (int slot = OBJECT_SLOT_FIRST; slot < CHAR_SLOT_COUNT; slot++)
    {
        char kind = g_charKind[slot];
        if ((kind == OBJECT_KIND_LETTER_F || kind == OBJECT_KIND_LETTER_M || kind == OBJECT_KIND_LETTER_N) &&
            g_characterModels[slot].objectId == objectId)
            return slot;
    }
    return OBJECT_NOT_FOUND;
}

// CharSlot_CompareByDepth: qsort comparator over slot numbers. A slot's depth is its model's sortBias plus the z of its
// position; returns -1 when `a` has the smaller depth, else 1 (smallest first). No direct caller.
extern "C" int CharSlot_CompareByDepth(const int *a, const int *b)
{
    float depthA = g_characterModels[*a].sortBias + g_characterModels[*a].matrix.m[MATRIX_TRANSLATION_Z];
    float depthB = g_characterModels[*b].sortBias + g_characterModels[*b].matrix.m[MATRIX_TRANSLATION_Z];
    return depthA < depthB ? -1 : 1;
}

// Objects_ClearReferencesInRange: for every object slot of kind OBJECT_KIND_REFERENCE_HOLDER, clears the slot numbers
// in its state values that lie between `first` and `last` (inclusive). Returns 0. Called when characters are removed
// (EN 0x6883a0).
extern "C" int Objects_ClearReferencesInRange(int first, int last)
{
    for (int slot = OBJECT_SLOT_FIRST; slot < ENEMY_SLOT_END; slot++)
    {
        if (g_charRecords[slot].kind == OBJECT_KIND_REFERENCE_HOLDER)
        {
            for (int i = 0; i < OBJECT_REFERENCE_VALUE_COUNT; i++)
            {
                int value = g_charState[slot].values[i];
                if (value >= first && value <= last)
                    g_charState[slot].values[i] = 0;
            }
        }
    }
    return 0;
}
