// Bounding sphere tests between nodes (EN build). Ported from the matched JP source decomp/src/node_spheres.cpp.
#include "node.h"

// The spheres overlap when their centres are closer than this factor times the sum of the radii (about 1/sqrt(3)).
const float SPHERE_OVERLAP_FACTOR = 0.578f;

// Node::SpheresOverlap: 1 when the bounding spheres `boundsA` and `boundsB` (centre x/y/z, radius w), each moved by its
// offset, are closer than SPHERE_OVERLAP_FACTOR times the sum of their radii. The vectors between the moved centres go
// to `aToB` (A - B) and `bToA` (B - A) when given. `this` is not used; the callers pass the nodes' own bounds.
int Node::SpheresOverlap(const Vec4 *boundsA, const Vec4 *offsetA, const Vec4 *boundsB, const Vec4 *offsetB,
                         Vec4 *aToB, Vec4 *bToA)
{
    Vec4 a;
    Vec4 b;
    a.Copy(*offsetA + *boundsA);
    b.Copy(*offsetB + *boundsB);
    if (aToB != 0)
        aToB->Copy(a - b);
    if (bToA != 0)
        bToA->Copy(b - a);
    float reach = (boundsA->w + boundsB->w) * SPHERE_OVERLAP_FACTOR;
    if (a.DistanceSquared(b) < reach * reach)
        return 1;
    return 0;
}

// Node::FindTouchingSphere: looks for nodes below this one (this one included) whose bounding sphere, moved to the
// node's position, overlaps the sphere of `radius` around `center` (SpheresOverlap divides the radius back out). Only
// nodes with the sphere-shape flag and collision are tested; the search goes down through the others. Each hit is
// appended to `found` (when given; then every hit is collected) and 1 is returned, else 0. Called by the character
// hit tests.
int Node::FindTouchingSphere(int unused, const Vec4 *center, float radius, Node **found, int *foundCount)
{
    if (g_testSkinnedNodes == 0 && skinned != 0)
        return 0;
    if ((flags & NODE_FLAG_SHAPE_SPHERE) == 0)
    {
        for (int i = 0; i < childCount; i++)
        {
            int hit = children[i]->FindTouchingSphere(unused, center, radius, found, foundCount);
            if (hit != 0 && found == 0)
                return hit;
        }
    }
    else if (hasCollision != 0)
    {
        Vec4 sphere;
        sphere.Fill(0);
        sphere.w = radius / SPHERE_OVERLAP_FACTOR;
        Vec4 position;
        position.Copy(*center);
        if (SpheresOverlap(&bounds, &worldMatrix.GetTranslation(), &sphere, &position, 0, 0) != 0)
        {
            if (found != 0)
            {
                found[*foundCount] = this;
                (*foundCount)++;
            }
            return 1;
        }
    }
    return 0;
}
