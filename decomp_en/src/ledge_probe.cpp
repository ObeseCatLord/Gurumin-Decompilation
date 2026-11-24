// Ledge probes of a character (EN build). JP twin: ledge_probe.cpp.
#include <math.h>

#include "anim_model.h"
#include "ledge_probe.h"

// Char_ProbeSideLedge: see the declaration in ledge_probe.h. Called while a character walks along a ledge.
extern "C" int Char_ProbeSideLedge(int index, Vec4 *wall, Vec4 *ground, float *wallDistance, float *height, float *drop, float reach)
{
    Vec4 from;
    Vec4 side;
    Vec4 forward;
    Vec4 wallHit;
    from.x = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
    from.y = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y];
    from.z = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] + LEDGE_START_UP;
    from.w = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_W];
    side.Fill(0.0f);
    side.x = reach;
    side.RotateZ(-g_characterModels[index].heading - LEDGE_QUARTER_TURN);
    forward.Copy(side);
    forward.Normalized3();
    *drop = LEDGE_NO_WALL;
    {
        Vec4 top;
        Vec4 bottom;
        Vec4 unused;
        Vec4 floor;
        floor.Fill(0.0f);
        floor.x = LEDGE_NO_GROUND;
        top.x = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
        top.y = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y];
        top.z = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z];
        top.w = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_W];
        bottom.Copy(top);
        bottom.z -= LEDGE_GROUND_DEPTH;
        if (g_worldRoot != 0)
            g_worldRoot->CastSegment(0, LEDGE_MASK, &bottom, &top, &floor, 0);
        from.Copy(floor);
        if (floor.x == LEDGE_NO_GROUND)
            return 0;
    }
    ground->Copy(from);
    *drop = (float)fabs(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] - from.z);
    from.z += LEDGE_GROUND_MARGIN;
    {
        float pullBack = LEDGE_PULL_BACK;
        from.Copy(from - forward * pullBack);
    }
    side.Copy(side + from);
    float distance = LEDGE_NO_WALL;
    if (g_worldRoot != 0 && g_worldRoot->CastRay(0, LEDGE_RAY_FLAGS, LEDGE_MASK, &side, &from, &wallHit, 0) != 0)
        distance = (wallHit - from).Length();
    if (distance == LEDGE_NO_WALL)
    {
        if (wallDistance != 0)
            *wallDistance = LEDGE_NO_WALL;
        return 0;
    }
    if (wall != 0)
        wall->Copy(wallHit);
    if (wallDistance != 0)
        *wallDistance = distance;
    side.Fill(0.0f);
    side.x = LEDGE_BEYOND_WALL;
    side.RotateZ(-g_characterModels[index].heading - LEDGE_QUARTER_TURN);
    from.Copy(wallHit + side);
    if (Collision_ProbeDown(from, &forward, LEDGE_DROP_REACH) == 0)
        return 1;
    if (height != 0)
        *height = (float)fabs(forward.z - wallHit.z);
    return 2;
}

// Char_ProbeSideStep: like Char_ProbeSideStep, but starting 40 units above the character and probing for ground both 100
// units to the left of the wall hit (near) and 100 units to the right (far). Returns 1 when there is no near ground or the
// far ground is not higher than the near ground; otherwise 2 with *height = the height of the near ground above the wall hit.
// Returns 0 without ground or wall. Called while a character walks along a ledge.
extern "C" int Char_ProbeSideStep(int index, Vec4 *wall, Vec4 *ground, float *wallDistance, float *height, float *drop, float reach)
{
    Vec4 from;
    Vec4 side;
    Vec4 forward;
    Vec4 wallHit;
    from.x = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
    from.y = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y];
    from.z = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] + STEP_START_UP;
    from.w = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_W];
    side.Fill(0.0f);
    side.x = reach;
    side.RotateZ(-g_characterModels[index].heading - LEDGE_QUARTER_TURN);
    forward.Copy(side);
    forward.Normalized3();
    *drop = LEDGE_NO_WALL;
    {
        Vec4 top;
        Vec4 bottom;
        Vec4 unused;
        Vec4 floor;
        floor.Fill(0.0f);
        floor.x = LEDGE_NO_GROUND;
        top.x = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
        top.y = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y];
        top.z = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z];
        top.w = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_W];
        bottom.Copy(top);
        bottom.z -= LEDGE_GROUND_DEPTH;
        if (g_worldRoot != 0)
            g_worldRoot->CastSegment(0, LEDGE_MASK, &bottom, &top, &floor, 0);
        from.Copy(floor);
        if (floor.x == LEDGE_NO_GROUND)
            return 0;
    }
    ground->Copy(from);
    *drop = (float)fabs(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] - from.z);
    from.z += LEDGE_GROUND_MARGIN;
    {
        float pullBack = LEDGE_PULL_BACK;
        from.Copy(from - forward * pullBack);
    }
    side.Copy(side + from);
    float distance = LEDGE_NO_WALL;
    if (g_worldRoot != 0 && g_worldRoot->CastRay(0, LEDGE_RAY_FLAGS, LEDGE_MASK, &side, &from, &wallHit, 0) != 0)
        distance = (wallHit - from).Length();
    if (distance == LEDGE_NO_WALL)
    {
        if (wallDistance != 0)
            *wallDistance = LEDGE_NO_WALL;
        return 0;
    }
    if (wall != 0)
        wall->Copy(wallHit);
    if (wallDistance != 0)
        *wallDistance = distance;
    side.Fill(0.0f);
    side.x = LEDGE_BEYOND_WALL;
    side.RotateZ(-g_characterModels[index].heading - LEDGE_QUARTER_TURN);
    from.Copy(wallHit + side);
    int nearFound = Collision_ProbeDown(from, &forward, LEDGE_DROP_REACH);
    from.Copy(wallHit - side);
    int farFound = Collision_ProbeDown(from, &side, LEDGE_DROP_REACH);
    if (nearFound == 0 || (farFound == 1 && forward.z - wallHit.z >= side.z - wallHit.z))
        return 1;
    if (height != 0)
        *height = (float)fabs(forward.z - wallHit.z);
    return 2;
}
