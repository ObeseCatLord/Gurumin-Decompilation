// Triangle helpers built on Vec4 subtraction and Cross.

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

// Plane a*x + b*y + c*z + d = 0 (vec_math.cpp).
struct Plane
{
    float a;
    float b;
    float c;
    float d;
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

// 0x101af360: the plane through the triangle (origin, a, b): the normal is (a - origin) x (b - origin) (not
// normalised) and d puts `origin` on the plane.
Plane PlaneFromTriangle(const Vec4 &a, const Vec4 &origin, const Vec4 &b)
{
    Vec4 edgeB;
    Vec4 edgeA;
    Vec4 normal;
    edgeA.Copy(a - origin);
    edgeB.Copy(b - origin);
    normal.Copy(Cross(edgeA, edgeB));
    Plane plane;
    plane.a = normal.x;
    plane.b = normal.y;
    plane.c = normal.z;
    plane.d = -(normal.x * origin.x + normal.y * origin.y + normal.z * origin.z);
    return plane;
}
