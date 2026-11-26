// Scene camera mode of the EN build and the test that tells whether the scene camera is in its plain state. The other
// camera globals are in camera.h; the colour effect flag is in colour_fades.h.
#ifndef GURUMIN_EN_CAMERA_SCENE_H
#define GURUMIN_EN_CAMERA_SCENE_H

// g_cameraSceneMode value of the plain camera.
const int CAMERA_SCENE_MODE_PLAIN = 0;
// The one other mode in which Camera_IsBusy counts the camera as free.
const int CAMERA_SCENE_MODE_FIXED = 5;
// Value g_cameraState1664a30 holds while the camera is in its plain state.
const int CAMERA_STATE_1664A30_PLAIN = 1;

extern "C" int g_cameraSceneMode;        // EN 0x016646ac: mode the scene scripts put the camera in (0 = plain; set by Camera_SetSceneMode)
extern "C" int g_cameraState166441c;     // EN 0x0166441c: non-zero while a scene camera effect runs (not recovered); makes Camera_IsBusy return 1

// Sets g_cameraSceneMode. Mode 0 also turns the colour effect off and puts the two camera scales (1.0) and two camera
// states back. Returns 0. EN 0x745c20. Called by the scene start and the scene script commands (0x605380, 0x603a90, 0x5a2dd0).
extern "C" int Camera_SetSceneMode(int mode);

// 1 unless the camera is plain: no scene camera effect, the scene mode 0 without a colour effect or the mode 5, the state
// at 0x01664a30 at 1 and no path camera running (then 0). EN 0x735670. Called by the per-frame update (0x60cb10).
extern "C" int Camera_IsBusy(void);

#endif
