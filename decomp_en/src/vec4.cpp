// EN Vec4 and Matrix4 helpers (JP twins: vec4.cpp, vec_math.cpp, containers.cpp). The EN compiler inlines small
// helpers and uses SSE2 for float math.

#include <math.h>
#include "vec4.h"

// Factor applied to the z difference in DistanceZWeighted.
static const float Z_DISTANCE_WEIGHT = 2.0f;

// Zero vector. Called wherever a Vec4 local is default-constructed out of line.
Vec4::Vec4()
{
    x = 0;
    y = 0;
    z = 0;
    w = 0;
}

// Vector from four components.
Vec4::Vec4(float nx, float ny, float nz, float nw)
{
    x = nx;
    y = ny;
    z = nz;
    w = nw;
}

// Sets all four components.
void Vec4::Set(float nx, float ny, float nz, float nw)
{
    x = nx;
    y = ny;
    z = nz;
    w = nw;
}

// Copies all four components of `source`.
void Vec4::Copy(const Vec4 &source)
{
    x = source.x;
    y = source.y;
    z = source.z;
    w = source.w;
}

// Sets all four components to the same value.
void Vec4::Fill(const float &value)
{
    x = value;
    y = value;
    z = value;
    w = value;
}

// Squared length of the x/y/z part. Used for distance comparisons.
float Vec4::LengthSquared3()
{
    return x * x + y * y + z * z;
}

// Dot product of the x/y/z parts of two vectors (free function, cdecl).
extern "C" float Vec3_Dot(const Vec4 *a, const Vec4 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

// Returns the translation row as a vector (w stays 0).
Vec4 Matrix4::GetTranslation()
{
    Vec4 result;
    result.x = m[MATRIX_TRANSLATION_X];
    result.y = m[MATRIX_TRANSLATION_Y];
    result.z = m[MATRIX_TRANSLATION_Z];
    return result;
}

// Sets the translation row to position.x/y/z (position.w is ignored).
void Matrix4::SetTranslation(Vec4 position)
{
    m[MATRIX_TRANSLATION_X] = position.x;
    m[MATRIX_TRANSLATION_Y] = position.y;
    m[MATRIX_TRANSLATION_Z] = position.z;
}

// Moves the matrix: adds (x, y, z) to its translation row.
void Matrix4::Translate(float x, float y, float z)
{
    m[MATRIX_TRANSLATION_X] += x;
    m[MATRIX_TRANSLATION_Y] += y;
    m[MATRIX_TRANSLATION_Z] += z;
}

// Matrix constructor: zeroes every element except m[5], m[9] and m[13] (no identity is set); callers that need a
// defined matrix overwrite it. The chained assignment reproduces the original store order (right to left).
Matrix4::Matrix4()
{
    m[12] = m[1] = m[14] = m[15] = m[8] = m[10] = m[11] = m[4] = m[6] = m[7] = m[0] = m[2] = m[3] = 0;
}

// Component-wise product with `other`.
Vec4 Vec4::operator*(const Vec4 &other) const
{
    Vec4 result;
    result.x = other.x * x;
    result.y = other.y * y;
    result.z = other.z * z;
    result.w = other.w * w;
    return result;
}

// Difference this - other (all four components).
Vec4 Vec4::operator-(const Vec4 &other) const
{
    Vec4 result;
    result.x = x - other.x;
    result.y = y - other.y;
    result.z = z - other.z;
    result.w = w - other.w;
    return result;
}

// Sum this + other (all four components).
Vec4 Vec4::operator+(const Vec4 &other) const
{
    Vec4 result;
    result.x = other.x + x;
    result.y = other.y + y;
    result.z = other.z + z;
    result.w = other.w + w;
    return result;
}

// Returns this vector divided by `divisor` (x/y/z; w stays 0).
Vec4 Vec4::Divided(const float &divisor)
{
    Vec4 result;
    result.x = x / divisor;
    result.y = y / divisor;
    result.z = z / divisor;
    return result;
}

// 3D distance to `other`.
float Vec4::Distance(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = other.z - z;
    return (float)sqrt(dy * dy + dx * dx + dz * dz);
}

// Length of the x/y/z part.
float Vec4::Length()
{
    return (float)sqrt(x * x + y * y + z * z);
}

// Cross product a x b (x/y/z; the result's w stays 0).
Vec4 Cross(const Vec4 &a, const Vec4 &b)
{
    Vec4 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

// Squared 3D distance to `other`.
float Vec4::DistanceSquared(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = other.z - z;
    return dy * dy + dx * dx + dz * dz;
}

// Distance between two points on the x/y plane (z ignored). `other` is passed by value.
float Vec4::Distance2D(Vec4 other)
{
    float dy = y - other.y;
    float dx = x - other.x;
    return (float)sqrt(dx * dx + dy * dy);
}

// Length of the x/y part only (distance on the ground plane from the origin).
float Vec4::Length2D()
{
    return (float)sqrt(x * x + y * y);
}

// 3D distance with the z difference counted twice, so points stacked vertically count as farther apart.
float Vec4::DistanceZWeighted(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = (other.z - z) * Z_DISTANCE_WEIGHT;
    return (float)sqrt(dy * dy + dx * dx + dz * dz);
}

// 1 if the point lies inside the box on the x/y plane (edges included), else 0. Early exit, NaN counts as inside.
int Vec4::InRect(const Box2 *box)
{
    if (x < box->min.x || x > box->max.x || y < box->min.y || y > box->max.y)
        return 0;
    return 1;
}

// 1 if the point lies inside the box (x, y and z; edges included), else 0. Same NaN-safe early exit as InRect.
int Vec4::InBox(const Box2 *box)
{
    if (x < box->min.x || x > box->max.x || y < box->min.y || y > box->max.y || z < box->min.z || z > box->max.z)
        return 0;
    return 1;
}

// Rotates the vector's x/y part by `angle` radians about the z axis (z and w unchanged).
// Open: 17 bytes differ (the compiler schedules y * cos before x * sin).
void Vec4::RotateZ(float angle)
{
    float cosX = (float)cos(angle) * x;
    float nx = cosX - (float)sin(angle) * y;
    float cosY = (float)cos(angle) * y;
    float ny = (float)sin(angle) * x + cosY;
    x = nx;
    y = ny;
}
