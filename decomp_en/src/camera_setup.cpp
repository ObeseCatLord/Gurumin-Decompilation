// Camera matrix set-up (EN build). Ported from the matched JP source decomp/src/camera_setup.cpp.
#include "camera.h"
#include "render.h"

// Perspective of the camera: near and far plane, field of view in radians and the stretch of the projection in y.
const float CAMERA_NEAR_PLANE = 20.0f;
const float CAMERA_FAR_PLANE = 10000.0f;
const float CAMERA_FIELD_OF_VIEW = 0.785375f;
const float CAMERA_Y_STRETCH = 1.333f;

// Camera_SetMatrices: fills `projection` with the perspective of the camera (field of view CAMERA_FIELD_OF_VIEW from
// CAMERA_NEAR_PLANE to CAMERA_FAR_PLANE, aspect from the back buffer, stretched by CAMERA_Y_STRETCH in y) and `view` with
// the look-at from `eye` to `target` (y up) turned by `roll`. Returns 1. Called whenever the camera moves.
int Camera_SetMatrices(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, float roll)
{
    Vec4 up;
    up.Set(0, 1.0f, 0, 0);
    *projection = PerspectiveMatrix(CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE, CAMERA_FIELD_OF_VIEW,
                                    (float)g_deviceBackBufferHeight / (float)g_deviceBackBufferWidth) *
                  ScaleMatrix(1.0f, CAMERA_Y_STRETCH, 1.0f);
    *view = LookAt(*eye, *target, &up, roll);
    return 1;
}
