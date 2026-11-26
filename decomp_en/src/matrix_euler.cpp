// Rotation helpers built from the axis rotations (EN build, EN-only: no JP twin found).
#include <math.h>
#include "vec4.h"

// Rotation matrix of the Euler angles (x, y, z in radians) in `angles`: a turn about y, then about x, then about z in
// the engine's row-vector order (RotationY * RotationX * RotationZ). No direct caller.
Matrix4 EulerMatrix(const Vec4 *angles)
{
    return RotationY(angles->y) * RotationX(angles->x) * RotationZ(angles->z);
}

// Stores in `result` the x/y part of `source` turned by `angle` radians about the z axis, with the z and w of `source`
// copied over; returns `result`. Called by the camera code at EN 0x60cb10.
Vec4 *Vec4_RotatedAboutZ(Vec4 *result, const Vec4 *source, float angle)
{
    result->z = source->z;
    result->w = source->w;
    float cosine = (float)cos(angle);
    float sine = (float)sin(angle);
    float turnedX = cosine * source->x - sine * source->y;
    float turnedY = cosine * source->y + sine * source->x;
    result->y = turnedY;
    result->x = turnedX;
    return result;
}
