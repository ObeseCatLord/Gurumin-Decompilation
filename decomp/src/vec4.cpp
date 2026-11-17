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
    Vec4 operator+(const Vec4 &other) const;
    Vec4 operator-(const Vec4 &other) const;
    Vec4 operator*(const Vec4 &other) const;
    Vec4 Scaled(const float &scale);
    Vec4 Divided(const float &divisor);
    Vec4 operator/(const int &divisor) const;
    float DistanceSquaredToSegment(const Vec4 &a, const Vec4 &b);
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
