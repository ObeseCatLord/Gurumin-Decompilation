// The per-frame game camera (camera_update.cpp): where the eye and the target stand this frame.
#ifndef GURUMIN_CAMERA_UPDATE_H
#define GURUMIN_CAMERA_UPDATE_H

#include "vec4.h"

// 0x1016c3e0 (camera_update.cpp): computes the camera eye and target for this frame from the active camera mode,
// stores them in `eye` and `target` and in the camera globals. Returns 1 when a camera was placed, 0 when the
// scene has no camera (no path and no mode).
int Camera_Update(Vec4 *eye, Vec4 *target);

#endif
