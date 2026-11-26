// Scene camera mode and busy test (EN build, EN-only: no JP twin found).
#include "camera.h"
#include "camera_scene.h"
#include "colour_fades.h"

// Scale value the camera scales return to when the scene mode goes back to plain.
const float CAMERA_SCALE_DEFAULT = 1.0f;

// Camera_SetSceneMode: stores the scene camera mode. Going back to the plain mode also ends the colour effect and
// resets the two camera scales and two camera states. Returns 0.
extern "C" int Camera_SetSceneMode(int mode)
{
    g_cameraSceneMode = mode;
    if (mode == CAMERA_SCENE_MODE_PLAIN)
    {
        g_colourEffectActive = 0;
        g_cameraScale = CAMERA_SCALE_DEFAULT;
        g_cameraScale9089e4 = CAMERA_SCALE_DEFAULT;
        g_cameraState1664178 = 0;
        g_cameraState1664208 = 0;
    }
    return 0;
}

// Camera_IsBusy: returns 1 when the camera is doing something besides plain play: a scene camera effect, a scene mode
// other than plain (without a colour effect) or fixed, the state at 0x01664a30 not at its plain value, or a path camera
// running. Returns 0 when it is plain.
extern "C" int Camera_IsBusy(void)
{
    if (g_cameraState166441c == 0 && ((g_cameraSceneMode == CAMERA_SCENE_MODE_PLAIN && g_colourEffectActive == 0) ||
                                      g_cameraSceneMode == CAMERA_SCENE_MODE_FIXED) &&
        g_cameraState1664a30 == CAMERA_STATE_1664A30_PLAIN)
        return g_cameraState1664fd0 != 0;
    return 1;
}
