// Line of sight of characters (EN build). Ported from the matched JP source decomp/src/sight_check.cpp.
#include <math.h>

#include "anim_model.h"
#include "node.h"
#include "visibility.h"

// A height difference counts this many times in the distance between two characters.
const float SIGHT_HEIGHT_FACTOR = 2.5f;
// Ray flags and mask of the wall test.
const int SIGHT_RAY_FLAGS = 4;

// Char_CanSee: returns 1 when `eye` (looking along `heading`) sees `target`: their distance, with the height difference
// counted SIGHT_HEIGHT_FACTOR times, is at most `range`, and the target lies within `angleRange` of the heading or within
// `nearRange`. With `checkWalls` the segment from `eye` to `target` must also be free of walls. Else 0. Called by the
// enemy behaviours.
extern "C" int Char_CanSee(const Vec4 *target, const Vec4 *eye, float heading, float range, float angleRange,
                           float nearRange, int checkWalls)
{
    Vec4 offset;
    offset.Copy(*eye - *target);
    float distance;
    {
        Vec4 stretched = offset;
        stretched.z = offset.z * SIGHT_HEIGHT_FACTOR;
        distance = stretched.Length();
    }
    if (distance > range)
        return 0;
    float angle = (float)atan2(offset.x, offset.y);
    angle += ANGLE_HALF_TURN;
    angle -= heading;
    while (angle > ANGLE_HALF_TURN)
        angle -= ANGLE_FULL_TURN;
    while (angle < -ANGLE_HALF_TURN)
        angle += ANGLE_FULL_TURN;
    if ((float)fabs(angle) > angleRange && distance > nearRange)
        return 0;
    if (checkWalls != 0)
    {
        Vec4 hit;
        if (g_mapRootNode != 0 && g_mapRootNode->CastRay(0, SIGHT_RAY_FLAGS, MAP_COLLISION_KIND, (Vec4 *)eye, (Vec4 *)target, &hit, 0) != 0)
            return 0;
    }
    return 1;
}
