// Removing the scale from a transform matrix (EN build, EN-only: no JP twin found).
#include <math.h>
#include "matrix_unscale.h"
#include "vec4.h"

// Length of the unit vector (x, y, z) after Project transformed it by `matrix` (w divide included). Inline copy of
// Project and Vec4::Length, which the original compiled into RemoveScale.
static inline float ProjectedAxisLength(const Matrix4 &matrix, float x, float y, float z)
{
    const float *e = matrix.m;
    float w = x * e[3] + y * e[7] + z * e[11] + e[15];
    float px = (x * e[0] + y * e[4] + z * e[8] + e[12]) / w;
    float py = (x * e[1] + y * e[5] + z * e[9] + e[13]) / w;
    float pz = (x * e[2] + y * e[6] + z * e[10] + e[14]) / w;
    return (float)sqrt(px * px + py * py + pz * pz);
}

// Returns `source` with its scale taken out and its translation dropped. The scale along each axis is the length of
// the unit vector of that axis transformed by the matrix (translation cleared); the result is the inverse of that scale
// matrix applied to the matrix. Called by the model placement code.
extern "C" Matrix4 RemoveScale(const Matrix4 &source)
{
    Matrix4 result = source;
    result.m[MATRIX_TRANSLATION_X] = 0;
    result.m[MATRIX_TRANSLATION_Y] = 0;
    result.m[MATRIX_TRANSLATION_Z] = 0;
    float scaleX = ProjectedAxisLength(result, 1, 0, 0);
    float scaleY = ProjectedAxisLength(result, 0, 1, 0);
    float scaleZ = ProjectedAxisLength(result, 0, 0, 1);
    result = InverseMatrix(ScaleMatrix(scaleX, scaleY, scaleZ)) * result;
    return result;
}
