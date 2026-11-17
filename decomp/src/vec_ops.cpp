// Vector helper built on Vec4 subtraction and Cross.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Copy(const Vec4 &source);
    Vec4 operator-(const Vec4 &other) const;  // 0x101aca40 (component-wise difference, w included)
};

Vec4 Cross(const Vec4 &a, const Vec4 &b);

// 0x101ae8b0: the normal of the triangle (origin, a, b): the cross product of the edges a - origin and b - origin
// (not normalised).
Vec4 TriangleNormal(const Vec4 &a, const Vec4 &origin, const Vec4 &b)
{
    Vec4 edgeB;
    Vec4 edgeA;
    Vec4 result;
    edgeA.Copy(a - origin);
    edgeB.Copy(b - origin);
    result.Copy(Cross(edgeA, edgeB));
    return result;
}
