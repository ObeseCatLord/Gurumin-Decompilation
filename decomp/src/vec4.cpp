// Vec4 constructors, setters and arithmetic operators (0x101ac990..0x101acc40), plus the vector helpers that call them.
// Placement matters: the compiler only reuses the stack slots of local vectors passed to Vec3_Dot/Vec3_LengthSquared
// when it can see those functions do not keep the pointer, so they live here too. The default constructor zeroes all four
// components. The operators that return a new vector construct it with Vec4() first; because Vec4() is defined in
// this file, the compiler knows it leaves edx alone and keeps `this` in edx across the call (this is why these
// operators only match when they live here).

#include <stdlib.h>
#include <string.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;

    Vec4();
    Vec4(float x, float y, float z, float w);
    void Set(float x, float y, float z, float w);
    void Copy(const Vec4 &source);
    void Fill(const float &value);
    float Distance(const Vec4 &other);   // 0x101ac590 (vec_math.cpp)
    float DistanceToSegment(const Vec4 &a, const Vec4 &b, Vec4 &closest);
    void RotateX(float angle);   // 0x101ac440 (vec_math.cpp)
    void RotateY(float angle);   // 0x101ac3a0
    void RotateZ(float angle);   // 0x101ac300
    void Mirror(const Vec4 &normal);
    Vec4 operator+(const Vec4 &other) const;
    Vec4 operator-(const Vec4 &other) const;
    Vec4 operator*(const Vec4 &other) const;
    Vec4 Scaled(const float &scale);
    Vec4 Divided(const float &divisor);
    Vec4 operator/(const int &divisor) const;
    float DistanceSquaredToSegment(const Vec4 &a, const Vec4 &b);
    float DistanceSquaredToSegment2D(float x1, float y1, float x2, float y2);
    float DistanceToLine(const Vec4 &a, const Vec4 &b);
    float DistanceSquaredToLine(const Vec4 &a, const Vec4 &b);
};

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
extern "C" float Vec3_Length(const Vec4 *v);                   // 0x101ae890 (x/y/z only)

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
Vec4 Vec4::Scaled(const float &scale)
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

// Squared distance between the points (ax, ay) and (bx, by).
inline float DistanceSquared2D(float ax, float ay, float bx, float by)
{
    float dy = ay - by;
    float dx = ax - bx;
    return dx * dx + dy * dy;
}

// Squared distance on the x/y plane from this point to the segment (x1, y1)-(x2, y2): the point is projected onto
// the line and the projection is clamped to the ends. A segment of zero length counts as its first point.
// Not byte-identical yet: the shared "first point" exit is placed in the middle in the original (see open.tsv).
float Vec4::DistanceSquaredToSegment2D(float x1, float y1, float x2, float y2)
{
    if (x1 == x2 && y1 == y2)
        return DistanceSquared2D(x, y, x1, y1);
    float dx = x2 - x1;
    float dy = y2 - y1;
    float t = (dx * x + (dy * y - (dy * y1 + dx * x1))) / (dx * dx + dy * dy);
    if (t < 0.0f)
        return DistanceSquared2D(x, y, x1, y1);
    if (t > 1.0f)
        return DistanceSquared2D(x, y, x2, y2);
    return DistanceSquared2D(x, y, t * dx + x1, t * dy + y1);
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
    return Vec3_LengthSquared(&(a + direction.Scaled(t) - *this));
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
    closest.Copy(a + direction.Scaled(t));
    return Vec3_Length(&(closest - *this));
}

// 0x101af140. Distance from this point to the infinite line through a and b (x/y/z): like DistanceToSegment,
// but the projection is not clamped to the segment's ends.
float Vec4::DistanceToLine(const Vec4 &a, const Vec4 &b)
{
    Vec4 direction = b - a;
    float t = (Vec3_Dot(this, &direction) - Vec3_Dot(&a, &direction)) / Vec3_LengthSquared(&direction);
    return Vec3_Length(&(a + direction.Scaled(t) - *this));
}

// 0x101af1d0. Squared distance from this point to the infinite line through a and b (x/y/z).
float Vec4::DistanceSquaredToLine(const Vec4 &a, const Vec4 &b)
{
    Vec4 direction = b - a;
    float t = (Vec3_Dot(this, &direction) - Vec3_Dot(&a, &direction)) / Vec3_LengthSquared(&direction);
    return Vec3_LengthSquared(&(a + direction.Scaled(t) - *this));
}

// Row-major 4x4 matrix (vec_math.cpp); the translation is in m[12..14].
struct Matrix4
{
    float m[16];
    Vec4 GetTranslation();
};

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

// Fixed key-frame bank (containers.cpp). Its constructor lives here, next to Vec4(), for the same reason as the
// operators above: the compiler keeps the array pointer in edx across the Vec4() calls of its value array.
struct KeyBank
{
    int m_0000[0x80];
    Vec4 m_0200[0x80];
    char flags[0x80];
    int m_0a80;
    KeyBank();
    void Clear();  // 0x101ad730
    int Insert(int frame, float x, float y, float z, float w);
    int Set(int frame, float x, float y, float z, float w);
};

// 0x101af420. Constructor: constructs the 0x80 key values and empties the bank.
KeyBank::KeyBank()
{
    Clear();
}

// Growable key-frame track (containers.cpp). Reserve lives here because its `new Vec4[]` loop keeps the element
// pointer in edx across the Vec4() calls.
struct KeyTrack
{
    int capacity;
    int *frames;
    Vec4 *values;
    char *flags;
    int count;
    void Reserve(int extra);
};

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
    for (i = 0; i < m_0a80; i++)
    {
        if (m_0000[i] == frame)
            break;
        if (m_0000[i] > frame)
            break;
    }
    for (int j = m_0a80 - 1; j >= i; j--)
    {
        m_0000[j + 1] = m_0000[j];
        m_0200[j + 1].Copy(m_0200[j]);
        flags[j + 1] = flags[j];
    }
    m_0200[i].x = x;
    m_0200[i].y = y;
    m_0200[i].z = z;
    m_0200[i].w = w;
    m_0000[i] = frame;
    flags[i] = 0;
    m_0a80++;
    return i;
}

// Sets the key at `frame` to (x, y, z, w): an existing key at that frame gets the new value (returns 1); otherwise a
// new key is inserted in frame order like Insert (returns its index).
int KeyBank::Set(int frame, float x, float y, float z, float w)
{
    int i;
    for (i = 0; i < m_0a80; i++)
    {
        if (m_0000[i] == frame)
        {
            m_0200[i].x = x;
            m_0200[i].y = y;
            m_0200[i].z = z;
            m_0200[i].w = w;
            return 1;
        }
        if (m_0000[i] > frame)
            break;
    }
    for (int j = m_0a80 - 1; j >= i; j--)
    {
        m_0000[j + 1] = m_0000[j];
        m_0200[j + 1].Copy(m_0200[j]);
        flags[j + 1] = flags[j];
    }
    m_0200[i].x = x;
    m_0200[i].y = y;
    m_0200[i].z = z;
    m_0200[i].w = w;
    m_0000[i] = frame;
    flags[i] = 0;
    m_0a80++;
    return i;
}

#include <math.h>

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

// x*x + y*y with the arguments spilled like the other inline length helpers.
inline float LengthSquared2(float x, float y)
{
    return x * x + y * y;
}

// Mirrors this vector at the plane through the origin with normal `normal`: the vector is turned so that the normal
// lies along an axis, that component is negated, and the vector is turned back.
void Vec4::Mirror(const Vec4 &normal)
{
    if (normal.x != 0.0f)
    {
        float yaw = -(float)atan(normal.y / normal.x);
        float pitch = -(float)atan(normal.z / sqrt(LengthSquared2(normal.y, normal.x)));
        RotateZ(yaw);
        RotateY(-pitch);
        x = -x;
        RotateY(pitch);
        RotateZ(-yaw);
        return;
    }
    float roll = -(float)atan(normal.y / normal.z);
    float pitch = -(float)atan(normal.x / sqrt(LengthSquared2(normal.z, normal.y)));
    RotateX(-roll);
    RotateY(-pitch);
    z = -z;
    RotateZ(pitch);
    RotateZ(roll);
}
