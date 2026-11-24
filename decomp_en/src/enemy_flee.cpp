// An enemy backing away from the player (EN build). Ported from the matched JP source decomp/src/enemy_flee.cpp: it
// walks directly away from the player, veering slightly to one side.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_move.h"
#include "engine.h"
#include "enemy_flee.h"
#include "vec4.h"

// CharState::values indices of this move.
const int FLEE_VALUE_WAIT = 0;        // frames the enemy waits
const int FLEE_VALUE_PAUSING = 1;     // 1 = pausing instead of moving
const int FLEE_VALUE_SIDE = 2;        // 1 = veers left, 2 = veers right (picked at random once)
// Side values.
const int FLEE_SIDE_NONE = 0;
const int FLEE_SIDE_LEFT = 1;
const int FLEE_SIDE_RIGHT = 2;
const float FLEE_VEER_ANGLE = 0.0872664f;   // radians the escape direction is turned to one side (about 5 degrees)
const float FLEE_MIN_RANGE = 50.0f;         // the point aimed for is kept this far
const float FLEE_MAX_RANGE = 200.0f;        // ... to this far from the player
const float FLEE_PROBE_HEIGHT = 20.0f;      // the collision probe is lifted by this much
const int FLEE_ROLL_RANGE = 400;            // random rolls are taken mod this
const float FLEE_THROW_SPEED = 18.0f;       // motion speed pushed when the weapon throw is picked
const float FLEE_TURN_PAD = 3.14159f;       // heading offset from the step's direction
// Motion played while walking away: layer, blend start and frames.
const int FLEE_MOTION_LAYER = 1;
const int FLEE_MOTION_FRAMES = 5;
// The player's character slot.
const int FLEE_PLAYER_SLOT = 0;

// Enemy_BackAwayFromPlayer: picks a side to veer to (1 = left, 2 = right, random once) and aims at a point 50..200
// units from the player on the far side from the enemy, walking there at 8 units a frame while facing away (motion 1).
// Returns 1 to end the move when the way 20 units up is blocked and `stopAfter` is below 1, or when a random roll below
// `stopAfter` comes up, or when the step leaves its range; else 0 (the enemy may then throw a weapon, when a random
// roll is below `shootBelow`). Called by the enemy AI updaters.
extern "C" int Enemy_BackAwayFromPlayer(int index, int stopAfter, int shootBelow)
{
    Vec4 step;
    Vec4 player;
    Vec4 self;
    Vec4 away;
    int blocked = 0;
    float distance = 0.0f;
    g_charState[index].values[FLEE_VALUE_WAIT] = 0;
    step.Fill(0.0f);
    player.Copy(g_characterModels[FLEE_PLAYER_SLOT].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    away.Copy(player - self);
    distance = away.Length2D();
    if (g_charState[index].values[FLEE_VALUE_SIDE] == FLEE_SIDE_NONE)
        g_charState[index].values[FLEE_VALUE_SIDE] = Random() % 2 + 1;
    away.Copy(self - player);
    away.z = 0.0f;
    distance = away.Length2D();
    away.Normalized3();
    if (g_charState[index].values[FLEE_VALUE_SIDE] == FLEE_SIDE_LEFT)
        away.RotateZ(FLEE_VEER_ANGLE);
    else if (g_charState[index].values[FLEE_VALUE_SIDE] == FLEE_SIDE_RIGHT)
        away.RotateZ(-FLEE_VEER_ANGLE);
    if (distance > FLEE_MAX_RANGE)
        distance = FLEE_MAX_RANGE;
    else if (distance < FLEE_MIN_RANGE)
        distance = FLEE_MIN_RANGE;
    step.Copy(away * distance + player - self);
    step.z = 0.0f;
    step.Copy(step.Normalized3());
    step.Copy(step * 8.0f);   // 8 units a frame (a literal: Vec4::operator* takes its factor by reference)
    g_characterModels[index].targetHeading = atan2f(step.x, step.y) + FLEE_TURN_PAD;
    g_characterModels[index].PlayMotion(FLEE_MOTION_LAYER, 0, FLEE_MOTION_FRAMES, 1.0f);
    g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] += step.x;
    g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y] += step.y;
    g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] += FLEE_PROBE_HEIGHT;
    if (g_characterModels[index].IsClearAny(0) == 0 || g_characterModels[index].IsClearOfMapLimited(0) == 0)
        blocked = 1;
    g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] -= step.x;
    g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y] -= step.y;
    g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] -= FLEE_PROBE_HEIGHT;
    if (blocked == 0)
    {
        if (g_enemiesFrozen == 0 && Char_OutsideLimit(index, step) == 0)
        {
            Character_SetMove(index, step.x, step.y, 0);
            g_characterModels[index].move.x *= g_characterModels[index].speedScale;
            g_characterModels[index].move.y *= g_characterModels[index].speedScale;
            goto walking;
        }
    }
    else if (stopAfter < 1)
        goto finished;
    if (Random() % FLEE_ROLL_RANGE < stopAfter || Char_OutsideLimit(index, step) != 0)
        goto finished;
walking:
    if (g_characterModels[index].idleMotionEnabled == 1 && Random() % FLEE_ROLL_RANGE < shootBelow)
        g_characterModels[index].PushMotionSpeed(FLEE_THROW_SPEED);
    return 0;
finished:
    g_charState[index].values[FLEE_VALUE_SIDE] = FLEE_SIDE_NONE;
    g_charState[index].values[FLEE_VALUE_PAUSING] = 0;
    g_charState[index].values[FLEE_VALUE_WAIT] = 0;
    return 1;
}
