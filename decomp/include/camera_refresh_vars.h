// Globals of the scene camera refresh (camera_refresh.cpp): the roll table and the cut-scene sound target.
#ifndef GURUMIN_CAMERA_REFRESH_VARS_H
#define GURUMIN_CAMERA_REFRESH_VARS_H

#include "vec4.h"

extern "C" float DAT_10c68944[];                          // camera roll in degrees, per roll index
extern "C" int DAT_10c6897c;                              // index into DAT_10c68944
extern "C" char DAT_10c6de2c;                             // non-zero = the sound target is DAT_10c6de30 (mode 1 only)
extern "C" Vec4 DAT_10c6de30;                             // sound target override

// 0x10015960 (camera_refresh.cpp): rebuilds the camera matrix and view-projections and publishes eye, target and
// the sound listener. Returns 0.
int Camera_Refresh(void);

#endif
