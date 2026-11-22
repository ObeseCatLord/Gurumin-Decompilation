// Bounding-sphere overlap test between two scene nodes (kept apart from scene_node.cpp, whose Vec4 has no
// constructor or operators).

#include "vec4.h"

#include "matrix4.h"

struct Node
{
    char pad_0000[0xa8];
    int m_00a8;               // 0xa8: non-zero when the node has collision
    unsigned int flags;       // 0xac: 0x400 = test this node itself, not its children
    char pad_00b0[0x19c];
    Matrix4 worldMatrix;       // 0x24c
    char pad_028c[0x448];
    Vec4 bounds;              // 0x6d4: bounding sphere (centre relative to the node, radius in w)
    char pad_06e4[0x20];
    char m_0704;              // 0x704: non-zero = skipped unless DAT_11b04510 is set
    char pad_0705[0x4b];
    int childCount;           // 0x750
    char childNames[0x40][0x40];
    Node *children[0x40];     // 0x1754
    int SpheresOverlap(const Vec4 *boundsA, const Vec4 *offsetA, const Vec4 *boundsB, const Vec4 *offsetB,
                       Vec4 *aToB, Vec4 *bToA);
    int FindTouchingSphere(int unused, const Vec4 *center, float radius, Node **found, int *foundCount);
};

extern "C" int DAT_11b04510;  // non-zero = also test the nodes marked in m_0704

// 0x10193060. 1 when the bounding spheres `boundsA` and `boundsB` (centre x/y/z, radius w), each moved by its offset,
// are closer than 0.578 times the sum of their radii (about 1/sqrt(3), so the spheres must overlap well). The
// vectors between the moved centres go to `aToB` (A - B) and `bToA` (B - A) when given. `this` is not used; the
// callers (0x10193140, 0x1019f210) pass the nodes' own bounds at 0x6d4.
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
    float reach = (boundsA->w + boundsB->w) * 0.578f;
    if (a.DistanceSquared(b) < reach * reach)
        return 1;
    return 0;
}

// 0x1019f210. Looks for nodes below this one (this one included) whose bounding sphere, moved to the node's
// position, overlaps the sphere of `radius` around `center` (SpheresOverlap divides the radius back out). Only nodes
// with flag 0x400 and collision are tested; the search goes down through the others. Each hit is appended to
// `found` (when given; then every hit is collected) and 1 is returned, else 0.
int Node::FindTouchingSphere(int unused, const Vec4 *center, float radius, Node **found, int *foundCount)
{
    if (DAT_11b04510 != 0 || m_0704 == 0)
    {
        if ((flags & 0x400) == 0)
        {
            for (int i = 0; i < childCount; i++)
            {
                int hit = children[i]->FindTouchingSphere(unused, center, radius, found, foundCount);
                if (hit != 0 && found == 0)
                    return hit;
            }
        }
        else if (m_00a8 != 0)
        {
            Vec4 sphere;
            sphere.Fill(0);
            sphere.w = radius / 0.578f;
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
    }
    return 0;
}
