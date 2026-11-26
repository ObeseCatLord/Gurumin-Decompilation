// Objects that hold references to other characters (EN build, EN-only: no JP twin found).
#include "char.h"

// Object_AddReference: finds the first object slot of kind OBJECT_KIND_REFERENCE_HOLDER that still has a free state value
// (zero), stores the character `character` there and saves `offset` as the node matrix of that value, with the four
// scale values in the w column. Reports the object slot in *objectSlot and the value index in *valueIndex. Returns 1,
// or 0 when no holder has a free value. Called by the script command that attaches a character to an object (EN 0x6883a0).
extern "C" int Object_AddReference(int character, const Matrix4 *offset, int *objectSlot, int *valueIndex, float scaleX,
                                   float scaleY, float scaleZ, float scaleW)
{
    for (int slot = OBJECT_SLOT_FIRST; slot < ENEMY_SLOT_END; slot++)
    {
        if (g_charRecords[slot].kind != OBJECT_KIND_REFERENCE_HOLDER)
            continue;
        for (int word = 1; word < OBJECT_REFERENCE_VALUE_COUNT + 1; word++)
        {
            if (g_charState[slot].words[word] == 0)
            {
                Matrix4 &saved = g_characterNodeMatrices[slot][word - 1];
                saved = *offset;
                saved.m[NODE_MATRIX_SCALE_X] = scaleX;
                saved.m[NODE_MATRIX_SCALE_Y] = scaleY;
                saved.m[NODE_MATRIX_SCALE_Z] = scaleZ;
                saved.m[NODE_MATRIX_SCALE_W] = scaleW;
                g_charState[slot].words[word] = character;
                *objectSlot = slot;
                *valueIndex = word - 1;
                return 1;
            }
        }
    }
    return 0;
}
