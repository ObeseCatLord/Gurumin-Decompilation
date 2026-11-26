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

// Splits `source` into its translation, the scale along each axis (x/y/z of `scale`), and the rotation quaternion of
// what is left once the scale is taken out (normalised over all four components; unit length unless it is zero).
// Same steps as RemoveScale, with the quaternion taken at the end. Called by the node transform code.
extern "C" int DecomposeMatrix(const Matrix4 *source, Vec4 *scale, Vec4 *rotation, Vec4 *translation)
{
    translation->x = source->m[MATRIX_TRANSLATION_X];
    translation->y = source->m[MATRIX_TRANSLATION_Y];
    translation->z = source->m[MATRIX_TRANSLATION_Z];
    translation->w = 0;

    Matrix4 unscaled = *source;
    unscaled.m[MATRIX_TRANSLATION_X] = 0;
    unscaled.m[MATRIX_TRANSLATION_Y] = 0;
    unscaled.m[MATRIX_TRANSLATION_Z] = 0;
    scale->x = ProjectedAxisLength(unscaled, 1, 0, 0);
    scale->y = ProjectedAxisLength(unscaled, 0, 1, 0);
    scale->z = ProjectedAxisLength(unscaled, 0, 0, 1);
    unscaled = InverseMatrix(ScaleMatrix(scale->x, scale->y, scale->z)) * unscaled;

    Vec4 quaternion = QuatFromMatrix(unscaled);
    float length = (float)sqrt(quaternion.x * quaternion.x + quaternion.y * quaternion.y + quaternion.z * quaternion.z + quaternion.w * quaternion.w);
    rotation->x = quaternion.x;
    rotation->y = quaternion.y;
    rotation->z = quaternion.z;
    rotation->w = quaternion.w;
    if (0.0f < length)
    {
        rotation->x = quaternion.x / length;
        rotation->y = quaternion.y / length;
        rotation->z = quaternion.z / length;
        rotation->w = quaternion.w / length;
    }
    return 0;
}
