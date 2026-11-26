// Globals and functions of the per-frame camera refresh (EN build): the view matrices, the sound target and the
// camera roll table. Declared once here; the rest of the camera is in camera.h and map_view.h.
#ifndef GURUMIN_EN_CAMERA_REFRESH_H
#define GURUMIN_EN_CAMERA_REFRESH_H

#include "vec4.h"

// Entries of the camera roll table (roll in degrees per roll index).
const int CAMERA_ROLL_COUNT = 14;

extern "C" Vec4 g_cameraSoundTarget;                  // EN 0x021ffd58 (JP DAT_10f66938): where the sound listener looks
extern "C" float g_cameraEyeHeight;                   // EN 0x021a34bc (JP DAT_10f65590): the player's height plus CAMERA_HEIGHT_OFFSET
extern "C" float g_cameraRollTable[CAMERA_ROLL_COUNT];  // EN 0x01339a3c (JP DAT_10c68944): camera roll in degrees per index
extern "C" int g_cameraRollIndex;                     // EN 0x01339a74 (JP DAT_10c6897c): entry of the roll table in use
extern "C" char g_cameraSoundTargetOverride;          // EN 0x0133ef24 (JP DAT_10c6de2c): non-zero = the sound target is g_cameraSoundTargetFixed
extern "C" Vec4 g_cameraSoundTargetFixed;             // EN 0x0133ef28 (JP DAT_10c6de30): sound target used while the override is set

// Fills `eye` and `target` with the camera of the running scene (path, follow or chase camera). EN 0x712f90 (JP
// 0x1016c3e0).
extern "C" int Camera_Update(Vec4 *eye, Vec4 *target);

// Rebuilds the camera view matrix and its two view-projection matrices, publishes the camera position, target and
// sound target and places the sound listener. Returns 0. EN 0x5a90c0 (JP 0x10015960).
int Camera_Refresh(void);

#endif
