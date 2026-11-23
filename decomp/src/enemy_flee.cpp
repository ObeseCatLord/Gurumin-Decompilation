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
