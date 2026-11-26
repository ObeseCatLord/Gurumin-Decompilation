// Matrix inverse (EN build). Ported from the EN bytes; the JP twin (APP.dll 0x101b0d40, InverseMatrix) is not matched.
#include "vec4.h"

// A matrix whose determinant is smaller than this fraction of the sum of its term magnitudes counts as singular.
const double MATRIX_SINGULAR_LIMIT = 1.0e-15;
// Element of the matrix that holds the homogeneous 1 (m[15]).
const int MATRIX_HOMOGENEOUS_ELEMENT = 15;

// The inverse of a matrix made of a rotation/scale part and a translation row (the last column is taken as 0, 0, 0, 1).
// The determinant of the 3x3 part is summed as positive and negative terms separately to judge how well conditioned it
// is; a singular matrix gives the identity. Called by the node, camera and animation code that need a world-to-local
// matrix.
Matrix4 InverseMatrix(const Matrix4 &matrix)
{
    Matrix4 inverse;
    float positive = 0.0f;
    float negative = 0.0f;
    float term = matrix.m[0] * matrix.m[5] * matrix.m[10];
    if (term >= 0.0f)
        positive += term;
    else
        negative += term;
    term = matrix.m[1] * matrix.m[6] * matrix.m[8];
    if (term >= 0.0f)
        positive += term;
    else
        negative += term;
    term = matrix.m[2] * matrix.m[4] * matrix.m[9];
    if (term >= 0.0f)
        positive += term;
    else
        negative += term;
    term = -(matrix.m[2] * matrix.m[5] * matrix.m[8]);
    if (term >= 0.0f)
        positive += term;
    else
        negative += term;
    term = -(matrix.m[4] * matrix.m[1] * matrix.m[10]);
    if (term >= 0.0f)
        positive += term;
    else
        negative += term;
    term = -(matrix.m[6] * matrix.m[0] * matrix.m[9]);
    if (term >= 0.0f)
        positive += term;
    else
        negative += term;

    float determinant = negative + positive;
    if (determinant == 0.0f)
        return IdentityMatrix();
    float conditioning = determinant / (positive - negative);
    if (conditioning < 0.0f)
        conditioning = -conditioning;
    if (MATRIX_SINGULAR_LIMIT > conditioning)
        return IdentityMatrix();

    float scale = 1.0f / determinant;
    inverse.m[0] = (matrix.m[5] * matrix.m[10] - matrix.m[9] * matrix.m[6]) * scale;
    inverse.m[4] = -((matrix.m[4] * matrix.m[10] - matrix.m[8] * matrix.m[6]) * scale);
    inverse.m[8] = (matrix.m[9] * matrix.m[4] - matrix.m[8] * matrix.m[5]) * scale;
    inverse.m[1] = -((matrix.m[1] * matrix.m[10] - matrix.m[9] * matrix.m[2]) * scale);
    inverse.m[5] = (matrix.m[0] * matrix.m[10] - matrix.m[8] * matrix.m[2]) * scale;
    inverse.m[9] = -((matrix.m[9] * matrix.m[0] - matrix.m[8] * matrix.m[1]) * scale);
    inverse.m[2] = (matrix.m[1] * matrix.m[6] - matrix.m[2] * matrix.m[5]) * scale;
    inverse.m[6] = -((matrix.m[0] * matrix.m[6] - matrix.m[2] * matrix.m[4]) * scale);
    inverse.m[10] = (matrix.m[0] * matrix.m[5] - matrix.m[4] * matrix.m[1]) * scale;
    inverse.m[MATRIX_TRANSLATION_X] =
        -(matrix.m[13] * inverse.m[4] + matrix.m[12] * inverse.m[0] + matrix.m[14] * inverse.m[8]);
    inverse.m[MATRIX_TRANSLATION_Y] =
        -(matrix.m[13] * inverse.m[5] + matrix.m[12] * inverse.m[1] + matrix.m[14] * inverse.m[9]);
    inverse.m[MATRIX_TRANSLATION_Z] =
        -(matrix.m[13] * inverse.m[6] + matrix.m[12] * inverse.m[2] + matrix.m[14] * inverse.m[10]);
    inverse.m[3] = 0.0f;
    inverse.m[7] = 0.0f;
    inverse.m[11] = 0.0f;
    inverse.m[MATRIX_HOMOGENEOUS_ELEMENT] = 1.0f;
    return inverse;
}
