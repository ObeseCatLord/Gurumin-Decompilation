// Point-in-box tests of Vec4 added by the English build (EN-only: no JP twin found).
#include "vec4.h"

// Width-to-height ratio of the 4:3 screen the box x ranges were authored for.
const float AUTHORED_ASPECT = 0.75f;
// One half, used to find the centre and half width of the box.
const float HALF = 0.5f;

// 1 when the point lies inside the box, with the box's x range stretched around its centre by AUTHORED_ASPECT / aspect
// (the widescreen variant of InBox: aspect 0.75 tests the box itself). y and z use the box edges. Called by the
// camera/region code at EN 0x749b10.
int Vec4::InBoxAspect(const Box2 *box, float aspect)
{
    float left;
    float right;
    float stretch = AUTHORED_ASPECT / aspect;
    right = box->max.x;
    if (stretch == 1.0f)
    {
        left = box->min.x;
    }
    else
    {
        float halfWidth = (right - box->min.x) * HALF;
        float stretched = halfWidth * stretch;
        float centre = halfWidth + box->min.x;
        left = centre - stretched;
        right = stretched + centre;
    }
    if (left <= x && x <= right && box->min.y <= y && y <= box->max.y && box->min.z <= z && z <= box->max.z)
        return 1;
    return 0;
}

// 1 when each of x, y and z of this point is within `halfSize` of the same component of `centre` (edges included),
// i.e. the point lies in the axis-aligned cube of half width `halfSize` around `centre`. No direct caller.
int Vec4::InCubeAround(const Vec4 *centre, float halfSize)
{
    if (centre->x - halfSize <= x && x <= centre->x + halfSize && centre->y - halfSize <= y &&
        y <= centre->y + halfSize && centre->z - halfSize <= z && z <= centre->z + halfSize)
        return 1;
    return 0;
}
