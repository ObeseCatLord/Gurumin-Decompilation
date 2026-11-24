// The 3D sound listener (EN build): follows the camera. This is the one declaration of these globals in decomp_en.
#ifndef GURUMIN_EN_SOUND_LISTENER_H
#define GURUMIN_EN_SOUND_LISTENER_H

#include "vec4.h"

// Sound command that places the listener: volume, position, direction and up vector. EN pointer 0x0090948c (JP FUN_100017c0).
extern "C" int (*g_soundArcSetListener)(float volume, float x, float y, float z, float directionX, float directionY,
                                        float directionZ, float upX, float upY, float upZ);
// Copy of the listener position kept for the other systems. EN 0x00f381d0 (JP DAT_10c67f40).
extern "C" Vec4 g_listenerPosition;
// Copy of the listener's view direction. EN 0x013388e8 (JP DAT_10c67fe0).
extern "C" Vec4 g_listenerDirection;

// Moves the camera towards its goals and updates the listener. EN 0x5b0440 (JP Sound_UpdateListener).
extern "C" int Sound_UpdateListener(void);

#endif
