// Matrix heading (EN build; JP twin: Matrix4::M_101b0a80).
#include <math.h>
#include "vec4.h"

// Angle of the matrix's x axis in the x/y plane, ignoring the translation: the point (1, 0, 0) is projected through a
// copy of the matrix with a zero translation row and the angle is atan2(y, x). Called by the scripts and the
// animation code to read the facing of a model.
float Matrix4::Heading() const
{
    Matrix4 rotation = *this;
    rotation.m[MATRIX_TRANSLATION_X] = 0;
    rotation.m[MATRIX_TRANSLATION_Y] = 0;
    rotation.m[MATRIX_TRANSLATION_Z] = 0;
    Vec4 axis = Project(Vec4(1, 0, 0, 1), rotation);
    return atan2f(axis.y, axis.x);
}
