// Bounding-sphere overlap test between two scene nodes (kept apart from scene_node.cpp, whose Vec4 has no
// constructor or operators).

struct Vec4
{
    float x;
    float y;
    float z;
    float w;                  // the sphere's radius when the vector is a node's bounds (Node 0x6d4..0x6e0)
    Vec4();
    void Copy(const Vec4 &source);
    Vec4 operator+(const Vec4 &other) const;
    Vec4 operator-(const Vec4 &other) const;
    float DistanceSquared(const Vec4 &other);  // 0x101ac650
};

struct Node
{
    int SpheresOverlap(const Vec4 *boundsA, const Vec4 *offsetA, const Vec4 *boundsB, const Vec4 *offsetB,
                       Vec4 *aToB, Vec4 *bToA);
};

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
