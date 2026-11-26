// Matrix interpolation of the EN build (JP twin: APP.dll 0x101b1420, not matched). Ported from the Ghidra output and the
// EN bytes.
#include "node.h"
#include "key_bank.h"

// Fills out[0 .. count - 1] with the matrices that lead from `a` (index 0) to `b` (index count - 1). The rotations are
// the quaternions of the two matrices, run through a key bank and turned back into matrices. In mode BLEND_LINEAR every
// result gets the translation of `b`; in mode BLEND_ROTATION the translations are interpolated through a second key
// bank. Always returns 1. Called by Node::BlendFrames and the model blending code.
// (The original zeroes the translation of `a` twice, before both quaternion conversions, and never that of `b`.)
extern "C" int MatrixBlend(Matrix4 *out, Matrix4 a, Matrix4 b, int count, int mode)
{
    KeyBank positionKeys;
    KeyBank rotationKeys;

    float startX = a.m[MATRIX_TRANSLATION_X];
    float positionX = startX;
    float startY = a.m[MATRIX_TRANSLATION_Y];
    float positionY = startY;
    float startZ = a.m[MATRIX_TRANSLATION_Z];
    float positionZ = startZ;
    float positionW;
    a.m[MATRIX_TRANSLATION_X] = 0.0f;
    a.m[MATRIX_TRANSLATION_Y] = 0.0f;
    a.m[MATRIX_TRANSLATION_Z] = 0.0f;

    const Vec4 &startRotation = QuatFromMatrix(a);
    float startRotX = startRotation.x;
    float rotationX = startRotX;
    float startRotY = startRotation.y;
    float rotationY = startRotY;
    float startRotZ = startRotation.z;
    float rotationZ = startRotZ;
    float startRotW = startRotation.w;
    float rotationW = startRotW;

    float endX = b.m[MATRIX_TRANSLATION_X];
    float endY = b.m[MATRIX_TRANSLATION_Y];
    float endZ = b.m[MATRIX_TRANSLATION_Z];
    a.m[MATRIX_TRANSLATION_X] = 0.0f;
    a.m[MATRIX_TRANSLATION_Y] = 0.0f;
    a.m[MATRIX_TRANSLATION_Z] = 0.0f;

    const Vec4 &endRotation = QuatFromMatrix(b);
    float endRotX = endRotation.x;
    float endRotY = endRotation.y;
    float endRotZ = endRotation.z;
    float endRotW = endRotation.w;

    rotationKeys.Clear();
    rotationKeys.Insert(0, startRotX, startRotY, startRotZ, startRotW);
    rotationKeys.Insert(count - 1, endRotX, endRotY, endRotZ, endRotW);

    if (mode != BLEND_LINEAR)
    {
        positionKeys.Clear();
        positionKeys.Insert(0, startX, startY, startZ, 0.0f);
        positionKeys.Insert(count - 1, endX, endY, endZ, 0.0f);
    }

    for (int i = 0; i < count; i++)
    {
        rotationKeys.Sample(i, &rotationX, &rotationY, &rotationZ, &rotationW, KEY_SAMPLE_ROTATION);
        out[i] = RotationMatrix(Vec4(rotationX, rotationY, rotationZ, rotationW));
        if (mode != BLEND_LINEAR)
        {
            positionKeys.Sample(i, &positionX, &positionY, &positionZ, &positionW, KEY_SAMPLE_POSITION);
            out[i].m[MATRIX_TRANSLATION_X] = positionX;
            out[i].m[MATRIX_TRANSLATION_Y] = positionY;
            out[i].m[MATRIX_TRANSLATION_Z] = positionZ;
        }
        else
        {
            out[i].m[MATRIX_TRANSLATION_X] = endX;
            out[i].m[MATRIX_TRANSLATION_Y] = endY;
            out[i].m[MATRIX_TRANSLATION_Z] = endZ;
        }
    }
    return 1;
}
