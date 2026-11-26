// Small Vec4 helpers of the EN build without a JP twin: integer fill, squared length and the z-scaled length.
#include "vec4.h"

// Sets all four components to `value` converted to float. The integer is read again for every component (the compiler
// cannot prove the vector does not overlap it).
void Vec4::FillFromInt(const int &value)
{
    x = (float)value;
    y = (float)value;
    z = (float)value;
    w = (float)value;
}

// Squared length of the x/y/z part of `v`.
float Vec3_LengthSquared(const Vec4 &v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

// Length of the x/y/z part of a copy of `*v` whose z was multiplied by `zScale` (flattens or stretches the height).
// No direct caller.
extern "C" float Vec3_LengthZScaled(const Vec4 *v, float zScale)
{
    Vec4 scaled = *v;
    scaled.z *= zScale;
    return scaled.Length();
}
