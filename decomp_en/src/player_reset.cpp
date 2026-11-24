// Player reset (JP twin: player_reset.cpp).

#include "camera.h"

// Zeroes the camera position and target (and their copies) and the per-frame camera state. Called when the player
// is placed on a new map. Returns 0.
extern "C" int Player_ResetMotion(void)
{
    g_cameraPositionNow.Fill(0.0f);
    g_cameraTargetNow.Fill(0.0f);
    g_cameraPositionCopy.Fill(0.0f);
    g_cameraTargetCopy.Fill(0.0f);
    g_cameraState1664448 = 0;
    g_cameraState1664fe0 = 0;
    g_cameraGroundHeight = CAMERA_GROUND_UNKNOWN;
    g_cameraEyePitch = 0;
    g_cameraTargetLift = 0;
    g_cameraMode = CAMERA_MODE_FOLLOW;
    g_cameraPushRightFrames = 0;
    g_cameraPushLeftFrames = 0;
    return 0;
}
