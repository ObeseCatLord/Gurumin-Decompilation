// Footstep effects and sounds of the player (EN build, no JP twin). Declared once here.
#ifndef GURUMIN_EN_FOOTSTEP_H
#define GURUMIN_EN_FOOTSTEP_H

#include "vec4.h"

// Surface flags the ground probe sets (to 3) for the surface under the player; they pick the step effect and sound.
extern "C" int g_stepSurfaceE;          // EN 0x00a1517c: effect 0x2da, sounds 0x3c0 + step
extern "C" int g_stepSurfaceA;          // EN 0x00a15178: sounds 0x227 + step, no effect
extern "C" int g_stepSurfaceC;          // EN 0x00a15174: effect 0x2e0, sounds 0x25c + step
extern "C" int g_stepSurfaceD;          // EN 0x00a15170: effects 0x2dc and 0x88bd, sounds 0x258 + step
extern "C" int g_stepSurfaceB;          // EN 0x00a15280: sounds 0 + step only
// Set to 1 while steps are silenced (cleared by the scene start). EN 0x00b91eaa.
extern "C" char g_stepsSilenced;
// Counts the steps taken; its low two bits pick one of four sounds. EN 0x00a207d2.
extern "C" unsigned char g_stepCounter;

// Plays the effect and sound of one footstep at `*position` for the surface the flags above name. Does nothing in the
// first frames of the game, in scene-timer range 30000-30999, while steps are silenced or sounds are muted, or when the
// game is not in play mode 1. The first two arguments are not used. EN 0x63f790.
void Footstep_Play(int unused1, int unused2, const Vec4 *position);

#endif
