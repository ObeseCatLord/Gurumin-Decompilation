// Vector and matrix math library (APP.dll 0x101ac000-0x101ae000).
//
// All math is single-precision float on the x87 FPU. The game was compiled with /Op (float consistency), so every
// intermediate result is rounded through memory, sqrt/sin/cos/atan2 are real CRT calls (no inline fsqrt), and float
// arguments of inline helpers are spilled to the stack. Matrices are row-major 4x4 with the translation in m[12..14]
// (Direct3D convention: row vectors multiplied on the left).
#include <math.h>

// Three floats; used where only x/y/z matter (positions, directions).
struct Vec3
{
    float x;
    float y;
    float z;
};

// Four floats; w is usually 1 for points and 0 for directions, and is ignored by most helpers here.
struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();  // 0x101ac990: zero vector
    float Length2D();
    float Distance2D(Vec4 other);
    float LengthSquared3();
    int InRect(const struct Box2 *box);
    int InBoxMargin(const struct Box2 *box, float margin);
    float DistanceSquared(const Vec4 &other);
    float Distance(const Vec4 &other);
    float DistanceZWeighted(const Vec4 &other);
    Vec4 Scaled(const float &scale);
    Vec4 Divided(const float &divisor);
};

// Axis-aligned box given by its minimum and maximum corners.
struct Box2
{
    Vec4 min;
    Vec4 max;
};

// Plane a*x + b*y + c*z + d = 0, stored in a Vec4 (x = a, y = b, z = c, w = d).
struct Plane
{
    float a;
    float b;
    float c;
    float d;
    int IntersectRay(Vec4 *out, const Vec4 *origin, const Vec4 *direction);
};

struct Matrix4
{
    float m[16];  // row-major; m[12], m[13], m[14] = translation
    Matrix4();
    void Translate(float x, float y, float z);
    void SetRotationZ(float angle);
};

// x*x + y*y + z*z. An inline helper in the original source: its three float arguments are evaluated right to left
// and spilled to the stack, which is what gives callers their z, y, x copy order.
inline float LengthSquared(float x, float y, float z)
{
    return x * x + y * y + z * z;
}

// Squared length of a Vec3 (free function, cdecl).
extern "C" float Vec3_LengthSquared(const Vec3 *v)
{
    return LengthSquared(v->x, v->y, v->z);
}

// Length of the x/y part only (distance on the ground plane from the origin).
float Vec4::Length2D()
{
    return (float)sqrt(x * x + y * y);
}

// Distance between two points on the x/y plane (z ignored). `other` is passed by value (16 bytes on the stack).
float Vec4::Distance2D(Vec4 other)
{
    float dy = y - other.y;
    float dx = x - other.x;
    return (float)sqrt(dx * dx + dy * dy);
}

// Squared length of x/y/z (w ignored). Cheaper than Length when only comparing distances.
float Vec4::LengthSquared3()
{
    return LengthSquared(x, y, z);
}

// Moves the matrix: adds (x, y, z) to its translation row.
void Matrix4::Translate(float x, float y, float z)
{
    m[12] += x;
    m[13] += y;
    m[14] += z;
}

// Matrix4 constructor. It zeroes every element except m[5], m[9] and m[13] (no identity is set); callers that need
// a defined matrix overwrite it. The chained assignment reproduces the original store order (right to left).
Matrix4::Matrix4()
{
    m[12] = m[1] = m[14] = m[15] = m[8] = m[10] = m[11] = m[4] = m[6] = m[7] = m[0] = m[2] = m[3] = 0;
}

// Squared 3D distance to `other`.
float Vec4::DistanceSquared(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = other.z - z;
    return dz * dz + dy * dy + dx * dx;
}

// 3D distance to `other`.
float Vec4::Distance(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = other.z - z;
    return (float)sqrt(dz * dz + dy * dy + dx * dx);
}

// 3D distance with the z difference counted twice, so points stacked vertically count as farther apart than
// points side by side (used for range checks that should prefer the same height).
float Vec4::DistanceZWeighted(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = (other.z - z) * 2;
    return (float)sqrt(dz * dz + dy * dy + dx * dx);
}

// 1 if the point lies inside the box on the x/y plane (edges included), else 0. Written as an early exit so the
// float compares come out NaN-safe like the original (a NaN coordinate counts as inside).
int Vec4::InRect(const Box2 *box)
{
    if (x < box->min.x || x > box->max.x || y < box->min.y || y > box->max.y)
        return 0;
    return 1;
}

// Returns this vector times `scale` (x/y/z; the result's w stays 0).
// Not byte-identical yet: the original keeps `this` in edx across the Vec4() constructor call (whole-program
// register allocation, see NOTES.md and open.tsv).
Vec4 Vec4::Scaled(const float &scale)
{
    Vec4 result;
    result.x = scale * x;
    result.y = y * scale;
    result.z = z * scale;
    return result;
}

// Returns this vector divided by `divisor` (x/y/z; w stays 0). Same register difference as Scaled.
Vec4 Vec4::Divided(const float &divisor)
{
    Vec4 result;
    result.x = x / divisor;
    result.y = y / divisor;
    result.z = z / divisor;
    return result;
}

// Length of a Vec3 (free function, cdecl).
extern "C" float Vec3_Length(const Vec3 *v)
{
    return (float)sqrt(Vec3_LengthSquared(v));
}

// Sets the upper-left 3x3 block to a rotation of `angle` radians about the z axis (m[3], m[7], m[11] and the
// translation row are left as they are). Statements are in the original's store order.
void Matrix4::SetRotationZ(float angle)
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    m[2] = 0;
    m[6] = 0;
    m[1] = -s;
    m[9] = 0;
    m[8] = 0;
    m[0] = c;
    m[5] = c;
    m[10] = 1.0f;
    m[4] = s;
}

// 1 if the point lies inside the box grown by `margin` on every side (x, y and z; edges included), else 0.
int Vec4::InBoxMargin(const Box2 *box, float margin)
{
    if (box->min.x - margin > x || margin + box->max.x < x || box->min.y - margin > y || margin + box->max.y < y ||
        box->min.z - margin > z || margin + box->max.z < z)
        return 0;
    return 1;
}

// Intersects the line origin + t * direction with the plane and stores the point in `out` (x/y/z).
// Returns 0 (and leaves `out` alone) when the line is parallel to the plane, else 1. t may be negative.
int Plane::IntersectRay(Vec4 *out, const Vec4 *origin, const Vec4 *direction)
{
    // The grouping y + (x + z) reproduces the original's evaluation order (the compiler orders the terms itself).
    float denominator = direction->y * b + (direction->x * a + direction->z * c);
    if (denominator == 0.0f)
        return 0;
    float t = -((origin->y * b + (origin->x * a + origin->z * c) + d) / denominator);
    out->x = t * direction->x + origin->x;
    out->y = t * direction->y + origin->y;
    out->z = t * direction->z + origin->z;
    return 1;
}
