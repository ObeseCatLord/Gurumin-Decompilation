// Direction to matrix (EN build). Ported from the Ghidra output and the EN bytes; the JP twin (APP.dll 0x101b1f30,
// DirectionMatrix) is declared but not matched.
#include <math.h>

#include "vec4.h"

// Element of the matrix that holds the w of a transformed point (column 3 of each row) and the homogeneous 1.
static const int MATRIX_W_X = 3;
static const int MATRIX_W_Y = 7;
static const int MATRIX_W_Z = 11;
static const int MATRIX_W_ONE = 15;

// The matrix that turns the z axis into `direction`: the heading of the direction about z is taken off first, then the
// remaining tilt towards z is found from the turned direction, and the result is that tilt about x followed by the
// heading about z. Used by the effect strips to face their direction of travel.
Matrix4 DirectionMatrix(Vec4 direction)
{
    Matrix4 matrix;
    double heading = atan2((double)direction.x, (double)direction.y);
    matrix = RotationZ(-(float)heading);
    float turnedW = matrix.m[MATRIX_W_Y] * direction.y + direction.x * matrix.m[MATRIX_W_X] +
                    matrix.m[MATRIX_W_Z] * direction.z + matrix.m[MATRIX_W_ONE];
    float turnedY = matrix.m[5] * direction.y + direction.x * matrix.m[1] + matrix.m[9] * direction.z +
                    matrix.m[MATRIX_TRANSLATION_Y];
    float turnedZ = matrix.m[2] * direction.x + matrix.m[6] * direction.y + matrix.m[10] * direction.z +
                    matrix.m[MATRIX_TRANSLATION_Z];
    double tilt = atan2((double)(turnedY / turnedW), (double)(turnedZ / turnedW));
    matrix = RotationX(-(float)tilt);
    matrix = RotationX((float)tilt) * RotationZ((float)heading);
    return matrix;
}
