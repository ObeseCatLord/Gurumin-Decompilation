// An enemy backing away from the player (0x100279b0, an enemy move like those in enemy_chase.cpp, used by the AI
// updaters 0x100e2110 and 0x100e2560): it walks directly away from the player, veering slightly to one side.

#include "char_tables.h"
#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "char_state.h"

#include "char_vars.h"

#include "enemy_flee.h"

extern "C" int FUN_1013f880(void);            // the game's random number
extern "C" int FUN_100241e0(int index, float x, float y, float z);   // Character_SetMove

#define FLEE_VEER_ANGLE 0.0872664f         // radians the escape direction is turned to one side (about 5 degrees)
#define FLEE_MIN_RANGE 50.0f            // the point aimed for is kept this far
#define FLEE_MAX_RANGE 200.0f           // ... to this far from the player
#define FLEE_SPEED 8.0f                 // units a frame
#define FLEE_PROBE_HEIGHT 20.0f         // the collision probe is lifted by this much
#define FLEE_ROLL_RANGE 400             // random rolls are taken mod this
#define FLEE_THROW_SPEED 18.0f          // motion speed pushed when the weapon throw is picked
#define FLEE_TURN_PAD 3.14159f          // heading offset from the step's direction

// 0x100279b0 (Enemy_BackAwayFromPlayer). Picks a side to veer to (fleeSide 1 = left, 2 = right, random once) and
// aims at a point 50..200 units from the player on the far side from the enemy, walking there at 8 units a frame
// while facing away (motion 1). Returns 1 to end the move when the way 20 units up is blocked and `stopAfter` is
// below 1, or when a random roll below `stopAfter` comes up, or when the step leaves its range; else 0 (the enemy
// may then throw a weapon, when a random roll is below `shootBelow`).
extern "C" int FUN_100279b0(int index, int stopAfter, int shootBelow)
{
    Vec4 step;
    Vec4 player;
    Vec4 self;
    Vec4 away;
    int blocked = 0;
    float distance = 0.0f;
    DAT_1020b430[index].wait = 0;
    step.Fill(0.0f);
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    away.Copy(player - self);
    distance = away.Length2D();
    if (DAT_1020b430[index].fleeSide == 0)
        DAT_1020b430[index].fleeSide = FUN_1013f880() % 2 + 1;
    away.Copy(self - player);
    away.z = 0.0f;
    distance = away.Length2D();
    away.Normalized3();
    if (DAT_1020b430[index].fleeSide == 1)
        away.RotateZ(FLEE_VEER_ANGLE);
    else if (DAT_1020b430[index].fleeSide == 2)
        away.RotateZ(-FLEE_VEER_ANGLE);
    if (distance > FLEE_MAX_RANGE)
        distance = FLEE_MAX_RANGE;
    else if (distance < FLEE_MIN_RANGE)
        distance = FLEE_MIN_RANGE;
    step.Copy(away * distance + player - self);
    step.z = 0.0f;
    step.Copy(step.Normalized3());
    step.Copy(step * FLEE_SPEED);
    DAT_10db6ee8[index].targetHeading = atan2f(step.x, step.y) + FLEE_TURN_PAD;
    DAT_10db6ee8[index].PlayMotion(1, 0, 5, 1.0f);
    DAT_10db6ee8[index].matrix.m[12] += step.x;
    DAT_10db6ee8[index].matrix.m[13] += step.y;
    DAT_10db6ee8[index].matrix.m[14] += FLEE_PROBE_HEIGHT;
    if (DAT_10db6ee8[index].IsClearAny(0) == 0 || DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0)
        blocked = 1;
    DAT_10db6ee8[index].matrix.m[12] -= step.x;
    DAT_10db6ee8[index].matrix.m[13] -= step.y;
    DAT_10db6ee8[index].matrix.m[14] -= FLEE_PROBE_HEIGHT;
    if (blocked == 0)
    {
        if (DAT_104bfd08 == 0 && FUN_10024e80(index, step) == 0)
        {
            FUN_100241e0(index, step.x, step.y, 0);
            DAT_10db6ee8[index].move.x *= DAT_10db6ee8[index].speedScale;
            DAT_10db6ee8[index].move.y *= DAT_10db6ee8[index].speedScale;
            goto walking;
        }
    }
    else if (stopAfter < 1)
        goto finished;
    if (FUN_1013f880() % FLEE_ROLL_RANGE < stopAfter || FUN_10024e80(index, step) != 0)
        goto finished;
walking:
    if (DAT_10db6ee8[index].m_238c == 1 && FUN_1013f880() % FLEE_ROLL_RANGE < shootBelow)
        DAT_10db6ee8[index].PushMotionSpeed(FLEE_THROW_SPEED);
    return 0;
finished:
    DAT_1020b430[index].fleeSide = 0;
    DAT_1020b430[index].pausing = 0;
    DAT_1020b430[index].wait = 0;
    return 1;
}

#define KEEP_STEP_FAR 3.0f
#define KEEP_STEP_NEAR 10.0f
#define KEEP_SPEED_EASE 0.15f
#define KEEP_CLOSE_RANGE 220.0f
#define KEEP_CLOSE_STRETCH 1.25f
#define KEEP_LEAVE_RANGE 500.0f
#define KEEP_WALK_STEP 5.0f
#define KEEP_ROLL_PAUSE 300
#define KEEP_PAUSE_LONG 0x92
#define KEEP_PAUSE_SHORT 0x5d

// 0x10027da0 (Enemy_KeepDistanceFromPlayer). Moves relative to the player by distance: with `running` 0 and the
// player more than 200 units away it backs off slowly (3), nearer or with `running` 1 faster (10), at exactly 200
// with `running` 2 it stands. The step eases (0.15 a frame) towards its wanted speed and is stretched by 1.25 within
// 220 units; it faces the player (turned fully round within 220). `running` follows the distance (1 below 150 or
// above 300, 0 between 150 and 250, 2 and the motion stops between 220 and 230 units). In state 2 a 1-in-300 roll
// every so often pauses it with a throw motion (5 or 6) for 146 or 93 frames. Returns 1 to end the move when the
// player is more than 500 units away, at random with `chanceEnd` (1 in 400) or when the step leaves its range;
// else 0. While pausing it only counts down.
extern "C" int FUN_10027da0(int index, int chanceEnd)
{
    Vec4 step;
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    DAT_1020b430[index].wait = 0;
    step.Fill(0.0f);
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    float distance = toPlayer.Length2D();
    if (DAT_1020b430[index].pausing == 0)
    {
        if (DAT_1020b430[index].running == 0 && distance > FLEE_MAX_RANGE)
        {
            step.y = -KEEP_STEP_FAR;
            step.RotateZ(-DAT_10db6ee8[index].heading);
        }
        else if (DAT_1020b430[index].running == 1 || distance < FLEE_MAX_RANGE)
        {
            step.y = -KEEP_STEP_NEAR;
            step.RotateZ(-DAT_10db6ee8[index].heading);
        }
        else
            step.Fill(0.0f);
        float *speed = &DAT_10255160[index][0];
        if (step.Length() > *speed)
        {
            *speed += KEEP_SPEED_EASE;
            if (distance < FLEE_MAX_RANGE)
                *speed += KEEP_SPEED_EASE;
        }
        else if (step.Length() < *speed)
        {
            *speed -= KEEP_SPEED_EASE;
            if (distance < FLEE_MAX_RANGE)
                *speed += KEEP_SPEED_EASE;
        }
        if (step.x != 0.0f || step.y != 0.0f || step.z != 0.0f)
        {
            step.Normalized3();
            step.Copy(step * *speed);
        }
        if (distance < KEEP_CLOSE_RANGE)
        {
            float stretch = KEEP_CLOSE_STRETCH;
            step.Copy(step * stretch);
        }
        DAT_10db6ee8[index].matrix.m[12] += step.x;
        DAT_10db6ee8[index].matrix.m[13] += step.y;
        DAT_10db6ee8[index].matrix.m[14] += FLEE_PROBE_HEIGHT;
        if (DAT_10db6ee8[index].IsClearAny(0) != 0)
            DAT_10db6ee8[index].IsClearOfMapLimited(0);
        DAT_10db6ee8[index].matrix.m[12] -= step.x;
        DAT_10db6ee8[index].matrix.m[13] -= step.y;
        DAT_10db6ee8[index].matrix.m[14] -= FLEE_PROBE_HEIGHT;
        if (distance > KEEP_LEAVE_RANGE || (chanceEnd != 0 && FUN_1013f880() % FLEE_ROLL_RANGE == 0) ||
            FUN_10024e80(index, step) != 0)
            return 1;
        DAT_10db6ee8[index].targetHeading = atan2f(toPlayer.x, toPlayer.y) + 3.1415f;
        if (distance < KEEP_CLOSE_RANGE)
            DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading + FLEE_TURN_PAD;
        if (DAT_104bfd08 == 0)
        {
            FUN_100241e0(index, step.x, step.y, 0);
            DAT_10db6ee8[index].move.x *= DAT_10db6ee8[index].speedScale;
            DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.y;
        }
        if (distance < 150.0f)
            DAT_1020b430[index].running = 1;
        else if (distance < FLEE_MAX_RANGE)
            DAT_1020b430[index].running = 0;
        else if (distance < 250.0f)
            DAT_1020b430[index].running = 0;
        else if (distance > 300.0f)
            DAT_1020b430[index].running = 1;
        if (distance > KEEP_CLOSE_RANGE && distance < 230.0f)
        {
            DAT_10db6ee8[index].velocity.Fill(0.0f);
            DAT_10db6ee8[index].move.Fill(0.0f);
            DAT_1020b430[index].running = 2;
        }
        DAT_10db6ee8[index].PlayMotion(step.Length() < KEEP_WALK_STEP ? 0 : 1, 0, 5, 1.0f);
        if (DAT_1020b430[index].running == 2)
            DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        if ((FUN_1013f880() % KEEP_ROLL_PAUSE != 0 || DAT_1020b430[index].running == 2) &&
            FUN_1013f880() % KEEP_ROLL_PAUSE == 0 && DAT_1020b430[index].running == 2)
        {
            switch (FUN_1013f880() % 2)
            {
            case 0:
                DAT_10db6ee8[index].PlayMotion(5, 0, 5, 1.0f);
                DAT_1020b430[index].pauseFrames = KEEP_PAUSE_LONG;
                DAT_1020b430[index].pausing = 1;
                return 0;
            case 1:
                DAT_10db6ee8[index].PlayMotion(6, 0, 5, 1.0f);
                DAT_1020b430[index].pauseFrames = KEEP_PAUSE_SHORT;
                DAT_1020b430[index].pausing = 1;
                return 0;
            }
        }
        return 0;
    }
    if (DAT_1020b430[index].pausing == 1)
    {
        if (DAT_1020b430[index].pauseFrames > 0)
            DAT_1020b430[index].pauseFrames--;
        if (DAT_1020b430[index].pauseFrames == 0)
            DAT_1020b430[index].pausing = 0;
    }
    return 0;
}
