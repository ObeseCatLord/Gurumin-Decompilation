// Camera_SetMatrices, declared once (defined in src/camera_setup.cpp).
#ifndef GURUMIN_CAMERA_SETUP_H
#define GURUMIN_CAMERA_SETUP_H

#include "matrix4.h"
#include "vec4.h"

// 0x10194360: fills `projection` and `view` for a camera at `eye` looking at `target`, turned by `roll`. Returns 1.
int Camera_SetMatrices(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, float roll);

#endif
