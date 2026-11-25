// Waking enemies near a character (EN build).
#include <stdarg.h>
#include "char.h"
#include "char_move.h"

// Wake radius used while g_wakeRadiusShort is set.
const float WAKE_RADIUS_SHORT = 300.0f;
// An enemy this low is out of the world (below CHAR_OFFWORLD_X is checked with a looser value).
const float WAKE_MIN_X = -20000.0f;

// Makes every idle waiting enemy (record kind CHAR_KIND_PACKED, slot kind CHAR_KIND_ENEMY_SLOT, model loaded, still in
// the world and not yet aware of the player) that lies within `radius` of character `index` aware of the player. The
// radius is 300 while g_wakeRadiusShort is set. When `count` is not 0 the extra arguments list the behaviour numbers
// an enemy must have to be woken. Called by the NPC and enemy behaviours. Returns 0. EN 0x6ac0e0 (JP 0x1001b670).
extern "C" int Enemies_WakeNearby(int index, float radius, int count, ...)
{
    Vec4 origin;
    Vec4 position;
    if (g_wakeRadiusShort != 0)
        radius = WAKE_RADIUS_SHORT;
    float radiusSquared = radius * radius;
    origin.Copy(g_characterModels[index].matrix.GetTranslation());
    va_list behaviours;
    va_start(behaviours, count);
    const int *behaviourList = (const int *)behaviours;   // the argument list is read as an array
    for (int slot = ENEMY_SLOT_FIRST; slot < ENEMY_SLOT_END; slot++)
    {
        if (g_charRecords[slot].kind == CHAR_KIND_PACKED && g_charKind[slot] == CHAR_KIND_ENEMY_SLOT &&
            g_characterModels[slot].root != 0 && g_characterModels[slot].matrix.m[MATRIX_TRANSLATION_X] > WAKE_MIN_X &&
            g_charState[slot].sawPlayer == 0)
        {
            if (count != 0)
            {
                int i = 0;
                while (i < count && g_charState[slot].behaviour != behaviourList[i])
                    i++;
                if (i == count)
                    continue;
            }
            position.Copy(g_characterModels[slot].matrix.GetTranslation());
            if ((origin - position).LengthSquared3() < radiusSquared)
                g_charState[slot].sawPlayer = 1;
        }
    }
    return 0;
}
