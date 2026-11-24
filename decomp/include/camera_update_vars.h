// Globals of the per-frame game camera (camera_update.cpp): the scripted camera key banks, the event camera key
// banks and the camera copies the free camera restores when it has to retry.
#ifndef GURUMIN_CAMERA_UPDATE_VARS_H
#define GURUMIN_CAMERA_UPDATE_VARS_H

#include "key_track.h"
#include "vec4.h"

#include "camera_vars.h"

#define CAMERA_SCRIPT_SLOTS 4

extern "C" KeyBank DAT_118e6ce8[CAMERA_SCRIPT_SLOTS];    // scripted camera: eye keys of each slot
extern "C" KeyBank DAT_111322b8[CAMERA_SCRIPT_SLOTS];    // scripted camera: target keys of each slot
extern "C" int DAT_1101ddf8[CAMERA_SCRIPT_SLOTS];        // kind of each scripted camera (1 = keys by frame, 2 = eased frame)
extern "C" float DAT_10203464[CAMERA_SCRIPT_SLOTS];      // easing exponent of the target keys of each slot
extern "C" KeyBank DAT_11af2420;                         // event camera: eye keys
extern "C" KeyBank DAT_11af2ea4;                         // event camera: target keys
extern "C" Vec4 DAT_118e6cc8;                            // the eye at the start of the free camera step
extern "C" Vec4 DAT_11af3e68;                            // the target at the start of the free camera step
extern "C" Vec4 DAT_11af3ef8;                            // the camera-space push of the previous step
extern "C" int DAT_1101de3c;                             // 1 while the free camera was pushed off a wall by the last step

// The scripted camera's current frame and easing exponent of each slot: DAT_1101de08 and DAT_10203454 are the
// first of CAMERA_SCRIPT_SLOTS consecutive values (camera_vars.h declares them one by one).
inline int &CameraScriptFrame(int slot)
{
    return (&DAT_1101de08)[slot];
}

inline float &CameraScriptExponent(int slot)
{
    return (&DAT_10203454)[slot];
}

#endif
