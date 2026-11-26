// Proximity checks between characters (the player is character 0).

#include <stdarg.h>
#include "world.h"

#include "vec4.h"
#include "anim_model.h"
#include "char_state.h"
#include "char_slot.h"

extern "C" int FUN_10043cc0(int index);
#include "char_tables.h"
#include "char_vars.h"

// For an idle character (state 0): plays its motion 2 and, when the player is within 10 * m_0004[1] units,
// triggers FUN_10043cc0 (the character notices the player). Returns 0.
extern "C" int FUN_10045b80(int index)
{
    float distance2 = (DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[index].matrix.GetTranslation())
                          .LengthSquared3();
    if (DAT_1020b430[index].state == 0)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        if (distance2 < (float)(DAT_1020b430[index].values[1] * DAT_1020b430[index].values[1] * 100))
            FUN_10043cc0(index);
    }
    return 0;
}

// 0x1001b670. Wakes the sleeping enemies (kind 0x62, slot kind 3, state 0) within `radius` of character `index` by
// setting their state to 1 (the radius is forced to 300 while DAT_10c66710 is set). With `count` != 0 only enemies
// whose group (m_0004[0]) is among the `count` group numbers that follow are woken. The radius is squared in place
// (its parameter slot holds the squared radius afterwards). Returns 0.
extern "C" int FUN_1001b670(int index, float radius, int count, ...)
{
    Vec4 position;
    Vec4 other;
    if (DAT_10c66710 != 0)
        radius = 300.0f;
    radius *= radius;  // compared with squared distances from here on
    position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    va_list groups;
    va_start(groups, count);
    const int *groupList = (const int *)groups;
    for (int i = 0x3a; i < 0x5c; i++)
    {
        if (DAT_10daaae8[i].kind == 0x62 && DAT_1021f3c0[i] == 3 && DAT_10db6ee8[i].root != 0 &&
            DAT_10db6ee8[i].matrix._41 > -20000.0f && DAT_1020b430[i].state == 0)
        {
            if (count != 0)
            {
                int g = 0;
                if (0 < count)
                {
                    do
                    {
                        if (DAT_1020b430[i].values[0] == groupList[g])
                            break;
                        g++;
                    } while (g < count);
                }
                if (g == count)
                    continue;
            }
            other.Copy(DAT_10db6ee8[i].matrix.GetTranslation());
            if ((position - other).LengthSquared3() < radius)
                DAT_1020b430[i].state = 1;
        }
    }
    va_end(groups);
    return 0;
}
