// Overlap test of two axis-aligned boxes, each given in its own space plus an offset (used by the scene node code
// at 0x10193140).

#include <math.h>

#include "vec4.h"

#include "box.h"

// 0x10191fd0 (Box_Overlap). Moves box `first` by `firstOffset` and box `second` by `secondOffset` and returns 1 when
// they overlap on all three axes (touching faces do not count), else 0. On overlap `centreDelta`, when given,
// receives the centre of the first box minus the centre of the second, and `negatedDelta`, when given, its negation
// (it reads `centreDelta`, so both must be given for that).
extern "C" int __stdcall FUN_10191fd0(const Box *first, const Vec4 *firstOffset, const Box *second,
                                      const Vec4 *secondOffset, Vec4 *centreDelta, Vec4 *negatedDelta)
{
    Vec4 a[2];
    Vec4 b[2];
    a[0].x = first->min.x + firstOffset->x;
    a[0].y = first->min.y + firstOffset->y;
    a[0].z = first->min.z + firstOffset->z;
    a[1].x = first->max.x + firstOffset->x;
    a[1].y = first->max.y + firstOffset->y;
    a[1].z = first->max.z + firstOffset->z;
    b[0].x = second->min.x + secondOffset->x;
    b[0].y = second->min.y + secondOffset->y;
    b[0].z = second->min.z + secondOffset->z;
    b[1].x = second->max.x + secondOffset->x;
    b[1].y = second->max.y + secondOffset->y;
    b[1].z = second->max.z + secondOffset->z;
    if (a[1].x <= b[0].x)
        return 0;
    if (a[0].x >= b[1].x)
        return 0;
    if (a[1].y <= b[0].y)
        return 0;
    if (a[0].y >= b[1].y)
        return 0;
    if (a[1].z <= b[0].z)
        return 0;
    if (a[0].z >= b[1].z)
        return 0;
    if (centreDelta != 0)
    {
        Vec4 centreA;
        Vec4 centreB;
        centreA.Copy((a[0] + a[1]) * 0.5f);
        centreB.Copy((b[0] + b[1]) * 0.5f);
        centreDelta->Copy(centreA - centreB);
    }
    if (negatedDelta != 0)
    {
        Vec4 zero;
        zero.Fill(0.0f);
        negatedDelta->Copy(zero - *centreDelta);
    }
    return 1;
}

// 0x10192200 (BoxCylinder_Overlap). Box `first` moved by `firstOffset` against the upright cylinder inscribed in box
// `second` moved by `secondOffset` (radius: half the x/y diagonal divided by 1.414, as in Cylinder_Overlap). In
// x/y they overlap when the cylinder's centre lies inside the box's rectangle or closer than the radius to one of
// its four edges; their height ranges must overlap as well. On overlap `delta`, when given, receives the box centre
// minus the cylinder centre with z = 0, and `negatedDelta`, when given, the cylinder centre minus the box centre.
// Returns 1 on overlap, else 0. Called by the scene node code at 0x10193140.
extern "C" int __stdcall FUN_10192200(const Box *first, const Vec4 *firstOffset, const Box *second,
                                      const Vec4 *secondOffset, Vec4 *delta, Vec4 *negatedDelta)
{
    Vec4 box[2];
    box[0].x = first->min.x + firstOffset->x;
    box[0].y = first->min.y + firstOffset->y;
    box[0].z = first->min.z + firstOffset->z;
    box[1].x = first->max.x + firstOffset->x;
    box[1].y = first->max.y + firstOffset->y;
    box[1].z = first->max.z + firstOffset->z;
    float radius;
    {
        float dy = second->max.y - second->min.y;
        float dx = second->max.x - second->min.x;
        radius = (float)sqrt(dx * dx + dy * dy) * 0.5f / 1.414f;
    }
    Vec4 cylinder[2];
    cylinder[0].x = secondOffset->x - radius;
    cylinder[0].y = secondOffset->y - radius;
    cylinder[0].z = second->min.z + secondOffset->z;
    cylinder[1].x = radius + secondOffset->x;
    cylinder[1].y = radius + secondOffset->y;
    cylinder[1].z = second->max.z + secondOffset->z;
    float radiusSquared = radius * radius;
    Vec4 centre;
    centre.x = secondOffset->x;
    centre.y = secondOffset->y;
    centre.z = secondOffset->z;

    // Outside the rectangle: the centre must be within the radius of one of the four edges.
    if ((box[0].x >= centre.x || box[1].x <= centre.x || box[0].y >= centre.y || box[1].y <= centre.y) &&
        centre.DistanceSquaredToSegment2D(box[0].x, box[0].y, box[1].x, box[0].y) >= radiusSquared &&
        centre.DistanceSquaredToSegment2D(box[1].x, box[0].y, box[1].x, box[1].y) >= radiusSquared &&
        centre.DistanceSquaredToSegment2D(box[0].x, box[1].y, box[1].x, box[1].y) >= radiusSquared &&
        centre.DistanceSquaredToSegment2D(box[0].x, box[0].y, box[0].x, box[1].y) >= radiusSquared)
        return 0;
    if (box[1].z <= cylinder[0].z)
        return 0;
    if (box[0].z >= cylinder[1].z)
        return 0;
    if (delta != 0)
    {
        Vec4 boxCentre;
        boxCentre.Copy((box[0] + box[1]) * 0.5f);
        delta->Copy(boxCentre - centre);
        delta->z = 0;
    }
    if (negatedDelta != 0)
    {
        Vec4 boxCentre;
        boxCentre.Copy((box[0] + box[1]) * 0.5f);
        negatedDelta->Copy(centre - boxCentre);
    }
    return 1;
}

// 0x10192a00 (Cylinder_Overlap). The same test for upright cylinders inscribed in the boxes: each radius is half the
// box's x/y diagonal divided by 1.414. The cylinders overlap when their centres (the offsets) are closer than the
// two radii together in x/y and their height ranges overlap. On overlap `delta` receives the first offset minus
// the second and `negatedDelta` the second minus the first, both with z = 0. Returns 1 on overlap, else 0.
extern "C" int __stdcall FUN_10192a00(const Box *first, const Vec4 *firstOffset, const Box *second,
                                      const Vec4 *secondOffset, Vec4 *delta, Vec4 *negatedDelta)
{
    float firstRadius;
    {
        float dy = first->max.y - first->min.y;
        float dx = first->max.x - first->min.x;
        firstRadius = (float)sqrt(dx * dx + dy * dy) * 0.5f / 1.414f;
    }
    float secondRadius;
    {
        float dy = second->max.y - second->min.y;
        float dx = second->max.x - second->min.x;
        secondRadius = (float)sqrt(dx * dx + dy * dy) * 0.5f / 1.414f;
    }
    Vec4 b;
    b.x = secondOffset->x;
    b.y = secondOffset->y;
    Vec4 a;
    a.x = firstOffset->x;
    a.y = firstOffset->y;
    float dy = b.y - a.y;
    float dx = b.x - a.x;
    float reach = secondRadius + firstRadius;
    if (dx * dx + dy * dy >= reach * reach)
        return 0;
    Vec4 firstBox[2];
    firstBox[0].x = firstOffset->x - firstRadius;
    firstBox[0].y = firstOffset->y - firstRadius;
    firstBox[0].z = first->min.z + firstOffset->z;
    firstBox[1].x = firstRadius + firstOffset->x;
    firstBox[1].y = firstRadius + firstOffset->y;
    firstBox[1].z = first->max.z + firstOffset->z;
    Vec4 secondBox[2];
    secondBox[0].x = secondOffset->x - secondRadius;
    secondBox[0].y = secondOffset->y - secondRadius;
    secondBox[0].z = second->min.z + secondOffset->z;
    secondBox[1].x = secondRadius + secondOffset->x;
    secondBox[1].y = secondRadius + secondOffset->y;
    secondBox[1].z = second->max.z + secondOffset->z;
    if (firstBox[1].z <= secondBox[0].z)
        return 0;
    if (firstBox[0].z >= secondBox[1].z)
        return 0;
    if (delta != 0)
    {
        delta->Copy(a - b);
        delta->z = 0;
    }
    if (negatedDelta != 0)
    {
        negatedDelta->Copy(b - a);
        negatedDelta->z = 0;
    }
    return 1;
}

// Side view of FUN_10192c90: whether the circle of `radiusSquared` around `point` (whose x/y hold the two
// coordinates of the view) touches the rectangle u0..u1 x v0..v1: inside it, or closer than the radius to an edge.
#define CIRCLE_MISSES_RECT(point, u0, v0, u1, v1, radiusSquared)                                                    \
    (((u0) >= (point).x || (u1) <= (point).x || (v0) >= (point).y || (v1) <= (point).y) &&                          \
     (point).DistanceSquaredToSegment2D(u0, v0, u1, v0) >= (radiusSquared) &&                                       \
     (point).DistanceSquaredToSegment2D(u1, v0, u1, v1) >= (radiusSquared) &&                                       \
     (point).DistanceSquaredToSegment2D(u0, v1, u1, v1) >= (radiusSquared) &&                                       \
     (point).DistanceSquaredToSegment2D(u0, v0, u0, v1) >= (radiusSquared))

// 0x10192c90 (CylinderSphere_Overlap). The upright cylinder inscribed in box `first` (moved by `firstOffset`)
// against the sphere `sphere` moved by `sphereOffset`; the sphere's w is its size, its radius w * 0.578 (about
// 1 / sqrt(3)). Seen from above the two circles must overlap; the cylinder's centre there takes x from the offset
// but y from the middle of the box. Seen from the side, across y and across x, the sphere's circle must touch the
// cylinder's rectangle (height range by radius). On overlap `delta`, when given, receives the cylinder centre minus
// the sphere centre, and `negatedDelta`, when given, the sphere centre minus the cylinder centre with z = 0.
// Returns 1 on overlap, else 0. Called by the scene node code at 0x10193140.
extern "C" int __stdcall FUN_10192c90(const Box *first, const Vec4 *firstOffset, const Vec4 *sphere,
                                      const Vec4 *sphereOffset, Vec4 *delta, Vec4 *negatedDelta)
{
    float radius;
    {
        float dy = first->max.y - first->min.y;
        float dx = first->max.x - first->min.x;
        radius = (float)sqrt(dx * dx + dy * dy) * 0.5f / 1.414f;
    }
    Vec4 point;
    point.x = sphere->x + sphereOffset->x;
    point.y = sphere->y + sphereOffset->y;
    point.z = sphere->z + sphereOffset->z;
    Vec4 centre;
    centre.x = firstOffset->x;
    centre.y = (first->min.y + first->max.y) * 0.5f + firstOffset->y;
    centre.z = firstOffset->z;
    float sphereRadius = sphere->w * 0.578f;

    // Top view: the two circles.
    float dy = point.y - centre.y;
    float dx = point.x - centre.x;
    float reach = sphereRadius + radius;
    if (dx * dx + dy * dy >= reach * reach)
        return 0;

    Vec4 box[2];
    box[0].x = firstOffset->x - radius;
    box[0].y = firstOffset->y - radius;
    box[0].z = first->min.z + firstOffset->z;
    box[1].x = radius + firstOffset->x;
    box[1].y = radius + firstOffset->y;
    box[1].z = first->max.z + firstOffset->z;
    float sphereRadiusSquared = sphereRadius * sphereRadius;

    // Side view across y (point = sphere z, y).
    point.x = sphere->z + sphereOffset->z;
    point.y = sphere->y + sphereOffset->y;
    if (CIRCLE_MISSES_RECT(point, box[0].z, box[0].y, box[1].z, box[1].y, sphereRadiusSquared))
        return 0;
    // Side view across x (point = sphere z, x).
    point.x = sphere->z + sphereOffset->z;
    point.y = sphere->x + sphereOffset->x;
    if (CIRCLE_MISSES_RECT(point, box[0].z, box[0].x, box[1].z, box[1].x, sphereRadiusSquared))
        return 0;

    point.x = sphere->x + sphereOffset->x;
    point.y = sphere->y + sphereOffset->y;
    if (delta != 0)
        delta->Copy(centre - point);
    if (negatedDelta != 0)
    {
        negatedDelta->Copy(point - centre);
        negatedDelta->z = 0;
    }
    return 1;
}
