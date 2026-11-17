#include <math.h>

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    float M_101ac500();
    float M_101ac540(Vec4 other);
    float M_101ac6a0();
    Vec4();
    int M_101ac7c0(const struct Box2 *box);
    float M_101ac650(const Vec4 &other);
    float M_101ac590(const Vec4 &other);
    float M_101ac5f0(const Vec4 &other);
    Vec4 M_101acb00(const float &scale);
    Vec4 M_101acb60(const float &divisor);
};

struct Box2
{
    Vec4 min;
    Vec4 max;
};

struct Matrix4
{
    float m[16];
    void M_101adec0(float x, float y, float z);
    void M_101ae710(float angle);
    Matrix4 &M_101ae770();
};

inline float LengthSquared(float x, float y, float z)
{
    return x * x + y * y + z * z;
}

extern "C" float FUN_101ac220(const Vec3 *v)
{
    return LengthSquared(v->x, v->y, v->z);
}

float Vec4::M_101ac500()
{
    return (float)sqrt(x * x + y * y);
}

float Vec4::M_101ac540(Vec4 other)
{
    float dy = y - other.y;
    float dx = x - other.x;
    return (float)sqrt(dx * dx + dy * dy);
}

float Vec4::M_101ac6a0()
{
    return LengthSquared(x, y, z);
}

void Matrix4::M_101adec0(float x, float y, float z)
{
    m[12] += x;
    m[13] += y;
    m[14] += z;
}

Matrix4 &Matrix4::M_101ae770()
{
    m[12] = m[1] = m[14] = m[15] = m[8] = m[10] = m[11] = m[4] = m[6] = m[7] = m[0] = m[2] = m[3] = 0;
    return *this;
}

float Vec4::M_101ac650(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = other.z - z;
    return dz * dz + dy * dy + dx * dx;
}

float Vec4::M_101ac590(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = other.z - z;
    return (float)sqrt(dz * dz + dy * dy + dx * dx);
}

float Vec4::M_101ac5f0(const Vec4 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = (other.z - z) * 2;
    return (float)sqrt(dz * dz + dy * dy + dx * dx);
}

int Vec4::M_101ac7c0(const Box2 *box)
{
    if (x < box->min.x || x > box->max.x || y < box->min.y || y > box->max.y)
        return 0;
    return 1;
}

Vec4 Vec4::M_101acb00(const float &scale)
{
    Vec4 result;
    result.x = scale * x;
    result.y = y * scale;
    result.z = z * scale;
    return result;
}

Vec4 Vec4::M_101acb60(const float &divisor)
{
    Vec4 result;
    result.x = x / divisor;
    result.y = y / divisor;
    result.z = z / divisor;
    return result;
}

extern "C" float FUN_101ae890(const Vec3 *v)
{
    return (float)sqrt(FUN_101ac220(v));
}

void Matrix4::M_101ae710(float angle)
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
