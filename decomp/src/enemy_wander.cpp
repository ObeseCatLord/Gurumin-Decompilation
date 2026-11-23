// An enemy wandering near its start position (0x10026d80, an enemy move like those in enemy_flee.cpp, used by the
// AI updater 0x100e2080): it picks a heading, steps along it while the way is clear and stays within its range.

#include "char_tables.h"
#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "char_state.h"

#include "last_position.h"

#include "char_vars.h"

#include "enemy_flee.h"
#include "enemy_home.h"

extern "C" int FUN_1013f880(void);                                   // the game's random number
extern "C" int FUN_100241e0(int index, float x, float y, float z);   // Character_SetMove

#define WANDER_PROBE_HEIGHT 10.0f      // the collision probe is lifted by this much
#define WANDER_FIRST_STEP 2.0f         // length of the step tried while the enemy walks on (backwards)
#define WANDER_RETRY_STEP 20.0f        // length of the step tried for a new heading (backwards)
#define WANDER_LOOK_AHEAD 5.0f         // the step is looked ahead by this factor when judging a new heading
#define WANDER_SETTLE 0.1f             // the step taken once a new heading was found
#define WANDER_HEADING_STEPS 360       // random headings are whole degrees
#define WANDER_DEGREES 0.0174551f      // radians per degree
#define WANDER_REHEAD_ROLL 100         // 1 in this a wandering enemy picks a new heading
#define WANDER_PAUSE_ROLL 200          // 1 in this a step that is too far from home still ends the walk
#define WANDER_END_ROLL 300            // 1 in this the move ends
#define WANDER_MOTION 2                // motion played while looking for a heading
#define WANDER_WALK_MOTION 0           // motion played while walking

// 0x10026d80 (Enemy_WanderNearHome). `minimum` is the distance from home that makes it turn back. With the
// state (wanderState) 0 it tries a short step along its heading: it walks on when the way is clear, the step stays
// within its range and may lead home; else it picks state 1 or 2 at random. In states 1 and 2 a 1-in-100 roll (or
// state 2) picks random headings until a step along one may lead home or stays within its range, and then turns to
// it. Returns 1 on a 1-in-300 roll to end the move, else 0.
extern "C" int FUN_10026d80(int index, float minimum)
{
    Vec4 step;
    int blocked = 0;
    DAT_1020b430[index].wait = 0;
    float angle = 0.0f;
    step.Fill(angle);
    if (DAT_1020b430[index].wanderState == 0)
    {
        step.y = -WANDER_FIRST_STEP;
        step.RotateZ(-DAT_10db6ee8[index].targetHeading);
        DAT_10db6ee8[index].matrix.m[12] += step.x;
        DAT_10db6ee8[index].matrix.m[13] += step.y;
        DAT_10db6ee8[index].matrix.m[14] += WANDER_PROBE_HEIGHT;
        if (DAT_10db6ee8[index].IsClearAny(0) == 0 || DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0)
            blocked = 1;
        DAT_10db6ee8[index].matrix.m[12] -= step.x;
        DAT_10db6ee8[index].matrix.m[13] -= step.y;
        DAT_10db6ee8[index].matrix.m[14] -= WANDER_PROBE_HEIGHT;
        float distance = (DAT_10802bf8[index].position - step - DAT_10db6ee8[index].matrix.GetTranslation()).Length2D();
        float homeDistance = (DAT_10802bf8[index].position - DAT_10db6ee8[index].matrix.GetTranslation()).Length2D();
        if (DAT_104bfd08 == 0 && blocked == 0 && FUN_10024d00(index, distance, homeDistance, step, minimum) != 0 &&
            FUN_10024e80(index, step) == 0)
            FUN_100241e0(index, step.x, step.y, 0);
        else if (blocked != 0 || distance > minimum || FUN_1013f880() % WANDER_PAUSE_ROLL == 0)
        {
            int roll = FUN_1013f880() % 3;
            DAT_1020b430[index].wanderState = roll;
            DAT_1020b430[index].wanderState = (roll >= 2) + 1;
        }
        DAT_10db6ee8[index].PlayMotion(WANDER_WALK_MOTION, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].wanderState == 1 || DAT_1020b430[index].wanderState == 2)
    {
        if (FUN_1013f880() % WANDER_REHEAD_ROLL == 0 || DAT_1020b430[index].wanderState == 2)
        {
            angle = (float)(FUN_1013f880() % WANDER_HEADING_STEPS) * WANDER_DEGREES;
            step.y = -WANDER_RETRY_STEP;
            step.RotateZ(-angle);
            DAT_10db6ee8[index].matrix.m[12] += step.x;
            DAT_10db6ee8[index].matrix.m[13] += step.y;
            DAT_10db6ee8[index].matrix.m[14] += WANDER_PROBE_HEIGHT;
            if (DAT_10db6ee8[index].IsClearAny(0) == 0 || DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0)
                blocked = 1;
            DAT_10db6ee8[index].matrix.m[12] -= step.x;
            DAT_10db6ee8[index].matrix.m[13] -= step.y;
            DAT_10db6ee8[index].matrix.m[14] -= WANDER_PROBE_HEIGHT;
            float lookAhead = WANDER_LOOK_AHEAD;
            float distance =
                (DAT_10802bf8[index].position - step * lookAhead - DAT_10db6ee8[index].matrix.GetTranslation())
                    .Length2D();
            float homeDistance = (DAT_10802bf8[index].position - DAT_10db6ee8[index].matrix.GetTranslation()).Length2D();
            while (FUN_10024d00(index, distance, homeDistance, step, minimum) == 0 &&
                   FUN_10024e80(index, step) == 0)
            {
                angle = (float)(FUN_1013f880() % WANDER_HEADING_STEPS) * WANDER_DEGREES;
                step.y = -WANDER_RETRY_STEP;
                step.RotateZ(-angle);
                DAT_10db6ee8[index].matrix.m[12] += step.x;
                DAT_10db6ee8[index].matrix.m[13] += step.y;
                DAT_10db6ee8[index].matrix.m[14] += WANDER_PROBE_HEIGHT;
                if (DAT_10db6ee8[index].IsClearAny(0) == 0 || DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0)
                    blocked = 1;
                float lookAhead = WANDER_LOOK_AHEAD;
                DAT_10db6ee8[index].matrix.m[12] -= step.x;
                DAT_10db6ee8[index].matrix.m[13] -= step.y;
                DAT_10db6ee8[index].matrix.m[14] -= WANDER_PROBE_HEIGHT;
                distance = (DAT_10802bf8[index].position - step * lookAhead -
                            DAT_10db6ee8[index].matrix.GetTranslation())
                               .Length2D();
                homeDistance = (DAT_10802bf8[index].position - DAT_10db6ee8[index].matrix.GetTranslation()).Length2D();
            }
            if (DAT_104bfd08 == 0 && blocked == 0)
            {
                DAT_10db6ee8[index].targetHeading = angle;
                DAT_1020b430[index].wanderState = 0;
                FUN_100241e0(index, step.x * WANDER_SETTLE, step.y * WANDER_SETTLE, 0);
            }
        }
        DAT_10db6ee8[index].PlayMotion(WANDER_MOTION, 0, 5, 1.0f);
    }
    return FUN_1013f880() % WANDER_END_ROLL == 0;
}
