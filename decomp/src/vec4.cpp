// Vec4 constructors, setters and arithmetic operators (0x101ac990..0x101acc40). The default constructor zeroes all four
// components. The operators that return a new vector construct it with Vec4() first; because Vec4() is defined in
// this file, the compiler knows it leaves edx alone and keeps `this` in edx across the call (this is why these
// operators only match when they live here).

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
};

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
