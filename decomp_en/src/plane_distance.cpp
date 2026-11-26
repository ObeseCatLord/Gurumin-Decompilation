// Distance helpers of the EN build that work from a plane or a vector length (EN-only: no JP twin found).
#include <math.h>
#include "plane.h"

// Length of a vector beyond which a sound or light starts to fade; closer than this it is at full strength.
const float FALLOFF_FULL_RANGE = 100.0f;
// Extra distance over which the strength falls from 1 to 0 once FALLOFF_FULL_RANGE is passed.
const float FALLOFF_FADE_RANGE = 500.0f;

// Unsigned distance of `point` from the plane: |a x + b y + c z + d| / |(a, b, c)|. Called by the collision tests at EN
// 0x74a530, 0x74bc60, 0x74c040 and 0x74c990.
float Plane::Distance(const Vec4 *point)
{
    float signedOffset = point->y * b + point->x * a + point->z * c + d;
    float normalLength = (float)sqrt(a * a + b * b + c * c);
    return (float)fabs(signedOffset / normalLength);
}

// Strength from 1 down to 0 for a vector `v` (a displacement): 1 up to FALLOFF_FULL_RANGE long, then falling linearly
// to 0 over FALLOFF_FADE_RANGE. No direct caller.
float Vec3_FalloffStrength(const Vec4 *v)
{
    float excess = (float)sqrt(v->x * v->x + v->y * v->y + v->z * v->z) - FALLOFF_FULL_RANGE;
    if (excess < 0.0f)
        excess = 0.0f;
    float remaining = FALLOFF_FADE_RANGE - excess;
    return (remaining < 0.0f ? 0.0f : remaining) / FALLOFF_FADE_RANGE;
}
