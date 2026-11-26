// Camera look-at matrix (EN build, JP 0x101b1c20 has no matched source).
#include <math.h>
#include "vec4.h"

// Scales (x, y, z) to length 1; a zero vector becomes (0, 0, 0). Inline in the original LookAt.
static inline void NormalizeOrZero(float &x, float &y, float &z)
{
    float length = (float)sqrt(x * x + y * y + z * z);
    if (length == 0.0f)
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }
    else
    {
        x = x / length;
        y = y / length;
        z = z / length;
    }
}

// Matrix that views from `eye` towards `target` with `up` as the upward direction, turned by `roll` radians about the
// view axis (0 = none). The engine's y and z are swapped: y and z of both positions are exchanged first, and the result
// is multiplied by a matrix that swaps y and z back. Identical eye and target give the identity matrix. Called by the
// camera set-up, the map view, the shadow texture and the mesh slots.
Matrix4 LookAt(Vec4 eye, Vec4 target, Vec4 *up, float roll)
{
    if (target.x == eye.x && target.y == eye.y && target.z == eye.z)
        return IdentityMatrix();

    float eyeY = eye.y;
    float eyeZ = eye.z;
    float targetY = target.y;
    float targetZ = target.z;
    eye.y = eyeZ;
    eye.z = eyeY;
    target.y = targetZ;
    target.z = targetY;

    Matrix4 view = IdentityMatrix();

    // forward axis: from the eye to the target
    float forwardY = targetZ - eyeZ;
    float forwardX = target.x - eye.x;
    float forwardZ = targetY - eyeY;
    NormalizeOrZero(forwardX, forwardY, forwardZ);

    // right axis: up x forward, normalised
    float rightX = up->y * forwardZ - up->z * forwardY;
    float rightY = up->z * forwardX - up->x * forwardZ;
    float rightZ = up->x * forwardY - up->y * forwardX;

    // true up axis: forward x right (taken before the right axis is normalised), normalised
    float trueUpX = forwardY * rightZ - forwardZ * rightY;
    float trueUpY = forwardZ * rightX - forwardX * rightZ;
    float trueUpZ = forwardX * rightY - forwardY * rightX;

    NormalizeOrZero(rightX, rightY, rightZ);
    NormalizeOrZero(trueUpX, trueUpY, trueUpZ);

    view.m[0] = rightX;
    view.m[1] = trueUpX;
    view.m[2] = forwardX;
    view.m[4] = rightY;
    view.m[5] = trueUpY;
    view.m[6] = forwardY;
    view.m[8] = rightZ;
    view.m[9] = trueUpZ;
    view.m[10] = forwardZ;
    view.m[MATRIX_TRANSLATION_X] = -(rightY * eyeZ + rightX * eye.x + rightZ * eyeY);
    view.m[MATRIX_TRANSLATION_Y] = -(trueUpY * eyeZ + trueUpX * eye.x + trueUpZ * eyeY);
    view.m[MATRIX_TRANSLATION_Z] = -(forwardY * eyeZ + forwardX * eye.x + forwardZ * eyeY);

    Matrix4 swapYZ = ZeroMatrix();
    swapYZ.m[0] = 1.0f;
    swapYZ.m[6] = 1.0f;
    swapYZ.m[9] = 1.0f;
    swapYZ.m[15] = 1.0f;
    view = Multiply(view, swapYZ);

    if (roll != 0.0f)
        view = view * RotationZ(-roll);
    return view;
}
