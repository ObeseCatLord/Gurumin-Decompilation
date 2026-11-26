// Angle difference (EN build).
#include "vec4.h"

#include <math.h>

// Wrap bounds used by AngleDifference: pi to four digits, as in the original, and twice pi to four digits.
const float ANGLE_WRAP_HALF = 3.1415f;
const float ANGLE_WRAP_FULL = 6.283f;

// AngleDifference: the signed difference a - b of two angles in radians, taken the short way round. Both angles are first
// pulled into -3.1415 .. 3.1415 by repeated steps of 3.1415, then the difference is tried with 0, +6.283 and -6.283 added
// to a and the one of smallest magnitude wins. Called by the world update (0x60cb10).
extern "C" float AngleDifference(float a, float b)
{
    while (a > ANGLE_WRAP_HALF)
        a = a - ANGLE_WRAP_HALF;
    while (a < -ANGLE_WRAP_HALF)
        a = a + ANGLE_WRAP_HALF;
    while (b > ANGLE_WRAP_HALF)
        b = b - ANGLE_WRAP_HALF;
    while (b < -ANGLE_WRAP_HALF)
        b = b + ANGLE_WRAP_HALF;
    float direct = a - b;
    float forward = (a + ANGLE_WRAP_FULL) - b;
    float backward = (a - ANGLE_WRAP_FULL) - b;
    float best = direct;
    if (fabsf(direct) > fabsf(forward))
        best = forward;
    if (fabsf(best) > fabsf(backward))
        best = backward;
    return best;
}
