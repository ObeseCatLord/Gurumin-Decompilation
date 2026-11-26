// Applying a camera and remembering its position (EN build, EN-only: no JP twin found).
#include "camera.h"

// Camera_SetAndRemember: builds the view and projection matrices for the camera (see Camera_SetMatrices) and then copies
// the eye and target into the global camera position and target. Returns 0. No direct caller.
int Camera_SetAndRemember(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, float roll)
{
    Camera_SetMatrices(eye, target, view, projection, roll);
    g_cameraEye.Copy(*eye);
    g_cameraTarget.Copy(*target);
    return 0;
}
