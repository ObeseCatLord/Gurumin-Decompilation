// Rotation of a point about an axis through a quaternion (EN build). Ported from the Ghidra output and the EN bytes;
// the JP twins (APP.dll 0x101b16f0 and 0x101b1290) are not matched.
#include <math.h>

#include "vec4.h"

// Half of an angle (the quaternion of a rotation holds half the angle).
static const float HALF = 0.5f;

// Transforms `point` by `matrix` and divides x/y/z by the resulting w (the out-of-line Project at EN 0x77a4a0 is expanded
// in the functions of this file).
static inline Vec4 ProjectPoint(const Vec4 &point, const Matrix4 &matrix)
{
    float w = point.x * matrix.m[3] + point.y * matrix.m[7] + point.z * matrix.m[11] + matrix.m[15];
    Vec4 result;
    result.x = (point.x * matrix.m[0] + point.y * matrix.m[4] + point.z * matrix.m[8] + matrix.m[12]) / w;
    result.y = (point.x * matrix.m[1] + point.y * matrix.m[5] + point.z * matrix.m[9] + matrix.m[13]) / w;
    result.z = (point.x * matrix.m[2] + point.y * matrix.m[6] + point.z * matrix.m[10] + matrix.m[14]) / w;
    result.w = w;
    return result;
}

// Rotates `*point` by `angle` radians about the axis `*axis` and returns the result (w = the divisor of the transform).
// The axis is scaled by sin(angle / 2) in place and becomes the vector part of the quaternion. Called by the model code
// at EN 0x739be0.
Vec4 RotatePointAboutAxis(Vec4 *axis, float angle, const Vec4 *point)
{
    Vec4 result;
    result.x = 0;
    result.y = 0;
    result.z = 0;
    result.w = 0;
    angle *= HALF;
    float sine = (float)sin(angle);
    axis->x = axis->x * sine;
    axis->y = axis->y * sine;
    axis->z = axis->z * sine;
    float cosine = (float)cos(angle);
    float length = (float)sqrt(axis->x * axis->x + axis->y * axis->y + axis->z * axis->z + cosine * cosine);
    Vec4 quaternion;
    quaternion.x = axis->x / length;
    quaternion.y = axis->y / length;
    quaternion.z = axis->z / length;
    quaternion.w = cosine / length;
    result = ProjectPoint(*point, RotationMatrix(quaternion));
    return result;
}

// Turns this point about `axis` by `angle` radians: the quaternion of the rotation is normalised, then normalised again
// (a unit quaternion stays as it is, a zero one is left alone) and the point goes through its rotation matrix. Called by
// the pendulum and effect code.
void Vec4::RotateAxis(const Vec4 &axis, float angle)
{
    angle *= HALF;
    float sine = (float)sin(angle);
    float vx = sine * axis.x;
    float vy = sine * axis.y;
    float vz = sine * axis.z;
    float vw = (float)cos(angle);
    float length = (float)sqrt(vy * vy + vx * vx + vz * vz + vw * vw);
    Vec4 quaternion;
    quaternion.x = vx / length;
    quaternion.y = vy / length;
    quaternion.z = vz / length;
    quaternion.w = vw / length;
    length = (float)sqrt(quaternion.x * quaternion.x + quaternion.y * quaternion.y + quaternion.z * quaternion.z +
                         quaternion.w * quaternion.w);
    if (length > 0.0f)
    {
        quaternion.x = quaternion.x / length;
        quaternion.y = quaternion.y / length;
        quaternion.w = quaternion.w / length;
        quaternion.z = quaternion.z / length;
    }
    Vec4 projected = ProjectPoint(*this, RotationMatrix(quaternion));
    this->x = projected.x;
    this->y = projected.y;
    this->z = projected.z;
}
