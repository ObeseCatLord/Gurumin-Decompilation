// Camera shift shot (EN build). Ported from the JP draft decomp/src/camera_follow.cpp (Camera_Shift, 0x10019310), which
// is not matched there.
#include "camera.h"

// Weights of the shift: the offsets keep this share of themselves each frame when the shift ends and move towards the
// shift goals by SHIFT_STEP_WEIGHT while it runs, keeping SHIFT_KEEP_WEIGHT. The camera then moves SHIFT_BLEND of the way
// (written as literals in the expressions below: Vec4's operator* takes the factor by reference, so a named constant
// would change the code).
//   shrink 0.6, keep 0.85, step 0.15, blend 0.5
// The step towards a goal is divided by 1 + the frames left * 0.5 (counted down in whole frames).
const float SHIFT_FRAMES_HALF = -0.5f;

// Like Camera_Ease with the goals offset: while a shift runs (g_cameraShiftFrames counts down) the offsets move 15 % towards
// the shift goals relative to the current camera, divided by 1 + the frames left / 2; otherwise they shrink to 60 % each
// frame. The target and position then move halfway from their previous values by the offsets, and the copies are
// refreshed. Returns 0. Called every frame while the camera runs this shot.
extern "C" int Camera_Shift(void)
{
    if (g_cameraShotStarted == 0)
    {
        Camera_SetMode2();
        g_cameraShotStarted = 1;
        g_cameraPlayerPosition.Fill(0.0f);
        g_cameraOrbitCentre.Fill(0.0f);
    }
    g_cameraPositionGoal.Copy(g_cameraPositionNow);
    g_cameraTargetGoal.Copy(g_cameraTargetNow);
    if (g_cameraShiftFrames == 0 || --g_cameraShiftFrames == 0)
    {
        g_cameraPlayerPosition.Copy(g_cameraPlayerPosition * 0.6f);
        g_cameraOrbitCentre.Copy(g_cameraOrbitCentre * 0.6f);
        g_cameraPositionGoal.Copy(g_cameraPlayerPosition);
        g_cameraTargetGoal.Copy(g_cameraOrbitCentre);
    }
    else
    {
        Vec4 positionStep;
        Vec4 targetStep;
        int half = (int)((float)g_cameraShiftFrames * SHIFT_FRAMES_HALF);
        positionStep.Copy((g_cameraShiftPosition - g_cameraPositionGoal).Divided((float)(1 - half)));
        targetStep.Copy((g_cameraShiftTarget - g_cameraTargetGoal).Divided((float)(1 - half)));
        g_cameraPlayerPosition.Copy(g_cameraPlayerPosition * 0.85f + positionStep * 0.15f);
        g_cameraOrbitCentre.Copy(g_cameraOrbitCentre * 0.85f + targetStep * 0.15f);
        g_cameraPositionGoal.Copy(g_cameraPlayerPosition);
        g_cameraTargetGoal.Copy(g_cameraOrbitCentre);
    }
    g_cameraTargetNow.Copy(g_cameraTargetNow + g_cameraTargetGoal * 0.5f + (g_cameraPreviousTarget - g_cameraTargetNow) * 0.5f);
    g_cameraPositionNow.Copy(g_cameraPositionNow + g_cameraPositionGoal * 0.5f +
                             (g_cameraPreviousPosition - g_cameraPositionNow) * 0.5f);
    g_cameraTargetCopy.Copy(g_cameraTargetNow);
    g_cameraPositionCopy.Copy(g_cameraPositionNow);
    g_cameraTarget.Copy(g_cameraTargetNow);
    g_cameraEye.Copy(g_cameraPositionNow);
    g_cameraPreviousPosition.Copy(g_cameraPositionNow);
    g_cameraPreviousTarget.Copy(g_cameraTargetNow);
    return 0;
}
