// Ledge probes of a character (EN build; JP: ledge_probe.cpp). This is the one declaration of these functions in
// decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_LEDGE_PROBE_H
#define GURUMIN_EN_LEDGE_PROBE_H

#include "char.h"
#include "collide_query.h"
#include "collide_regions.h"
#include "node.h"

// Collision mask of the ground and walls.
const unsigned int LEDGE_MASK = 0x4000000;
// Ray flags of the wall probe.
const int LEDGE_RAY_FLAGS = 4;
// Probe geometry: start above the character, ground search depth, margin above the ground, pull-back from the ground
// point, further reach past the wall hit and the ground search reach under that point.
const float LEDGE_START_UP = 10.0f;
const float LEDGE_GROUND_DEPTH = 100000.0f;
const float LEDGE_NO_GROUND = -200000.0f;
const float LEDGE_GROUND_MARGIN = 10.0f;
const float LEDGE_PULL_BACK = 10.0f;
const float LEDGE_BEYOND_WALL = 100.0f;
const float LEDGE_DROP_REACH = 1000.0f;
// Quarter turn in radians, and the "no wall" distance.
const float LEDGE_QUARTER_TURN = 1.5708f;
const float LEDGE_NO_WALL = -1.0f;

// Casts down from character `index` for the ground (*ground, *drop = its depth below the character, -1 without ground),
// then a ray `reach` units to the character's left from 10 units above that ground. With a wall hit (*wall,
// *wallDistance) it looks for ground 1000 units below a point 100 units further left: returns 2 with *height = the
// floored height of that ground above the wall hit, or 1 without ground there. Returns 0 without ground or wall
// (*wallDistance = -1 when the ground was found). EN 0x5c24a0.
extern "C" int Char_ProbeSideLedge(int index, Vec4 *wall, Vec4 *ground, float *wallDistance, float *height, float *drop, float reach);
#endif
