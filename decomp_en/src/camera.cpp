// Camera mode setters and title caption (EN build). Ported from the matched JP source decomp/src/camera.cpp.
#include <string.h>

#include "camera.h"

// Camera_SetMode2: switches the camera to control mode 2. Called through the script command table.
extern "C" int Camera_SetMode2(void)
{
    g_cameraMode = CAMERA_MODE_2;
    return 0;
}

// Camera_SetMode1: switches the camera to control mode 1. Called through the script command table.
extern "C" int Camera_SetMode1(void)
{
    g_cameraMode = CAMERA_MODE_1;
    return 0;
}

// Camera_SetModeFollow: switches the camera back to the normal follow mode. Called through the script command table.
extern "C" int Camera_SetModeFollow(void)
{
    g_cameraMode = CAMERA_MODE_FOLLOW;
    return 0;
}

// Caption_SetTitle: sets the title caption text and restarts its timer. Called through the script command table.
extern "C" int Caption_SetTitle(const char *text)
{
    strcpy(g_titleCaptionText, text);
    g_titleCaptionTimer = 0;
    return 0;
}

// Camera_StartMove: starts camera move `moveId`: resets the move state, collects the move's paths and runs it for
// CAMERA_MOVE_FRAMES frames. Called through the script command table.
extern "C" int Camera_StartMove(int moveId)
{
    int pathCount;
    g_cameraMoveReset = 0;
    g_cameraMoveSelected = -1;
    pathCount = 0;
    g_cameraMoveStep = 0;
    Camera_CollectPaths(moveId, moveId, &pathCount);
    g_cameraPathCount = pathCount;
    g_cameraMoveFramesLeft = CAMERA_MOVE_FRAMES;
    return 0;
}

// Camera_LookAt: places the camera at `eye` looking at `target` and remembers both positions. Called through the
// script command table.
extern "C" int Camera_LookAt(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection)
{
    Camera_SetMatrices(eye, target, view, projection, 0.0f);
    g_cameraEye.Copy(*eye);
    g_cameraTarget.Copy(*target);
    return 0;
}
