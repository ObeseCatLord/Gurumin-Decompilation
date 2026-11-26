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
