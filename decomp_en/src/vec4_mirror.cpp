// Vec4 mirroring at a plane (EN build). Ported from the JP twin (APP.dll 0x101aed50).
#include <math.h>

#include "vec4.h"

// The EN compiler expands the three axis rotations inside Mirror; these file-local copies of Vec4::RotateX/Y/Z
// (vec4.cpp) give it the bodies to inline.
static inline void RotateAboutX(Vec4 &v, float angle)
{
    float c = (float)cos(angle);
    float negZ = -v.z;
    float s = (float)sin(angle);
    float ny = v.y * c - s * negZ;
    float nz = v.y * s + negZ * c;
    v.y = ny;
    v.z = -nz;
}

static inline void RotateAboutY(Vec4 &v, float angle)
{
    float c = (float)cos(angle);
    float negZ = -v.z;
    float s = (float)sin(angle);
    float nx = v.x * c - s * negZ;
    float nz = v.x * s + negZ * c;
    v.z = -nz;
    v.x = nx;
}

static inline void RotateAboutZ(Vec4 &v, float angle)
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    float nx = v.x * c - s * v.y;
    float ny = v.x * s + c * v.y;
    v.x = nx;
    v.y = ny;
}

// Mirrors this vector at the plane through the origin with normal `normal`: the vector is turned so that the normal
// lies along an axis, that component is negated, and the vector is turned back. Called by the reflection code that
// bounces moving objects off walls.
void Vec4::Mirror(const Vec4 &normal)
{
    if (normal.x != 0.0f)
    {
        float yaw = -(float)atan(normal.y / normal.x);
        float pitch = -(float)atan(normal.z / sqrt(normal.y * normal.y + normal.x * normal.x));
        RotateAboutZ(*this, yaw);
        RotateAboutY(*this, -pitch);
        x = -x;
        RotateAboutY(*this, pitch);
        RotateAboutZ(*this, -yaw);
        return;
    }
    float roll = -(float)atan(normal.y / normal.z);
    float pitch = -(float)atan(normal.x / sqrt(normal.z * normal.z + normal.y * normal.y));
    RotateAboutX(*this, -roll);
    RotateAboutY(*this, -pitch);
    z = -z;
    RotateAboutZ(*this, pitch);
    RotateAboutZ(*this, roll);
}
