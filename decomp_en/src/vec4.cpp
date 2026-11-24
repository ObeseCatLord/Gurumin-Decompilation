// EN Vec4 and Matrix4 helpers (JP twins: vec4.cpp, vec_math.cpp, containers.cpp). The EN compiler inlines small
// helpers and uses SSE2 for float math.

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "vec4.h"
#include "key_bank.h"
#include "containers.h"

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

// Returns this vector times `scale` (x/y/z; the result's w stays 0).
Vec4 Vec4::operator*(const float &scale) const
{
    Vec4 result;
    result.x = scale * x;
    result.y = y * scale;
    result.z = z * scale;
    return result;
}

// Transforms `point` by `matrix` and divides x, y and z by the resulting w (perspective divide); w is kept. Unlike a
// coordinate transform there is no sign fix for points behind the camera.
Vec4 Project(const Vec4 &point, const Matrix4 &matrix)
{
    float v[4];  // the transformed x, y, z, w
    v[0] = point.x * matrix.m[0] + point.y * matrix.m[4] + point.z * matrix.m[8] + matrix.m[12];
    v[1] = point.x * matrix.m[1] + point.y * matrix.m[5] + point.z * matrix.m[9] + matrix.m[13];
    v[2] = point.x * matrix.m[2] + point.y * matrix.m[6] + point.z * matrix.m[10] + matrix.m[14];
    v[3] = point.x * matrix.m[3] + point.y * matrix.m[7] + point.z * matrix.m[11] + matrix.m[15];
    Vec4 result;
    result.x = v[0] / v[3];
    result.y = v[1] / v[3];
    result.z = v[2] / v[3];
    result.w = v[3];
    return result;
}

// Transforms direction `v` by the rotation part of `matrix` (no translation); the result's w is 0.
Vec4 TransformNormal(const Vec4 &v, const Matrix4 &matrix)
{
    float t[4];  // the transformed x, y, z
    t[0] = v.x * matrix.m[0] + v.y * matrix.m[4] + v.z * matrix.m[8];
    t[1] = v.x * matrix.m[1] + v.y * matrix.m[5] + v.z * matrix.m[9];
    t[2] = v.x * matrix.m[2] + v.y * matrix.m[6] + v.z * matrix.m[10];
    Vec4 result;
    result.x = t[0];
    result.y = t[1];
    result.z = t[2];
    return result;
}

// Sets the upper-left 3x3 block to a rotation of `angle` radians about the z axis (m[3], m[7], m[11] and the
// translation row are left as they are).
void Matrix4::SetRotationZ(float angle)
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    m[2] = 0;
    m[6] = 0;
    m[10] = 1.0f;
    m[9] = 0;
    m[8] = 0;
    m[0] = c;
    m[5] = c;
    m[1] = -s;
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

// Rotates the vector's y/z part by `angle` radians about the x axis (the 2D rotation applied to (y, -z)).
void Vec4::RotateX(float angle)
{
    float c = (float)cos(angle);
    float negZ = -z;
    float s = (float)sin(angle);
    float ny = y * c - s * negZ;
    float nz = y * s + negZ * c;
    y = ny;
    z = -nz;
}

// Rotates the vector's x/z part by `angle` radians about the y axis (a positive angle turns x towards -z).
void Vec4::RotateY(float angle)
{
    float c = (float)cos(angle);
    float negZ = -z;
    float s = (float)sin(angle);
    float nx = x * c - s * negZ;
    float nz = x * s + negZ * c;
    z = -nz;
    x = nx;
}

// Pi as the game approximates it everywhere.
static const float GAME_PI = 3.1415f;
// Degrees in a half turn.
static const float HALF_TURN_DEGREES = 180.0f;

// Snaps an angle in radians to the nearest multiple of `step` degrees (rounding half away from zero) and returns it
// in radians again.
float SnapAngle(float radians, int step)
{
    float degrees = radians * HALF_TURN_DEGREES / GAME_PI;
    if (degrees >= 0.0f)
    {
        int snapped = ((int)degrees + step / 2) / step * step;
        return snapped * GAME_PI / HALF_TURN_DEGREES;
    }
    int snapped = ((int)degrees - step / 2) / step * step;
    return snapped * GAME_PI / HALF_TURN_DEGREES;
}

// Inserts key (x, y, z, w) at `frame`, before the first key whose frame is the same or later, so the keys stay
// sorted (an existing key at that frame moves up one place). Returns the new key's index.
int KeyBank::Insert(int frame, float x, float y, float z, float w)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (frames[i] == frame)
            break;
        if (frames[i] > frame)
            break;
    }
    for (int j = count - 1; j >= i; j--)
    {
        frames[j + 1] = frames[j];
        values[j + 1].Copy(values[j]);
        flags[j + 1] = flags[j];
    }
    frames[i] = frame;
    values[i].x = x;
    values[i].y = y;
    values[i].z = z;
    values[i].w = w;
    flags[i] = 0;
    count++;
    return i;
}

// Empties the bank, leaving one zero key at frame 0 as the default value.
void KeyBank::Clear()
{
    count = 0;
    frames[0] = 0;
    values[0].x = 0;
    values[0].y = 0;
    values[0].z = 0;
    values[0].w = 0;
}

// Constructs the key values (zero) and empties the bank.
KeyBank::KeyBank()
{
    Clear();
}

// Sets the key at `frame` to (x, y, z, w): an existing key at that frame gets the new value (returns 1); otherwise a
// new key is inserted in frame order like Insert (returns its index).
int KeyBank::Set(int frame, float x, float y, float z, float w)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (frames[i] == frame)
        {
            values[i].x = x;
            values[i].y = y;
            values[i].z = z;
            values[i].w = w;
            return 1;
        }
        if (frames[i] > frame)
            break;
    }
    for (int j = count - 1; j >= i; j--)
    {
        frames[j + 1] = frames[j];
        values[j + 1].Copy(values[j]);
        flags[j + 1] = flags[j];
    }
    frames[i] = frame;
    values[i].x = x;
    values[i].y = y;
    values[i].z = z;
    values[i].w = w;
    flags[i] = 0;
    count++;
    return i;
}

// Factor applied to the margin in View_ContainsPoint.
static const float VIEW_MARGIN_SCALE = 2.0f;

// Smaller and larger of two values; the arguments are evaluated twice, as in the original code.
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

// 1 when `point` transformed by `matrix` (a view-projection) falls inside the view volume in x and y, i.e.
// -limit <= x, y <= limit. In front of the camera (w > 0) the limit is w widened by 2 * margin * (w - min(w, z)), so
// points just off screen still count. Behind the camera the coordinates are negated; there the widening term
// w - max(w, z) is never positive, so it can only narrow the volume. Called by the flare and effect culling.
extern "C" int View_ContainsPoint(const Vec4 *point, const Matrix4 *matrix, float margin)
{
    float v[4];  // the transformed x, y, z, w
    v[0] = point->x * matrix->m[0] + point->y * matrix->m[4] + point->z * matrix->m[8] + matrix->m[12];
    v[1] = point->x * matrix->m[1] + point->y * matrix->m[5] + point->z * matrix->m[9] + matrix->m[13];
    v[2] = point->x * matrix->m[2] + point->y * matrix->m[6] + point->z * matrix->m[10] + matrix->m[14];
    v[3] = point->x * matrix->m[3] + point->y * matrix->m[7] + point->z * matrix->m[11] + matrix->m[15];
    if (v[3] > 0.0f)
    {
        float slack = MAX(v[3] - MIN(v[3], v[2]), 0.0f);
        float limit = margin * VIEW_MARGIN_SCALE * slack + v[3];
        if (v[0] < -limit || v[0] > limit || v[1] < -limit || v[1] > limit)
            return 0;
        return 1;
    }
    float w = -v[3];
    float z = -v[2];
    float slack = MIN(w - MAX(w, z), 0.0f);
    float limit = slack * margin - v[3];
    if (-v[0] < -limit || -v[0] > limit || -v[1] < -limit || -v[1] > limit)
        return 0;
    return 1;
}

// Squared length of the x/y/z part of `v`.
static float LengthSquaredXyz(const Vec4 &v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

// Squared distance from this point to the segment a-b (x/y/z): the point is projected onto the line and the
// projection is clamped to the segment's ends. A segment of zero length counts as the point a.
float Vec4::DistanceSquaredToSegment(const Vec4 &a, const Vec4 &b)
{
    Vec4 direction = b - a;
    if (direction.x == 0.0f && direction.y == 0.0f && direction.z == 0.0f)
        return LengthSquaredXyz(a - *this);
    float t = (Vec3_Dot(this, &direction) - Vec3_Dot(&a, &direction)) / LengthSquaredXyz(direction);
    if (t < 0.0f)
        return LengthSquaredXyz(a - *this);
    if (t > 1.0f)
        return LengthSquaredXyz(b - *this);
    return LengthSquaredXyz(a + direction * t - *this);
}

// Distance from this point to the segment a-b (x/y/z); the closest point of the segment is stored in `closest`.
// Unlike DistanceSquaredToSegment a segment of zero length is not special-cased. Called by the collision tests.
float Vec4::DistanceToSegment(const Vec4 &a, const Vec4 &b, Vec4 &closest)
{
    Vec4 direction = b - a;
    float t = (Vec3_Dot(this, &direction) - Vec3_Dot(&a, &direction)) / LengthSquaredXyz(direction);
    if (t < 0.0f)
    {
        closest.Copy(a);
        return Distance(a);
    }
    if (t > 1.0f)
    {
        closest.Copy(b);
        return Distance(b);
    }
    closest.Copy(a + direction * t);
    return Distance(closest);
}

// Grows the three key arrays by `extra` entries, keeping their contents. Called when a track runs out of room.
void KeyTrack::Reserve(int extra)
{
    int *newFrames = new int[capacity + extra];
    Vec4 *newValues = new Vec4[capacity + extra];
    char *newFlags = new char[capacity + extra];
    if (capacity != 0)
    {
        memcpy(newFrames, frames, capacity * sizeof(int));
        memcpy(newValues, values, capacity * sizeof(Vec4));
        memcpy(newFlags, flags, capacity);
        free(frames);
        frames = newFrames;
        free(values);
        values = newValues;
        free(flags);
        flags = newFlags;
    }
    else
    {
        frames = newFrames;
        flags = newFlags;
        values = newValues;
    }
    capacity += extra;
}

// Sets component `index` (0 = x, 1 = y, 2 = z, 3 = w) of `v` to `value`; other indexes change nothing.
#define SET_COMPONENT(v, index, value)     if ((index) == 0)                          (v).x = (value);                   else if ((index) == 1)                     (v).y = (value);                   else if ((index) == 2)                     (v).z = (value);                   else if ((index) == 3)                     (v).w = (value)

// Factor applied to the square root (twice the quaternion's scale).
static const float QUAT_SCALE = 2.0f;

// Quaternion threshold: with a trace of at least 1 the quaternion follows from the trace directly.
static const float QUAT_TRACE_LIMIT = 1.0f;

// Rotation quaternion of the rotation part of `matrix` (element [row][column] is m[row * 4 + column]). With a large
// trace the quaternion follows from the trace directly; otherwise from the largest diagonal element i and the two
// after it, j and k (Shoemake's method).
Vec4 QuatFromMatrix(const Matrix4 &matrix)
{
    Vec4 result;
    float s = matrix.m[0] + matrix.m[5] + matrix.m[10] + matrix.m[15];
    if (s >= QUAT_TRACE_LIMIT)
    {
        s = (float)sqrt(s);
        s = s * QUAT_SCALE;
        result.w = s / 4.0f;
        result.x = (matrix.m[9] - matrix.m[6]) / s;
        result.y = (matrix.m[2] - matrix.m[8]) / s;
        result.z = (matrix.m[4] - matrix.m[1]) / s;
    }
    else
    {
        int i = matrix.m[0] > matrix.m[5] ? 0 : 1;
        if (matrix.m[10] > matrix.m[i * 5])
            i = 2;
        int j = (i + 1) % 3;
        int k = (j + 1) % 3;
        s = (float)sqrt(matrix.m[i * 5] - matrix.m[j * 5] - matrix.m[k * 5] + 1.0f);
        s = s * QUAT_SCALE;
        SET_COMPONENT(result, i, s / 4.0f);
        SET_COMPONENT(result, j, (matrix.m[i * 4 + j] + matrix.m[j * 4 + i]) / s);
        SET_COMPONENT(result, k, (matrix.m[i * 4 + k] + matrix.m[k * 4 + i]) / s);
        result.w = (matrix.m[k * 4 + j] - matrix.m[j * 4 + k]) / s;
    }
    return result;
}

// Cosine of the angle from which Slerp falls back to the cheaper normalised lerp (the quaternions are nearly equal).
static const float SLERP_NLERP_COSINE = 0.95f;

// Interpolates between `a` and `b` (all four components) by `t` and normalises the result to unit length (a zero
// vector is left unchanged): a normalised lerp, used by Slerp for nearly equal rotations.
static Vec4 Nlerp(const Vec4 &a, const Vec4 &b, float t)
{
    Vec4 result;
    result.x = (b.x - a.x) * t + a.x;
    result.y = (b.y - a.y) * t + a.y;
    result.z = (b.z - a.z) * t + a.z;
    result.w = (b.w - a.w) * t + a.w;
    float length = (float)sqrt(result.y * result.y + result.x * result.x + result.z * result.z + result.w * result.w);
    if (length > 0.0f)
    {
        result.x /= length;
        result.y /= length;
        result.z /= length;
        result.w /= length;
    }
    return result;
}

// Spherical interpolation between the rotation quaternions `a` and `b` by `t`. `b` is flipped when the two point into
// opposite hemispheres, so the shorter way round is taken; when they are nearly equal the cheaper Nlerp is used.
Vec4 Slerp(const Vec4 &a, const Vec4 &b, float t)
{
    Vec4 target;
    float cosAngle = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    if (cosAngle < 0.0f)
    {
        cosAngle = -cosAngle;
        target.x = b.x * -1.0f;
        target.y = b.y * -1.0f;
        target.z = b.z * -1.0f;
        target.w = b.w * -1.0f;
    }
    else
    {
        target.Copy(b);
    }
    if (cosAngle < SLERP_NLERP_COSINE)
    {
        float angle = (float)acos(cosAngle);
        float sinAngle = (float)sin(angle);
        float weightB = (float)sin(angle * t);
        float weightA = (float)sin((1.0f - t) * angle);
        Vec4 result;
        result.x = (weightA * a.x + weightB * target.x) / sinAngle;
        result.y = (weightA * a.y + weightB * target.y) / sinAngle;
        result.z = (weightA * a.z + weightB * target.z) / sinAngle;
        result.w = (weightB * target.w + weightA * a.w) / sinAngle;
        return result;
    }
    return Nlerp(a, target, t);
}

// Destructor: nothing to release. Declared out of line so that constructors of owning types register an unwind frame.
Matrix4::~Matrix4()
{
}
