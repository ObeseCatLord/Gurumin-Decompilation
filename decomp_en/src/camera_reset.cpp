// Camera reset (EN build). Ported from the matched JP source decomp/src/camera_reset.cpp.
#include "camera.h"
#include "colour_fades.h"
#include "small_b11.h"
#include "world_renderer.h"

// Camera_Reset: clears the camera globals: the camera position and target vectors and their goals, the blend and cut
// counters (g_cameraGroundHeight = -100, g_cameraCounter9089ec = 1), and sets the scale factors to 1 and g_cameraRate to
// 12. Returns 0. Called when a scene starts.
extern "C" int Camera_Reset(void)
{
    g_cameraPushRightFrames = 0;
    g_cameraPushLeftFrames = 0;
    g_cameraState1664418 = 0;
    g_cameraTargetLift = 0;
    g_cameraFlag1664468 = 0;
    g_cameraFlag162ef61 = 0;
    g_cameraRate = CAMERA_RESET_RATE;
    g_cameraScale = 1.0f;
    g_cameraState1664444 = 0;
    g_cameraShotEase[0] = 1.0f;
    g_cameraShotEase[1] = 1.0f;
    g_cameraShotEase[2] = 1.0f;
    g_cameraShotEase[3] = 1.0f;
    g_cameraState166444c = 0;
    g_cameraPositionNow.Fill(0.0f);
    g_cameraTargetNow.Fill(0.0f);
    g_cameraPositionCopy.Fill(0.0f);
    g_cameraState1664450 = 0;
    g_cameraTargetCopy.Fill(0.0f);
    g_cameraState1664448 = 0;
    g_cameraCounter9089ec = 1;
    g_boundModelsExtra = 0;
    g_cameraState1664fd0 = 0;
    g_cameraState1664178 = 0;
    g_cameraState1664fe0 = 0;
    g_cameraGroundHeight = CAMERA_GROUND_UNKNOWN;
    g_cameraPathCount = 0;
    g_cameraMoveStep = 0;
    g_cameraState166469c = 0;
    g_cameraMoveFramesLeft = 0;
    g_cameraState16646a4 = 0;
    g_cameraState1664204 = 0;
    g_cameraState1664ab8 = 0;
    g_titleCaptionText[0] = 0;
    g_titleCaptionTimer = 0;
    g_cameraState1664fe4 = 0;
    g_cameraState166cfec = 0;
    g_cameraState166cff0 = 0;
    g_cameraState1664210 = 0;
    g_cameraState166cff4 = 0;
    g_cameraState1664200 = 0;
    g_cameraState1664208 = 0;
    g_cameraScale9089e4 = 1.0f;
    g_cameraState1664a30 = 0;
    g_worldFrameA = 0;
    g_worldFrameB = 0;
    g_worldFrameC = 0;
    g_worldFrameD = 0;
    g_cameraEyePitch = 0;
    g_cameraPathFrame = 0;
    g_cameraState1664214 = 0;
    g_colourEffectActive = 0;
    g_colourEffectPhase = 0;
    g_colourEffectMode = 0;
    g_cameraState166cfe8 = 0;
    g_cameraState1664fd4 = 0;
    g_cameraState166cff8 = 0;
    g_cameraState166cffc = 0;
    g_cameraMoveSelected = 0;
    g_cameraMoveReset = 0;
    g_cameraState1664680 = 0;
    g_cameraTargetPathDistance = 0;
    g_colourEffectColour.Fill(0.0f);
    g_cameraPushOffset.Fill(0.0f);
    g_colourEffectColour.Fill(0.0f);
    g_cameraPositionCopy.Fill(0.0f);
    g_cameraTargetCopy.Fill(0.0f);
    g_cameraPositionCopyB.Fill(0.0f);
    g_cameraTargetCopyB.Fill(0.0f);
    g_cameraPositionCopyC.Fill(0.0f);
    g_cameraTargetCopyC.Fill(0.0f);
    g_cameraPositionCopyD.Fill(0.0f);
    g_cameraTargetCopyD.Fill(0.0f);
    g_cameraVectorSpare.Fill(0.0f);
    g_cameraPositionNow.Fill(0.0f);
    g_cameraTargetNow.Fill(0.0f);
    return 0;
}
