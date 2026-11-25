// Stacking carried objects on top of each other (EN build). Ported from decomp/src/carry_stack.cpp.
#include "carry_stack.h"
#include "char.h"
#include "node.h"

// Objects whose top lies this far above the carried object's position (low and high bound) can be stacked on; a tall
// carried object (the model has the node "@O_019b") uses the second pair.
const float STACK_HEIGHT_LOW = 135.0f;
const float STACK_HEIGHT_HIGH = 165.0f;
const float STACK_TALL_HEIGHT_LOW = 285.0f;
const float STACK_TALL_HEIGHT_HIGH = 315.0f;
// Half size of the square (x and y) the object below must be within.
const float STACK_HALF_SIZE = 25.0f;
// Name of the node a tall object has.
const char STACK_TALL_NODE[] = "@O_019b";
// Object slots can be stacked on when their kind is one of these.
const char STACK_KIND_A = 5;
const char STACK_KIND_B = 0x5f;
// A slot moved below this x has been removed from the world.
const float STACK_REMOVED_X = -20000.0f;
// z the search starts from (the highest object below that is found wins).
const float STACK_SEARCH_START_Z = 999999.0f;

// Carry_StackOnObject: finds the object of kind 5 or 0x5f (slots 0x5c..0x7f) whose top lies 135..165 units above the
// carried object (285..315 for a tall one) within 25 units in x and y, keeping the highest, and stores its position
// with z made relative. Once found, the carried object follows it: its model is placed at the remembered x/y on top.
// Slots moved below x -20000 (removed) reset the search. Returns 0. Called every frame by the carry code.
extern "C" int Carry_StackOnObject(void)
{
    if (g_carriedSlot >= OBJECT_SLOT_FIRST)
    {
        if (g_carriedSlotLast != g_carriedSlot ||
            (g_stackTargetSlot >= OBJECT_SLOT_FIRST &&
             g_characterModels[g_stackTargetSlot].matrix.m[MATRIX_TRANSLATION_X] < STACK_REMOVED_X))
        {
            g_stackTargetSlot = 0;
            g_carriedSlotLast = g_carriedSlot;
        }
        Vec4 position = g_characterModels[g_carriedSlot].matrix.GetTranslation();
        Vec4 other;
        if (g_stackTargetSlot < OBJECT_SLOT_FIRST)
        {
            int tall = 0;
            if (g_characterModels[g_carriedSlot].root->Find(STACK_TALL_NODE, 0) != 0)
                tall = 1;
            g_stackPosition.Fill(0.0f);
            g_stackPosition.z = STACK_SEARCH_START_Z;
            for (int i = OBJECT_SLOT_FIRST; i < CHAR_SLOT_COUNT; i++)
            {
                if (g_stackTargetSlot == g_carriedSlot || (g_charKind[i] != STACK_KIND_A && g_charKind[i] != STACK_KIND_B))
                    continue;
                other.Copy(g_characterModels[i].matrix.GetTranslation());
                if (g_stackTargetSlot != 0 && (g_stackTargetSlot < OBJECT_SLOT_FIRST || !(g_stackPosition.z > other.z)))
                    continue;
                if (tall != 0)
                {
                    if (!(position.z + STACK_TALL_HEIGHT_LOW < other.z) || !(position.z + STACK_TALL_HEIGHT_HIGH > other.z))
                        continue;
                }
                else
                {
                    if (!(position.z + STACK_HEIGHT_LOW < other.z) || !(position.z + STACK_HEIGHT_HIGH > other.z))
                        continue;
                }
                if (position.x - STACK_HALF_SIZE < other.x && position.x + STACK_HALF_SIZE > other.x &&
                    position.y - STACK_HALF_SIZE < other.y && position.y + STACK_HALF_SIZE > other.y)
                {
                    g_stackTargetSlot = i;
                    g_stackPosition.Copy(other);
                }
            }
            g_stackPosition.z -= position.z;
            return 0;
        }
        if (g_characterModels[g_carriedSlot].matrix.m[MATRIX_TRANSLATION_X] > STACK_REMOVED_X)
        {
            g_stackPosition.x = position.x;
            g_stackPosition.y = position.y;
            g_characterModels[g_stackTargetSlot].SetPosition(position.x, position.y, g_stackPosition.z + position.z);
            return 0;
        }
    }
    g_stackTargetSlot = 0;
    return 0;
}
