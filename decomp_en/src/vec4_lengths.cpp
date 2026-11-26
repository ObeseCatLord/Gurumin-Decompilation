// Length and distance helpers of Vec4 that take their operand by pointer or reference (EN build, EN-only: no JP twin).
#include <math.h>
#include "vec4.h"

// Distance between this point and `other` on the x/y plane; the z difference is counted as zero. No direct caller.
float Vec4::DistanceFlat(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    return (float)sqrt(dy * dy + dx * dx + 0.0f);
}

// Length of the x/y/z part of `v`. No direct caller.
extern "C" float Vec3_Length(const Vec4 *v)
{
    return (float)sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}
