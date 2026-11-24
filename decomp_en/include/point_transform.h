// Transforming a point by a matrix (EN build; JP: decomp/src/point_transform.cpp).
#ifndef GURUMIN_EN_POINT_TRANSFORM_H
#define GURUMIN_EN_POINT_TRANSFORM_H

#include "vec4.h"

// Rotates `point` by the rotation part of `matrix` and moves it by the matrix's translation. Returns 0. EN 0x6e0600 (JP 0x1001c510).
int Point_Transform(Vec4 *point, Matrix4 *matrix);

#endif
