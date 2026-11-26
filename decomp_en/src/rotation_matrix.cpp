// Quaternion to rotation matrix (EN build). Ported from the Ghidra output and the EN bytes; the JP twin (APP.dll
// 0x101b08c0, RotationMatrix) is not matched.
#include "vec4.h"

// Element of the matrix that holds the homogeneous 1 (m[15]).
static const int MATRIX_HOMOGENEOUS = 15;

// The rotation matrix of the quaternion `quaternion` (x, y, z = vector part, w = scalar part). The quaternion does not have
// to be a unit one: the scale 2 / |q|^2 normalises it (all zero for a zero quaternion). The translation row is zero.
// Called by the animation and node code (0x731970, 0x649da0, 0x778ed0, 0x771830, 0x778b60, 0x7723c0, 0x6a0950, 0x763e70).
Matrix4 RotationMatrix(const Vec4 &quaternion)
{
    Matrix4 matrix;
    float xx = quaternion.x * quaternion.x;
    float zz = quaternion.z * quaternion.z;
    float scale = 0.0f;
    float lengthSquared = xx + quaternion.y * quaternion.y + zz + quaternion.w * quaternion.w;
    if (lengthSquared > 0.0f)
        scale = 2.0f / lengthSquared;
    float yyScaled = quaternion.y * quaternion.y * scale;
    float zzScaled = zz * scale;
    float xxScaled = xx * scale;
    float wz = quaternion.w * quaternion.z * scale;
    float xy = quaternion.x * quaternion.y * scale;
    float zx = quaternion.z * quaternion.x * scale;
    float yz = quaternion.z * quaternion.y * scale;
    float wx = quaternion.w * quaternion.x * scale;
    float wy = quaternion.w * quaternion.y * scale;
    matrix.m[0] = 1.0f - (zzScaled + yyScaled);
    matrix.m[4] = wz + xy;
    matrix.m[1] = xy - wz;
    matrix.m[8] = zx - wy;
    matrix.m[5] = 1.0f - (zzScaled + xxScaled);
    matrix.m[9] = wx + yz;
    matrix.m[2] = wy + zx;
    matrix.m[6] = yz - wx;
    matrix.m[10] = 1.0f - (yyScaled + xxScaled);
    matrix.m[11] = 0.0f;
    matrix.m[7] = 0.0f;
    matrix.m[3] = 0.0f;
    matrix.m[MATRIX_TRANSLATION_Z] = 0.0f;
    matrix.m[MATRIX_TRANSLATION_Y] = 0.0f;
    matrix.m[MATRIX_TRANSLATION_X] = 0.0f;
    matrix.m[MATRIX_HOMOGENEOUS] = 1.0f;
    return matrix;
}
