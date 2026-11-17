// Vec4 constructors, setters and arithmetic operators (0x101ac990..0x101acc40). The default constructor zeroes all four
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
    void Fill(const float &value);
    Vec4 operator+(const Vec4 &other) const;
    Vec4 operator-(const Vec4 &other) const;
    Vec4 operator*(const Vec4 &other) const;
    Vec4 Scaled(const float &scale);
    Vec4 Divided(const float &divisor);
    Vec4 operator/(const int &divisor) const;
    float DistanceSquaredToSegment(const Vec4 &a, const Vec4 &b);
};

extern "C" float Vec3_Dot(const Vec4 *a, const Vec4 *b);       // 0x101ac200 (x/y/z only)
extern "C" float Vec3_LengthSquared(const Vec4 *v);            // 0x101ac220 (x/y/z only)

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
// Not byte-identical yet: the original's frame is 0x10 smaller (its last temporary reuses `direction`'s slot).
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
