// Proximity checks between characters (the player is character 0).

#include <stdarg.h>
#include "world.h"

#include "vec4.h"

extern "C" int FUN_10043cc0(int index);
extern "C" char DAT_1021f3c0[0x80];   // slot kind (3 = enemy)
extern "C" int DAT_10c66710;          // non-zero: wake-up radius forced to 300

// For an idle character (state 0): plays its motion 2 and, when the player is within 10 * m_0004[1] units,
// triggers FUN_10043cc0 (the character notices the player). Returns 0.
extern "C" int FUN_10045b80(int index)
{
    float distance2 = (DAT_10db6ed8[0].matrix.GetTranslation() - DAT_10db6ed8[index].matrix.GetTranslation())
                          .LengthSquared3();
    if (DAT_1020b430[index].m_0000 == 0)
    {
        DAT_10db6ed8[index].model.M_1017b0e0(2, 0, 5, 1.0f);
        if (distance2 < (float)(DAT_1020b430[index].m_0004[1] * DAT_1020b430[index].m_0004[1] * 100))
            FUN_10043cc0(index);
    }
    return 0;
}

// Wakes the sleeping enemies (kind 0x62, state 0) within `radius` of character `index` by setting their state to 1.
// With `count` != 0 only enemies whose group (m_0004[0]) is among the `count` group numbers that follow are woken.
// Returns 0. Not byte-identical yet: the two vectors get each other's stack slots (see open.tsv).
extern "C" int FUN_1001b670(int index, float radius, int count, ...)
{
    Vec4 other;
    Vec4 position;
    if (DAT_10c66710 != 0)
        radius = 300.0f;
    radius = radius * radius;  // compared with squared distances from here on
    position.Copy(DAT_10db6ed8[index].matrix.GetTranslation());
    va_list groups;
    va_start(groups, count);
    for (int i = 0x3a; i < 0x5c; i++)
    {
        if (DAT_10daab84[i].m_0000 == 0x62 && DAT_1021f3c0[i] == 3 && DAT_10db6ed8[i].m_22fc != 0 &&
            DAT_10db6ed8[i].matrix._41 > -20000.0f && DAT_1020b430[i].m_0000 == 0)
        {
            if (count != 0)
            {
                int g;
                for (g = 0; g < count; g++)
                {
                    if (DAT_1020b430[i].m_0004[0] == ((int *)groups)[g])
                        break;
                }
                if (g == count)
                    continue;
            }
            other.Copy(DAT_10db6ed8[i].matrix.GetTranslation());
            if ((other - position).LengthSquared3() < radius)
                DAT_1020b430[i].m_0000 = 1;
        }
    }
    va_end(groups);
    return 0;
}
