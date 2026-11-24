// Balloons (EN build; JP: decomp/src/balloon_burst.cpp).
#ifndef GURUMIN_EN_BALLOON_H
#define GURUMIN_EN_BALLOON_H

#include "vec4.h"

// Lets every balloon that has not burst react to a blast at `blast` (direction and push; a strong blast bursts it).
// Returns 0. EN 0x5e85f0 (JP 0x100543a0).
extern "C" int Balloon_Blast(Vec4 blast);

#endif
