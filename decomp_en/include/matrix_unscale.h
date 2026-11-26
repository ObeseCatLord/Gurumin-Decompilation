// RemoveScale (EN build, no JP name). Declared once here; C linkage because the original has no unwind frame.
#ifndef GURUMIN_EN_MATRIX_UNSCALE_H
#define GURUMIN_EN_MATRIX_UNSCALE_H

#include "vec4.h"

// Matrix `source` with the scale along its three axes taken out (the scale matrix's inverse applied to it) and the
// translation row cleared. EN 0x773f20 (JP 0x101b17a0), matrix_unscale.cpp.
extern "C" Matrix4 RemoveScale(const Matrix4 &source);

// Splits `source` into scale (x/y/z of `scale`), unit rotation quaternion and translation; returns 0.
// EN 0x7743b0 (JP 0x101b1910), matrix_unscale.cpp.
extern "C" int DecomposeMatrix(const Matrix4 *source, Vec4 *scale, Vec4 *rotation, Vec4 *translation);

#endif
