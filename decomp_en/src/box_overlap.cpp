// Overlap tests between the boxes (and the cylinders inscribed in them) of nodes (EN build). Ported from the matched JP
// source decomp/src/box_overlap.cpp. `this` is not used by any of the three; the callers pass the nodes' own boxes.
#include <math.h>

#include "node.h"

// The upright cylinder inscribed in a box has the radius of half the box's x/y diagonal divided by this factor.
const float CYLINDER_DIAGONAL_FACTOR = 1.414f;
// The centre of a box is the mean of its corners: its two corners summed, times 0.5 (written as a literal: the engine
// passes the factor to Vec4::operator* by reference).

// Node::BoxesOverlap: moves box `first` by `firstOffset` and box `second` by `secondOffset` and returns 1 when they
// overlap on all three axes (touching faces do not count), else 0. On overlap `centreDelta`, when given, receives the
// centre of the first box minus the centre of the second, and `negatedDelta`, when given, its negation (it reads
// `centreDelta`, so both must be given for that). Called by the node collision tests.
int Node::BoxesOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *second, const Vec4 *secondOffset,
                       Vec4 *centreDelta, Vec4 *negatedDelta)
{
    Vec4 a[2];
    Vec4 b[2];
    a[NODE_BOX_MIN].x = first[NODE_BOX_MIN].x + firstOffset->x;
    a[NODE_BOX_MIN].y = first[NODE_BOX_MIN].y + firstOffset->y;
    a[NODE_BOX_MIN].z = first[NODE_BOX_MIN].z + firstOffset->z;
    a[NODE_BOX_MAX].x = first[NODE_BOX_MAX].x + firstOffset->x;
    a[NODE_BOX_MAX].y = first[NODE_BOX_MAX].y + firstOffset->y;
    a[NODE_BOX_MAX].z = first[NODE_BOX_MAX].z + firstOffset->z;
    b[NODE_BOX_MIN].x = second[NODE_BOX_MIN].x + secondOffset->x;
    b[NODE_BOX_MIN].y = second[NODE_BOX_MIN].y + secondOffset->y;
    b[NODE_BOX_MIN].z = second[NODE_BOX_MIN].z + secondOffset->z;
    b[NODE_BOX_MAX].x = second[NODE_BOX_MAX].x + secondOffset->x;
    b[NODE_BOX_MAX].y = second[NODE_BOX_MAX].y + secondOffset->y;
    b[NODE_BOX_MAX].z = second[NODE_BOX_MAX].z + secondOffset->z;
    if (a[NODE_BOX_MAX].x <= b[NODE_BOX_MIN].x)
        return 0;
    if (a[NODE_BOX_MIN].x >= b[NODE_BOX_MAX].x)
        return 0;
    if (a[NODE_BOX_MAX].y <= b[NODE_BOX_MIN].y)
        return 0;
    if (a[NODE_BOX_MIN].y >= b[NODE_BOX_MAX].y)
        return 0;
    if (a[NODE_BOX_MAX].z <= b[NODE_BOX_MIN].z)
        return 0;
    if (a[NODE_BOX_MIN].z >= b[NODE_BOX_MAX].z)
        return 0;
    if (centreDelta != 0)
    {
        Vec4 centreA;
        Vec4 centreB;
        centreA.Copy((a[NODE_BOX_MIN] + a[NODE_BOX_MAX]) * 0.5f);
        centreB.Copy((b[NODE_BOX_MIN] + b[NODE_BOX_MAX]) * 0.5f);
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

// Node::CylindersOverlap: the same test for upright cylinders inscribed in the boxes: each radius is half the box's
// x/y diagonal divided by CYLINDER_DIAGONAL_FACTOR. The cylinders overlap when their centres (the offsets) are closer
// than the two radii together in x/y and their height ranges overlap. On overlap `delta` receives the first offset
// minus the second and `negatedDelta` the second minus the first, both with z = 0. Returns 1 on overlap, else 0.
int Node::CylindersOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *second, const Vec4 *secondOffset,
                           Vec4 *delta, Vec4 *negatedDelta)
{
    float firstRadius;
    {
        float dy = first[NODE_BOX_MAX].y - first[NODE_BOX_MIN].y;
        float dx = first[NODE_BOX_MAX].x - first[NODE_BOX_MIN].x;
        firstRadius = (float)sqrt(dx * dx + dy * dy) * 0.5f / CYLINDER_DIAGONAL_FACTOR;
    }
    float secondRadius;
    {
        float dy = second[NODE_BOX_MAX].y - second[NODE_BOX_MIN].y;
        float dx = second[NODE_BOX_MAX].x - second[NODE_BOX_MIN].x;
        secondRadius = (float)sqrt(dx * dx + dy * dy) * 0.5f / CYLINDER_DIAGONAL_FACTOR;
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
    firstBox[NODE_BOX_MIN].x = firstOffset->x - firstRadius;
    firstBox[NODE_BOX_MIN].y = firstOffset->y - firstRadius;
    firstBox[NODE_BOX_MIN].z = first[NODE_BOX_MIN].z + firstOffset->z;
    firstBox[NODE_BOX_MAX].x = firstRadius + firstOffset->x;
    firstBox[NODE_BOX_MAX].y = firstRadius + firstOffset->y;
    firstBox[NODE_BOX_MAX].z = first[NODE_BOX_MAX].z + firstOffset->z;
    Vec4 secondBox[2];
    secondBox[NODE_BOX_MIN].x = secondOffset->x - secondRadius;
    secondBox[NODE_BOX_MIN].y = secondOffset->y - secondRadius;
    secondBox[NODE_BOX_MIN].z = second[NODE_BOX_MIN].z + secondOffset->z;
    secondBox[NODE_BOX_MAX].x = secondRadius + secondOffset->x;
    secondBox[NODE_BOX_MAX].y = secondRadius + secondOffset->y;
    secondBox[NODE_BOX_MAX].z = second[NODE_BOX_MAX].z + secondOffset->z;
    if (firstBox[NODE_BOX_MAX].z <= secondBox[NODE_BOX_MIN].z)
        return 0;
    if (firstBox[NODE_BOX_MIN].z >= secondBox[NODE_BOX_MAX].z)
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

// Node::BoxCylinderOverlap: box `first` moved by `firstOffset` against the upright cylinder inscribed in box `second`
// moved by `secondOffset` (radius as in CylindersOverlap). In x/y they overlap when the cylinder's centre lies inside
// the box's rectangle or closer than the radius to one of its four edges; their height ranges must overlap as well.
// On overlap `delta`, when given, receives the box centre minus the cylinder centre with z = 0, and `negatedDelta`,
// when given, the cylinder centre minus the box centre. Returns 1 on overlap, else 0.
int Node::BoxCylinderOverlap(const Vec4 *first, const Vec4 *firstOffset, const Vec4 *second,
                             const Vec4 *secondOffset, Vec4 *delta, Vec4 *negatedDelta)
{
    Vec4 box[2];
    box[NODE_BOX_MIN].x = first[NODE_BOX_MIN].x + firstOffset->x;
    box[NODE_BOX_MIN].y = first[NODE_BOX_MIN].y + firstOffset->y;
    box[NODE_BOX_MIN].z = first[NODE_BOX_MIN].z + firstOffset->z;
    box[NODE_BOX_MAX].x = first[NODE_BOX_MAX].x + firstOffset->x;
    box[NODE_BOX_MAX].y = first[NODE_BOX_MAX].y + firstOffset->y;
    box[NODE_BOX_MAX].z = first[NODE_BOX_MAX].z + firstOffset->z;
    float radius;
    {
        float dy = second[NODE_BOX_MAX].y - second[NODE_BOX_MIN].y;
        float dx = second[NODE_BOX_MAX].x - second[NODE_BOX_MIN].x;
        radius = (float)sqrt(dx * dx + dy * dy) * 0.5f / CYLINDER_DIAGONAL_FACTOR;
    }
    Vec4 cylinder[2];
    cylinder[NODE_BOX_MIN].x = secondOffset->x - radius;
    cylinder[NODE_BOX_MIN].y = secondOffset->y - radius;
    cylinder[NODE_BOX_MIN].z = second[NODE_BOX_MIN].z + secondOffset->z;
    cylinder[NODE_BOX_MAX].x = radius + secondOffset->x;
    cylinder[NODE_BOX_MAX].y = radius + secondOffset->y;
    cylinder[NODE_BOX_MAX].z = second[NODE_BOX_MAX].z + secondOffset->z;
    float radiusSquared = radius * radius;
    Vec4 centre;
    centre.x = secondOffset->x;
    centre.y = secondOffset->y;
    centre.z = secondOffset->z;

    // Outside the rectangle: the centre must be within the radius of one of the four edges.
    if ((box[NODE_BOX_MIN].x >= centre.x || box[NODE_BOX_MAX].x <= centre.x || box[NODE_BOX_MIN].y >= centre.y ||
         box[NODE_BOX_MAX].y <= centre.y) &&
        centre.DistanceSquaredToSegment2D(box[NODE_BOX_MIN].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MAX].x,
                                          box[NODE_BOX_MIN].y) >= radiusSquared &&
        centre.DistanceSquaredToSegment2D(box[NODE_BOX_MAX].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MAX].x,
                                          box[NODE_BOX_MAX].y) >= radiusSquared &&
        centre.DistanceSquaredToSegment2D(box[NODE_BOX_MIN].x, box[NODE_BOX_MAX].y, box[NODE_BOX_MAX].x,
                                          box[NODE_BOX_MAX].y) >= radiusSquared &&
        centre.DistanceSquaredToSegment2D(box[NODE_BOX_MIN].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MIN].x,
                                          box[NODE_BOX_MAX].y) >= radiusSquared)
        return 0;
    if (box[NODE_BOX_MAX].z <= cylinder[NODE_BOX_MIN].z)
        return 0;
    if (box[NODE_BOX_MIN].z >= cylinder[NODE_BOX_MAX].z)
        return 0;
    if (delta != 0)
    {
        Vec4 boxCentre;
        boxCentre.Copy((box[NODE_BOX_MIN] + box[NODE_BOX_MAX]) * 0.5f);
        delta->Copy(boxCentre - centre);
        delta->z = 0;
    }
    if (negatedDelta != 0)
    {
        Vec4 boxCentre;
        boxCentre.Copy((box[NODE_BOX_MIN] + box[NODE_BOX_MAX]) * 0.5f);
        negatedDelta->Copy(centre - boxCentre);
    }
    return 1;
}
