// Whether a party character stands in the way of a move: used before a character steps so that it does not walk
// into the player or the companions.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

// Absolute value of a float (an inline helper: its argument is rounded to float first).
inline float AbsFloat(float value)
{
    return (float)fabs(value);
}

#include "char_vars.h"

// 0x10034cc0 (Party_BlocksMove). Returns 1 when one of the five party characters other than `self` would be within
// `range` of character `self` after it moves by `step` (on the ground plane) and either the move brings `self`
// closer to it while heading at it (within 0.25 rad) or it would be closer than 0.4 * `range`; else 0.
extern "C" int FUN_10034cc0(int self, const Vec4 &step, int range)
{
    Vec4 ahead = DAT_10db6ee8[self].matrix.GetTranslation() + step;
    Vec4 other;
    for (int i = 0; i < 5; i++)
    {
        if (DAT_101d67f0[i] == self)
            continue;
        other.Copy(DAT_10db6ee8[DAT_101d67f0[i]].matrix.GetTranslation());
        float distance = (other - ahead).Length2D();
        float gain = distance - (other - DAT_10db6ee8[self].matrix.GetTranslation()).Length2D();
        float heading = atan2f(step.x, step.y);
        float toOther = atan2f(other.x - ahead.x, other.y - ahead.y);
        if (distance < (float)range)
        {
            if (gain > 0.0f && AbsFloat(heading - toOther) < 0.25f)
                return 1;
            if (distance < (float)range * 0.4f)
                return 1;
        }
    }
    return 0;
}
