// Camera and caption globals of the EN build (declared once here; JP equivalents in decomp/include/camera_vars.h).
#ifndef GURUMIN_EN_CAMERA_H
#define GURUMIN_EN_CAMERA_H

// Camera control modes stored in g_cameraMode.
enum CameraMode
{
    CAMERA_MODE_FOLLOW = 0,   // normal follow camera
    CAMERA_MODE_1 = 1,        // mode 1 (set by Camera_SetMode1)
    CAMERA_MODE_2 = 2         // mode 2 (set by Camera_SetMode2)
};

const int CAPTION_TEXT_SIZE = 0x100;       // bytes of the title caption buffer

extern "C" int g_cameraMode;                           // EN 0x01664a34 (JP DAT_1101de1c): camera control mode (CameraMode)
extern "C" char g_titleCaptionText[CAPTION_TEXT_SIZE]; // EN 0x01664ac0 (JP DAT_1101d8b8): title caption text
extern "C" int g_titleCaptionTimer;                    // EN 0x01664abc (JP DAT_1101d9b8): title caption timer, 0 = just started


#include "vec4.h"

const int CAMERA_MOVE_FRAMES = 10;          // frames a started camera move runs

extern "C" int g_cameraMoveReset;           // EN 0x01664fdc (JP DAT_10fabe88): cleared when a camera move starts
extern "C" int g_cameraMoveSelected;        // EN 0x01664fd8 (JP DAT_11018738): selected path of the move, -1 = none
extern "C" int g_cameraMoveStep;            // EN 0x01664698 (JP DAT_1101d8a0): step of the move, 0 = first
extern "C" int g_cameraPathCount;           // EN 0x01664694 (JP DAT_1101d89c): number of camera paths
extern "C" int g_cameraMoveFramesLeft;      // EN 0x016646a0 (JP DAT_1101d8a8): frames left in the camera move
extern "C" Vec4 g_cameraEye;                // EN 0x021ffed8 (JP DAT_10f65688): camera position
extern "C" Vec4 g_cameraTarget;             // EN 0x02200ee8 (JP DAT_10f668e8): camera target

// Collects the camera paths of move `moveId` into *pathCount. EN 0x744e40 (JP 0x1016ac90).
extern "C" int Camera_CollectPaths(int moveId, int moveIdAgain, int *pathCount);

// Builds the view and projection matrices for a camera at `eye` looking at `target`. EN 0x759300 (JP 0x10194360).
extern "C" int Camera_SetMatrices(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, float roll);

// Positions the camera keeps in view (at most CAMERA_TRACKED_COUNT).
const int CAMERA_TRACKED_COUNT = 0x30;

extern "C" float *g_trackedPositions[CAMERA_TRACKED_COUNT];   // EN 0x0091f2b0 (JP DAT_1032d5b8): translation rows kept in view
extern "C" char g_trackedFlags[CAMERA_TRACKED_COUNT];         // EN 0x0091f410 (JP DAT_1021bbc0): a flag per tracked position
extern "C" int g_trackedCount;                                // EN 0x0091e76c (JP DAT_1049f0f0): entries of the two lists

// Adds character `index` position to the list of positions the camera keeps in view. Returns 1. EN 0x645730.
extern "C" int Camera_AddTrackedPosition(int index);

#endif
