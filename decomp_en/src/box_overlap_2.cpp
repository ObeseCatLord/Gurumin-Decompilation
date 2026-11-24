// Overlap tests of a node's box or cylinder against a sphere (EN build). Ported from the matched JP source
// decomp/src/box_overlap.cpp; the box/cylinder pairs are in box_overlap.cpp. `this` is not used by either test.
#include <math.h>

#include "node.h"

// The upright cylinder inscribed in a box has the radius of half the box's x/y diagonal divided by this factor.
const float CYLINDER_DIAGONAL_FACTOR = 1.414f;
// The radius of a bounding sphere is its size (w) times this factor (about 1 / sqrt(3)).
const float SPHERE_RADIUS_FACTOR = 0.578f;
// Scale applied to the z push of BoxSphereOverlap's negated delta.
const float PUSH_Z_SCALE = 0.01f;

// Side view of Node::CylinderSphereOverlap: whether the circle of `radiusSquared` around `point` (whose x/y hold the two
// coordinates of the view) touches the rectangle u0..u1 x v0..v1: inside it, or closer than the radius to an edge.
#define CIRCLE_MISSES_RECT(point, u0, v0, u1, v1, radiusSquared)                                                    \
    (((u0) >= (point).x || (u1) <= (point).x || (v0) >= (point).y || (v1) <= (point).y) &&                          \
     (point).DistanceSquaredToSegment2D(u0, v0, u1, v0) >= (radiusSquared) &&                                       \
     (point).DistanceSquaredToSegment2D(u1, v0, u1, v1) >= (radiusSquared) &&                                       \
     (point).DistanceSquaredToSegment2D(u0, v1, u1, v1) >= (radiusSquared) &&                                       \
     (point).DistanceSquaredToSegment2D(u0, v0, u0, v1) >= (radiusSquared))

// JP 0x10192c90. The upright cylinder inscribed in box `first` (moved by `firstOffset`)
// against the sphere `sphere` moved by `sphereOffset`; the sphere's w is its size, its radius w * 0.578 (about
// 1 / sqrt(3)). Seen from above the two circles must overlap; the cylinder's centre there takes x from the offset
// but y from the middle of the box. Seen from the side, across y and across x, the sphere's circle must touch the
// cylinder's rectangle (height range by radius). On overlap `delta`, when given, receives the cylinder centre minus
// the sphere centre, and `negatedDelta`, when given, the sphere centre minus the cylinder centre with z = 0.
// Returns 1 on overlap, else 0. Called by the scene node code at 0x10193140.
int Node::CylinderSphereOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *sphere,
                                      const Vec4 *sphereOffset, Vec4 *delta, Vec4 *negatedDelta)
{
    float radius;
    {
        float dy = first[NODE_BOX_MAX].y - first[NODE_BOX_MIN].y;
        float dx = first[NODE_BOX_MAX].x - first[NODE_BOX_MIN].x;
        radius = (float)sqrt(dx * dx + dy * dy) * 0.5f / CYLINDER_DIAGONAL_FACTOR;
    }
    Vec4 point;
    point.x = sphere->x + sphereOffset->x;
    point.y = sphere->y + sphereOffset->y;
    point.z = sphere->z + sphereOffset->z;
    Vec4 centre;
    centre.x = firstOffset->x;
    centre.y = (first[NODE_BOX_MIN].y + first[NODE_BOX_MAX].y) * 0.5f + firstOffset->y;
    centre.z = firstOffset->z;
    float sphereRadius = sphere->w * SPHERE_RADIUS_FACTOR;

    // Top view: the two circles.
    float dy = point.y - centre.y;
    float dx = point.x - centre.x;
    float reach = sphereRadius + radius;
    if (dx * dx + dy * dy >= reach * reach)
        return 0;

    Vec4 box[2];
    box[0].x = firstOffset->x - radius;
    box[0].y = firstOffset->y - radius;
    box[0].z = first[NODE_BOX_MIN].z + firstOffset->z;
    box[1].x = radius + firstOffset->x;
    box[1].y = radius + firstOffset->y;
    box[1].z = first[NODE_BOX_MAX].z + firstOffset->z;
    float sphereRadiusSquared = sphereRadius * sphereRadius;

    // Side view across y (point = sphere z, y).
    point.x = sphere->z + sphereOffset->z;
    point.y = sphereOffset->y + sphere->y;
    if (CIRCLE_MISSES_RECT(point, box[0].z, box[0].y, box[1].z, box[1].y, sphereRadiusSquared))
        return 0;
    // Side view across x (point = sphere z, x).
    point.x = sphere->z + sphereOffset->z;
    point.y = sphere->x + sphereOffset->x;
    if (CIRCLE_MISSES_RECT(point, box[0].z, box[0].x, box[1].z, box[1].x, sphereRadiusSquared))
        return 0;

    point.x = sphere->x + sphereOffset->x;
    point.y = sphereOffset->y + sphere->y;
    if (delta != 0)
        delta->Copy(centre - point);
    if (negatedDelta != 0)
    {
        negatedDelta->Copy(point - centre);
        negatedDelta->z = 0;
    }
    return 1;
}

// Whether the circle of `radiusSquared` around `point` reaches the segment (u0, v0)-(u1, v1) of a side view: written
// as "not at least the radius away" (the original's comparison, which also counts an unordered result as near).
#define NEAR_EDGE(u0, v0, u1, v1) (!(point.DistanceSquaredToSegment2D(u0, v0, u1, v1) >= radiusSquared))

// JP 0x10192520. Box `first` moved by `firstOffset` against the sphere `sphere` moved by
// `sphereOffset` (radius w * 0.578, as in CylinderSphere_Overlap). The sphere's circle must touch the box's
// rectangle in three views: across x (z, y), across y (z, x) and from above (x, y). Each view also records the
// axis to push along: a circle inside the rectangle pushes along the view's own axis (x for the first view, z for
// the top view; the second view records nothing), a circle touching an edge pushes along that edge's normal. On
// overlap `delta`, when given, receives the box centre minus the sphere centre, and `negatedDelta`, when given, the
// sphere centre minus the box centre with its z scaled by 0.01; in both, axes no view chose are zeroed. Returns 1
// on overlap, else 0. Called by the scene node code at 0x10193140.
int Node::BoxSphereOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *sphere,
                                      const Vec4 *sphereOffset, Vec4 *delta, Vec4 *negatedDelta)
{
    Vec4 point;
    Vec4 box[2];
    box[0].x = first[NODE_BOX_MIN].x + firstOffset->x;
    box[0].y = first[NODE_BOX_MIN].y + firstOffset->y;
    box[0].z = first[NODE_BOX_MIN].z + firstOffset->z;
    box[1].x = first[NODE_BOX_MAX].x + firstOffset->x;
    box[1].y = first[NODE_BOX_MAX].y + firstOffset->y;
    box[1].z = first[NODE_BOX_MAX].z + firstOffset->z;
    int pushX = 0;
    int pushY = 0;
    int pushZ = 0;
    float sphereRadius = sphere->w * SPHERE_RADIUS_FACTOR;
    float radiusSquared = sphereRadius * sphereRadius;

    // Across x: point = sphere (z, y).
    point.x = sphereOffset->z + sphere->z;
    point.y = sphereOffset->y + sphere->y;
    if (box[0].z >= point.x || box[1].z <= point.x || box[0].y >= point.y || box[1].y <= point.y)
    {
        if (NEAR_EDGE(box[0].z, box[0].y, box[1].z, box[0].y))
            pushY = 1;
        else if (NEAR_EDGE(box[1].z, box[0].y, box[1].z, box[1].y))
            pushZ = 1;
        else if (NEAR_EDGE(box[0].z, box[1].y, box[1].z, box[1].y))
            pushY = 1;
        else if (NEAR_EDGE(box[0].z, box[0].y, box[0].z, box[1].y))
            pushZ = 1;
        else
            return 0;
    }
    else
        pushX = 1;

    // Across y: point = sphere (z, x).
    point.x = sphereOffset->z + sphere->z;
    point.y = sphere->x + sphereOffset->x;
    if (box[0].z >= point.x || box[1].z <= point.x || box[0].x >= point.y || box[1].x <= point.y)
    {
        if (NEAR_EDGE(box[0].z, box[0].x, box[1].z, box[0].x))
            pushX = 1;
        else if (NEAR_EDGE(box[1].z, box[0].x, box[1].z, box[1].x))
            pushZ = 1;
        else if (NEAR_EDGE(box[0].z, box[1].x, box[1].z, box[1].x))
            pushX = 1;
        else if (NEAR_EDGE(box[0].z, box[0].x, box[0].z, box[1].x))
            pushZ = 1;
        else
            return 0;
    }

    // From above: point = sphere (x, y, z).
    point.x = sphere->x + sphereOffset->x;
    point.y = sphereOffset->y + sphere->y;
    point.z = sphereOffset->z + sphere->z;
    if (box[0].x >= point.x || box[1].x <= point.x || box[0].y >= point.y || box[1].y <= point.y)
    {
        if (NEAR_EDGE(box[0].x, box[0].y, box[1].x, box[0].y))
            pushY = 1;
        else if (NEAR_EDGE(box[1].x, box[0].y, box[1].x, box[1].y))
            pushX = 1;
        else if (NEAR_EDGE(box[0].x, box[1].y, box[1].x, box[1].y))
            pushY = 1;
        else if (NEAR_EDGE(box[0].x, box[0].y, box[0].x, box[1].y))
            pushX = 1;
        else
            return 0;
    }
    else
        pushZ = 1;

    if (delta != 0)
    {
        Vec4 boxCentre;
        boxCentre.Copy((box[0] + box[1]) * 0.5f);
        delta->Copy(boxCentre - point);
        if (!pushX)
            delta->x = 0;
        if (!pushY)
            delta->y = 0;
        if (!pushZ)
            delta->z = 0;
    }
    if (negatedDelta != 0)
    {
        Vec4 boxCentre;
        boxCentre.Copy((box[0] + box[1]) * 0.5f);
        negatedDelta->Copy(point - boxCentre);
        if (!pushX)
            negatedDelta->x = 0;
        if (!pushY)
            negatedDelta->y = 0;
        if (!pushZ)
            negatedDelta->z = 0;
        negatedDelta->z *= PUSH_Z_SCALE;
    }
    return 1;
}

