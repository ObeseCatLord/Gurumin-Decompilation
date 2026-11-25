// Camera mode setters and title caption (EN build). Ported from the matched JP source decomp/src/camera.cpp.
#include <string.h>

#include "camera.h"
#include "effect_nodes.h"
#include "char.h"

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

// Camera_StartMove: starts a camera move: resets the move state, collects the camera paths of the node tree `root` and
// runs the move for CAMERA_MOVE_FRAMES frames. Called through the script command table.
extern "C" int Camera_StartMove(Node *root)
{
    int pathCount;
    g_cameraMoveReset = 0;
    g_cameraMoveSelected = -1;
    pathCount = 0;
    g_cameraMoveStep = 0;
    Camera_CollectPaths(root, root, &pathCount);
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

// EffectNodes_ReleaseAll: releases every used node of the effect node pool. Called when the effects are unloaded.
extern "C" int EffectNodes_ReleaseAll(void)
{
    for (int i = 0; i < EFFECT_NODE_COUNT; i++)
    {
        if (g_effectNodeLoaded[i] != 0)
            g_effectNodes[i].Release();
    }
    return 0;
}

// Camera_AddTrackedPosition: adds the position of character `index` (the translation row of its world matrix) to the
// list of positions the camera keeps in view. Called through the script command table.
extern "C" int Camera_AddTrackedPosition(int index)
{
    g_trackedPositions[g_trackedCount] = &g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
    g_trackedFlags[g_trackedCount] = 0;
    g_trackedCount++;
    return 1;
}
