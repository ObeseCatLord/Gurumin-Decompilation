// Vec4 constructors, setters and arithmetic operators (0x101ac990..0x101acc40), plus the vector helpers that call them.
// Placement matters: the compiler only reuses the stack slots of local vectors passed to Vec3_Dot/Vec3_LengthSquared
// when it can see those functions do not keep the pointer, so they live here too. The default constructor zeroes all four
// components. The operators that return a new vector construct it with Vec4() first; because Vec4() is defined in
// this file, the compiler knows it leaves edx alone and keeps `this` in edx across the call (this is why these
// operators only match when they live here).

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "vec4.h"
#include "key_track.h"

// Dot product of the x/y/z parts of two vectors (free function, cdecl; 0x101ac200).
extern "C" float Vec3_Dot(const Vec4 *a, const Vec4 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

// x*x + y*y + z*z: an inline helper whose float arguments are spilled right to left (see vec_math.cpp).
inline float LengthSquared(float x, float y, float z)
{
    return x * x + y * y + z * z;
}

// Squared length of the x/y/z part (free function, cdecl; 0x101ac220).
extern "C" float Vec3_LengthSquared(const Vec4 *v)
{
    return LengthSquared(v->x, v->y, v->z);
}

// 0x101ac4e0. Length of the x/y/z part.
float Vec4::Length()
{
    return (float)sqrt(Vec3_LengthSquared(this));
}

// Zero vector.
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

// Component-wise sum (all four components).
Vec4 Vec4::operator+(const Vec4 &other) const
{
    Vec4 result;
    result.x = other.x + x;
    result.y = other.y + y;
    result.z = other.z + z;
    result.w = other.w + w;
    return result;
}

// Component-wise difference (all four components).
Vec4 Vec4::operator-(const Vec4 &other) const
{
    Vec4 result;
    result.x = x - other.x;
    result.y = y - other.y;
    result.z = z - other.z;
    result.w = w - other.w;
    return result;
}

// Component-wise product (all four components).
Vec4 Vec4::operator*(const Vec4 &other) const
{
    Vec4 result;
    result.x = other.x * x;
    result.y = other.y * y;
    result.z = other.z * z;
    result.w = other.w * w;
    return result;
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

// Returns this vector divided by `divisor` (x/y/z; w stays 0).
Vec4 Vec4::Divided(const float &divisor)
{
    Vec4 result;
    result.x = x / divisor;
    result.y = y / divisor;
    result.z = z / divisor;
    return result;
}

// Returns this vector divided by an integer (x/y/z; w stays 0).
Vec4 Vec4::operator/(const int &divisor) const
{
    Vec4 result;
    float d = (float)divisor;
    result.x = x / d;
    result.y = y / d;
    result.z = z / d;
    return result;
}

// 0x101acc60. Squared distance on the x/y plane from this point to the segment (x1, y1)-(x2, y2): the point is
// projected onto the line and the projection is clamped to the ends. A segment of zero length counts as its first
// point. The three exits spell the squared distance out (the original is not a call to a shared helper).
float Vec4::DistanceSquaredToSegment2D(float x1, float y1, float x2, float y2)
{
    if (x1 == x2 && y1 == y2)
        return (x - x1) * (x - x1) + (y - y1) * (y - y1);
    float dx = x2 - x1;
    float dy = y2 - y1;
    float t = (dx * x + (dy * y - (dy * y1 + dx * x1))) / (dx * dx + dy * dy);
    if (t < 0.0f)
        return (x - x1) * (x - x1) + (y - y1) * (y - y1);
    if (t > 1.0f)
        return (x - x2) * (x - x2) + (y - y2) * (y - y2);
    return (x - (t * dx + x1)) * (x - (t * dx + x1)) + (y - (t * dy + y1)) * (y - (t * dy + y1));
}

// Squared distance from this point to the segment a-b (x/y/z): the point is projected onto the line and the
// projection is clamped to the segment's ends. A segment of zero length counts as the point a.
float Vec4::DistanceSquaredToSegment(const Vec4 &a, const Vec4 &b)
{
    Vec4 direction = b - a;
    if (direction.x == 0.0f && direction.y == 0.0f && direction.z == 0.0f)
        return Vec3_LengthSquared(&(a - *this));
    float t = (Vec3_Dot(this, &direction) - Vec3_Dot(&a, &direction)) / Vec3_LengthSquared(&direction);
    if (t < 0.0f)
        return Vec3_LengthSquared(&(a - *this));
    if (t > 1.0f)
        return Vec3_LengthSquared(&(b - *this));
    return Vec3_LengthSquared(&(a + direction * t - *this));
}

// Distance from this point to the segment a-b (x/y/z); the closest point of the segment is stored in `closest`.
float Vec4::DistanceToSegment(const Vec4 &a, const Vec4 &b, Vec4 &closest)
{
    Vec4 direction = b - a;
    float t = (Vec3_Dot(this, &direction) - Vec3_Dot(&a, &direction)) / Vec3_LengthSquared(&direction);
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
    return Vec3_Length(&(closest - *this));
}

// 0x101af140. Distance from this point to the infinite line through a and b (x/y/z): like DistanceToSegment,
// but the projection is not clamped to the segment's ends.
float Vec4::DistanceToLine(const Vec4 &a, const Vec4 &b)
{
    Vec4 direction = b - a;
    float t = (Vec3_Dot(this, &direction) - Vec3_Dot(&a, &direction)) / Vec3_LengthSquared(&direction);
    return Vec3_Length(&(a + direction * t - *this));
}

// 0x101af1d0. Squared distance from this point to the infinite line through a and b (x/y/z).
float Vec4::DistanceSquaredToLine(const Vec4 &a, const Vec4 &b)
{
    Vec4 direction = b - a;
    float t = (Vec3_Dot(this, &direction) - Vec3_Dot(&a, &direction)) / Vec3_LengthSquared(&direction);
    return Vec3_LengthSquared(&(a + direction * t - *this));
}

// 0x101aeee0. 1 when this point, projected onto the plane of triangle a-b-c, lies strictly inside the triangle
// (x/y/z). Uses barycentric coordinates without dividing by the denominator: u and v must be positive and u + v
// below the denominator. Used by the collision code (0x1019f340, 0x101a0cf0, 0x101a1640).
int Vec4::InTriangle(const Vec4 &a, const Vec4 &b, const Vec4 &c)
{
    Vec4 ab = b - a;
    Vec4 ac = c - a;
    Vec4 ap = *this - a;
    float u = Vec3_Dot(&ac, &ac) * Vec3_Dot(&ap, &ab) - Vec3_Dot(&ac, &ab) * Vec3_Dot(&ap, &ac);
    if (!(u > 0.0f))
        return 0;
    float v = Vec3_Dot(&ab, &ab) * Vec3_Dot(&ap, &ac) - Vec3_Dot(&ab, &ac) * Vec3_Dot(&ap, &ab);
    if (!(v > 0.0f))
        return 0;
    float denominator = Vec3_Dot(&ab, &ab) * Vec3_Dot(&ac, &ac) - Vec3_Dot(&ab, &ac) * Vec3_Dot(&ab, &ac);
    if (!(u + v - denominator < 0.0f))
        return 0;
    return 1;
}

#include "matrix4.h"

// 0x101ade80. Returns the translation row as a vector (w stays 0).
Vec4 Matrix4::GetTranslation()
{
    Vec4 result;
    result.x = m[12];
    result.y = m[13];
    result.z = m[14];
    return result;
}

// Transforms `point` (as x, y, z, 1) by `matrix` and divides by the resulting w (perspective projection). When w is
// not positive (behind the camera) the result is negated, so callers can still tell the side.
Vec4 TransformCoord(const Vec4 &point, const Matrix4 &matrix)
{
    float v[4];  // the transformed x, y, z, w
    v[0] = point.x * matrix.m[0] + point.y * matrix.m[4] + point.z * matrix.m[8] + matrix.m[12];
    v[1] = point.x * matrix.m[1] + point.y * matrix.m[5] + point.z * matrix.m[9] + matrix.m[13];
    v[2] = point.x * matrix.m[2] + point.y * matrix.m[6] + point.z * matrix.m[10] + matrix.m[14];
    v[3] = point.x * matrix.m[3] + point.y * matrix.m[7] + point.z * matrix.m[11] + matrix.m[15];
    Vec4 result;
    if (v[3] > 0.0f)
    {
        result.x = v[0] / v[3];
        result.y = v[1] / v[3];
        result.z = v[2] / v[3];
        result.w = v[3];
    }
    else
    {
        result.x = -(v[0] / v[3]);
        result.y = -(v[1] / v[3]);
        result.z = -(v[2] / v[3]);
        result.w = -v[3];
    }
    return result;
}

// 0x101ae060. Transforms `point` by `matrix` and divides x, y and z by the resulting w (perspective divide); w is
// kept. Unlike TransformCoord there is no sign fix for points behind the camera.
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

// Sets component `index` (0 = x, 1 = y, 2 = z) of the vector `v` to `value`.
#define SET_XYZ(v, index, value)      if ((index) == 0)                      (v).x = (value);               else if ((index) == 1)                 (v).y = (value);               else if ((index) == 2)                 (v).z = (value)

// 0x101ae2a0. Rotation quaternion (x, y, z = vector part, w = scalar part) of the rotation part of `matrix`
// (element [row][column] is m[row * 4 + column]). With a large trace the quaternion follows from the trace
// directly; otherwise from the largest diagonal element i and the two after it, j and k (Shoemake's method).
Vec4 QuatFromMatrix(const Matrix4 &matrix)
{
    Vec4 result;
    float s = matrix.m[0] + matrix.m[5] + matrix.m[10] + matrix.m[15];
    if (s >= 1.0f)
    {
        s = (float)sqrt(s);
        s = s + s;
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
        s = s + s;
        SET_XYZ(result, i, s / 4.0f);
        SET_XYZ(result, j, (matrix.m[i * 4 + j] + matrix.m[j * 4 + i]) / s);
        SET_XYZ(result, k, (matrix.m[i * 4 + k] + matrix.m[k * 4 + i]) / s);
        result.w = (matrix.m[k * 4 + j] - matrix.m[j * 4 + k]) / s;
    }
    return result;
}

#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

// 0x101ae4d0. 1 when `point` transformed by `matrix` (a view-projection) falls inside the view volume in x and y,
// i.e. -limit <= x, y <= limit. In front of the camera (w > 0) the limit is w widened by 2 * margin * (w - min(w, z)),
// so points just off screen still count (AddFlarePoint passes 24). Behind the camera the coordinates are negated;
// there the widening term w - max(w, z) is never positive, so it can only narrow the volume. The MIN/MAX macros
// evaluate their arguments twice, as the original code does.
extern "C" int FUN_101ae4d0(const Vec4 *point, const Matrix4 *matrix, float margin)
{
    float v[4];  // the transformed x, y, z, w
    v[0] = point->x * matrix->m[0] + point->y * matrix->m[4] + point->z * matrix->m[8] + matrix->m[12];
    v[1] = point->x * matrix->m[1] + point->y * matrix->m[5] + point->z * matrix->m[9] + matrix->m[13];
    v[2] = point->x * matrix->m[2] + point->y * matrix->m[6] + point->z * matrix->m[10] + matrix->m[14];
    v[3] = point->x * matrix->m[3] + point->y * matrix->m[7] + point->z * matrix->m[11] + matrix->m[15];
    if (v[3] > 0.0f)
    {
        float slack = MAX(v[3] - MIN(v[3], v[2]), 0.0f);
        float limit = slack * margin + slack * margin + v[3];
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

// 0x101af420. Constructor: constructs the 0x80 key values and empties the bank.
KeyBank::KeyBank()
{
    Clear();
}


// Grows the three key arrays by `extra` entries, keeping their contents. Returns nothing; the capacity grows.
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
    values[i].x = x;
    values[i].y = y;
    values[i].z = z;
    values[i].w = w;
    frames[i] = frame;
    flags[i] = 0;
    count++;
    return i;
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
    values[i].x = x;
    values[i].y = y;
    values[i].z = z;
    values[i].w = w;
    frames[i] = frame;
    flags[i] = 0;
    count++;
    return i;
}

// 0x101ae950. Rotation quaternion for `angle` radians about `axis` (x, y, z = vector part, w = scalar part),
// normalised (the sum of squares starts with z). Note that `axis` itself is scaled by sin(angle / 2) on the way.
Vec4 QuatFromAxisAngle(Vec4 &axis, float angle)
{
    Vec4 result;
    float half = angle / 2.0f;
    float s = (float)sin(half);
    axis.x = s * axis.x;
    axis.y = s * axis.y;
    axis.z = s * axis.z;
    angle = (float)cos(half);                           // cos(half) is kept in the parameter's slot
    float length = (float)sqrt(axis.z * axis.z + axis.y * axis.y + axis.x * axis.x + angle * angle);
    result.w = angle / length;
    result.x = axis.x / length;
    result.y = axis.y / length;
    result.z = axis.z / length;
    return result;
}

// Difference a - b between two angles in radians, taking the shorter way round. Both angles are first folded into
// -pi..pi (in steps of pi, as the game does; pi = 3.1415), then the differences a - b and a - b +- 2pi are compared.
float AngleDifference(float a, float b)
{
    while (a > 3.1415f)
        a -= 3.1415f;
    while (a < -3.1415f)
        a += 3.1415f;
    while (b > 3.1415f)
        b -= 3.1415f;
    while (b < -3.1415f)
        b += 3.1415f;
    float difference = a - b;
    float up = a + 6.283f - b;
    float down = a - 6.283f - b;
    if (fabs(difference) > fabs(up))
        difference = up;
    if (fabs(difference) > fabs(down))
        difference = down;
    return difference;
}

// Mirrors this vector at the plane through the origin with normal `normal`: the vector is turned so that the normal
// lies along an axis, that component is negated, and the vector is turned back.
void Vec4::Mirror(const Vec4 &normal)
{
    if (normal.x != 0.0f)
    {
        float yaw = -(float)atan(normal.y / normal.x);
        float pitch = -(float)atan(normal.z / sqrt(normal.y * normal.y + normal.x * normal.x));
        RotateZ(yaw);
        RotateY(-pitch);
        x = -x;
        RotateY(pitch);
        RotateZ(-yaw);
        return;
    }
    float roll = -(float)atan(normal.y / normal.z);
    float pitch = -(float)atan(normal.x / sqrt(normal.z * normal.z + normal.y * normal.y));
    RotateX(-roll);
    RotateY(-pitch);
    z = -z;
    RotateZ(pitch);
    RotateZ(roll);
}

// Cubic Hermite interpolation between `b` (t = 0) and `c` (t = 1) with neighbours `a` and `d`. The tangents are
// (b - a) / 2 + (c - b) at b and (c - b) / 2 + (d - c) at c (not the usual Catmull-Rom halves of both differences).
// Not byte-identical yet: cl 13.00 rewrites t3 * -2 as -(t3 + t3) and shares it with the b term (see open.tsv).
// A member of the data-less base KeyInterpolator of KeyTrack and KeyBank (include/key_track.h): every caller loads
// ecx with its own key container first (`mov ecx, edx`), and Spline itself never reads `this`.
float KeyInterpolator::Spline(float t, float a, float b, float c, float d)
{
    float bc = c - b;
    float tangentB = (b - a) * 0.5f + bc;
    float tangentC = (d - c) + bc * 0.5f;
    float t2 = t * t;
    float t3 = t2 * t;
    float t2x3 = t2 * 3.0f;
    return (t3 - (t2 + t2) + t) * tangentB + (t3 + t3 - t2x3 + 1.0f) * b + (t3 * -2.0f + t2x3) * c +
           (t3 - t2) * tangentC;
}

Vec4 Slerp(const Vec4 &a, const Vec4 &b, float t);    // 0x101aeae0 (vec_math.cpp)

// Key indices clamped to the bank: not before the first key, not past the last key (`last` = count - 1).
#define AT_LEAST_FIRST(index) ((index) < 0 ? 0 : (index))
#define AT_MOST_LAST(index, last) ((index) > (last) ? (last) : (index))

// The value of the bank at `frame`, written to x/y/z/w. A key exactly at `frame` is copied (returns 2). Before the
// first key and past the last key the first/last value is held. Between two keys: with `rotation` set the values are
// quaternions blended with Slerp; otherwise the later key's flags choose: KEY_FLAG_HOLD keeps the earlier value,
// KEY_FLAG_MOTION (linear) interpolates in a straight line, and anything else follows a Spline through the two
// keys and their outer neighbours (clamped to the bank). Returns 1 unless a key sat exactly at `frame`.
// Called by the motion channels, effect models, camera paths and the world's tables.
int KeyBank::Sample(int frame, float *x, float *y, float *z, float *w, int rotation)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (frames[i] == frame)
        {
            *x = values[i].x;
            *y = values[i].y;
            *z = values[i].z;
            *w = values[i].w;
            return 2;
        }
        if (frames[i] > frame)
            break;
    }
    if (i == 0)
    {
        *x = values[0].x;
        *y = values[0].y;
        *z = values[0].z;
        *w = values[0].w;
        return 1;
    }
    if (i == count)
    {
        *x = values[i - 1].x;
        *y = values[i - 1].y;
        *z = values[i - 1].z;
        *w = values[i - 1].w;
        return 1;
    }

    // Between key i - 1 and key i.
    int span = frames[i] - frames[i - 1];
    int offset = frame - frames[i - 1];
    if (rotation != 0)
    {
        Vec4 blended;
        blended.Copy(Slerp(values[i - 1], values[i], (float)offset / (float)span));
        *x = blended.x;
        *y = blended.y;
        *z = blended.z;
        *w = blended.w;
        return 1;
    }

    int keyFlags = flags[AT_LEAST_FIRST(i)];
    int linear = (keyFlags >> 2) & 1;     // KEY_FLAG_MOTION
    if (keyFlags & KEY_FLAG_HOLD)
    {
        *x = values[i - 1].x;
        *y = values[i - 1].y;
        *z = values[i - 1].z;
        *w = values[i - 1].w;
        return 1;
    }
    if (linear)
    {
        float along = (float)offset;
        float length = (float)span;
        *x = (values[i].x - values[i - 1].x) * along / length + values[i - 1].x;
        *y = (values[i].y - values[i - 1].y) * along / length + values[i - 1].y;
        *z = (values[i].z - values[i - 1].z) * along / length + values[i - 1].z;
        *w = (values[i].w - values[i - 1].w) * along / length + values[i - 1].w;
        return 1;
    }

    // Spline through keys i - 2 .. i + 1, clamped to the first and last key. The blend factor is written inside
    // each call (cl 13.00 evaluates it once, after the key indices); a separate `t` statement makes it convert
    // `offset` before the linear/spline branch instead.
#define SPLINE_T ((float)offset / (float)span)
    *x = this->Spline(SPLINE_T,
                values[AT_LEAST_FIRST(i - 1 - 1)].x, values[AT_LEAST_FIRST(i - 1)].x, values[AT_MOST_LAST(i, count - 1)].x,
                values[AT_MOST_LAST(i + 1, count - 1)].x);
    *y = this->Spline(SPLINE_T, values[AT_LEAST_FIRST(i - 1 - 1)].y, values[AT_LEAST_FIRST(i - 1)].y, values[AT_MOST_LAST(i, count - 1)].y,
                values[AT_MOST_LAST(i + 1, count - 1)].y);
    *z = this->Spline(SPLINE_T, values[AT_LEAST_FIRST(i - 1 - 1)].z, values[AT_LEAST_FIRST(i - 1)].z, values[AT_MOST_LAST(i, count - 1)].z,
                values[AT_MOST_LAST(i + 1, count - 1)].z);
    *w = this->Spline(SPLINE_T, values[AT_LEAST_FIRST(i - 1 - 1)].w, values[AT_LEAST_FIRST(i - 1)].w, values[AT_MOST_LAST(i, count - 1)].w,
                values[AT_MOST_LAST(i + 1, count - 1)].w);
#undef SPLINE_T
    return 1;
}

// 0x101afe00 (KeyTrack::Sample). KeyBank::Sample's lookup over a KeyTrack's separately allocated frame, value and flag
// arrays: a key exactly at `frame` is copied (returns 2); before the first key and past the last the first/last value is
// held; between two keys `rotation` blends quaternions, KEY_FLAG_HOLD holds, KEY_FLAG_MOTION is linear, else a Spline.
// Returns 1 unless a key sat exactly at `frame`. Used by scene nodes, camera paths and the motion channels.
int KeyTrack::Sample(int frame, float *x, float *y, float *z, float *w, int rotation)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (frames[i] == frame)
        {
            *x = values[i].x;
            *y = values[i].y;
            *z = values[i].z;
            *w = values[i].w;
            return 2;
        }
        if (frames[i] > frame)
            break;
    }
    if (i == 0)
    {
        *x = values[0].x;
        *y = values[0].y;
        *z = values[0].z;
        *w = values[0].w;
        return 1;
    }
    if (i == count)
    {
        *x = values[i - 1].x;
        *y = values[i - 1].y;
        *z = values[i - 1].z;
        *w = values[i - 1].w;
        return 1;
    }

    // Between key i - 1 and key i.
    int span = frames[i] - frames[i - 1];
    int offset = frame - frames[i - 1];
    if (rotation != 0)
    {
        Vec4 blended;
        blended.Copy(Slerp(values[i - 1], values[i], (float)offset / (float)span));
        *x = blended.x;
        *y = blended.y;
        *z = blended.z;
        *w = blended.w;
        return 1;
    }

    int keyFlags = flags[AT_LEAST_FIRST(i)];
    int linear = (keyFlags >> 2) & 1;     // KEY_FLAG_MOTION
    if (keyFlags & KEY_FLAG_HOLD)
    {
        *x = values[i - 1].x;
        *y = values[i - 1].y;
        *z = values[i - 1].z;
        *w = values[i - 1].w;
        return 1;
    }
    if (linear)
    {
        float along = (float)offset;
        float length = (float)span;
        *x = (values[i].x - values[i - 1].x) * along / length + values[i - 1].x;
        *y = (values[i].y - values[i - 1].y) * along / length + values[i - 1].y;
        *z = (values[i].z - values[i - 1].z) * along / length + values[i - 1].z;
        *w = (values[i].w - values[i - 1].w) * along / length + values[i - 1].w;
        return 1;
    }

    // Spline through keys i - 2 .. i + 1, clamped to the first and last key. The blend factor is written inside
    // each call (cl 13.00 evaluates it once, after the key indices); a separate `t` statement makes it convert
    // `offset` before the linear/spline branch instead.
#define SPLINE_T ((float)offset / (float)span)
    *x = this->Spline(SPLINE_T,
                values[AT_LEAST_FIRST(i - 2)].x, values[AT_LEAST_FIRST(i - 1)].x, values[AT_MOST_LAST(i, count - 1)].x,
                values[AT_MOST_LAST(i + 1, count - 1)].x);
    *y = this->Spline(SPLINE_T, values[AT_LEAST_FIRST(i - 2)].y, values[AT_LEAST_FIRST(i - 1)].y, values[AT_MOST_LAST(i, count - 1)].y,
                values[AT_MOST_LAST(i + 1, count - 1)].y);
    *z = this->Spline(SPLINE_T, values[AT_LEAST_FIRST(i - 2)].z, values[AT_LEAST_FIRST(i - 1)].z, values[AT_MOST_LAST(i, count - 1)].z,
                values[AT_MOST_LAST(i + 1, count - 1)].z);
    *w = this->Spline(SPLINE_T, values[AT_LEAST_FIRST(i - 2)].w, values[AT_LEAST_FIRST(i - 1)].w, values[AT_MOST_LAST(i, count - 1)].w,
                values[AT_MOST_LAST(i + 1, count - 1)].w);
#undef SPLINE_T
    return 1;
}

// 0x101af960 (KeyBank::Sample, float frame). KeyBank::Sample for a fractional frame: the keys' frames are compared as
// floats and the blend factor is offset / span in float. Returns 1 unless a key sat exactly at `frame`.
int KeyBank::Sample(float frame, float *x, float *y, float *z, float *w, int rotation)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if ((float)frames[i] == frame)
        {
            *x = values[i].x;
            *y = values[i].y;
            *z = values[i].z;
            *w = values[i].w;
            return 2;
        }
        if ((float)frames[i] > frame)
            break;
    }
    if (i == 0)
    {
        *x = values[0].x;
        *y = values[0].y;
        *z = values[0].z;
        *w = values[0].w;
        return 1;
    }
    if (i == count)
    {
        *x = values[i - 1].x;
        *y = values[i - 1].y;
        *z = values[i - 1].z;
        *w = values[i - 1].w;
        return 1;
    }

    // Between key i - 1 and key i.
    float span = (float)(frames[i] - frames[i - 1]);
    float offset = frame - (float)frames[i - 1];
    if (rotation != 0)
    {
        Vec4 blended;
        blended.Copy(Slerp(values[i - 1], values[i], offset / span));
        *x = blended.x;
        *y = blended.y;
        *z = blended.z;
        *w = blended.w;
        return 1;
    }

    int keyFlags = flags[AT_LEAST_FIRST(i)];
    int linear = (keyFlags >> 2) & 1;     // KEY_FLAG_MOTION
    if (keyFlags & KEY_FLAG_HOLD)
    {
        *x = values[i - 1].x;
        *y = values[i - 1].y;
        *z = values[i - 1].z;
        *w = values[i - 1].w;
        return 1;
    }
    if (linear)
    {
        float along = offset;
        float length = span;
        *x = (values[i].x - values[i - 1].x) * along / length + values[i - 1].x;
        *y = (values[i].y - values[i - 1].y) * along / length + values[i - 1].y;
        *z = (values[i].z - values[i - 1].z) * along / length + values[i - 1].z;
        *w = (values[i].w - values[i - 1].w) * along / length + values[i - 1].w;
        return 1;
    }

    // Spline through keys i - 2 .. i + 1, clamped to the first and last key. The blend factor is written inside
    // each call (cl 13.00 evaluates it once, after the key indices); a separate `t` statement makes it convert
    // `offset` before the linear/spline branch instead.
#define SPLINE_T (offset / span)
    *x = this->Spline(SPLINE_T,
                values[AT_LEAST_FIRST(i - 2)].x, values[AT_LEAST_FIRST(i - 1)].x, values[AT_MOST_LAST(i, count - 1)].x,
                values[AT_MOST_LAST(i + 1, count - 1)].x);
    *y = this->Spline(SPLINE_T, values[AT_LEAST_FIRST(i - 2)].y, values[AT_LEAST_FIRST(i - 1)].y, values[AT_MOST_LAST(i, count - 1)].y,
                values[AT_MOST_LAST(i + 1, count - 1)].y);
    *z = this->Spline(SPLINE_T, values[AT_LEAST_FIRST(i - 2)].z, values[AT_LEAST_FIRST(i - 1)].z, values[AT_MOST_LAST(i, count - 1)].z,
                values[AT_MOST_LAST(i + 1, count - 1)].z);
    *w = this->Spline(SPLINE_T, values[AT_LEAST_FIRST(i - 2)].w, values[AT_LEAST_FIRST(i - 1)].w, values[AT_MOST_LAST(i, count - 1)].w,
                values[AT_MOST_LAST(i + 1, count - 1)].w);
#undef SPLINE_T
    return 1;
}
