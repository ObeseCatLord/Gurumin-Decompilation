// Matrix helpers of the engine's math unit that the effect code calls: the inverse of a matrix and the turn of a
// direction by a matrix without its translation. The
// Matrix4 and Vec4 types are in matrix4.h and vec4.h.
#ifndef GURUMIN_MATRIX_TRANSFORM_H
#define GURUMIN_MATRIX_TRANSFORM_H

#include "vec4.h"
#include "matrix4.h"

Matrix4 InverseMatrix(const Matrix4 &matrix);                          // 0x101b0d40
Vec4 TransformNormal(const Vec4 &direction, const Matrix4 &matrix);    // 0x101ae130: rotation part only

#endif
