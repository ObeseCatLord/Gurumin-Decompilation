// Camera matrices: the projection (perspective with the 4:3 aspect correction) and the view (look-at with roll).
// C++ rather than extern "C": the function keeps an exception frame for its matrix temporaries.

#include "vec4.h"

// Matrix4 with the engine's constructor and empty destructor (node_bones.cpp).
struct Matrix4
{
    float m[16];
    Matrix4();                                      // 0x101ae770
    ~Matrix4();                                     // 0x1013eda0 (empty)
    Matrix4 operator*(const Matrix4 &other);        // 0x101b0670: matrix product
};

Matrix4 ScaleMatrix(float x, float y, float z);                              // 0x101b13c0
Matrix4 PerspectiveMatrix(float nearPlane, float farPlane, float fov);       // 0x101b0c10
Matrix4 LookAtMatrix(Vec4 eye, Vec4 target, const Vec4 *up, float roll);     // 0x101b1c20

// 0x10194360 (Camera_SetMatrices). Fills `projection` with a perspective of fov 0.785375 (about pi/4) from 20 to 10000
// units, stretched by 1.333 in y, and `view` with the look-at from `eye` to `target` (y up) turned by `roll`.
// Returns 1.
int Camera_SetMatrices(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, float roll)
{
    Vec4 up;
    up.Set(0, 1.0f, 0, 0);
    *projection = PerspectiveMatrix(20.0f, 10000.0f, 0.785375f) * ScaleMatrix(1.0f, 1.333f, 1.0f);
    *view = LookAtMatrix(*eye, *target, &up, roll);
    return 1;
}
