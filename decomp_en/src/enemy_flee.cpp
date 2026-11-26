// An enemy backing away from the player (EN build). Ported from the matched JP source decomp/src/enemy_flee.cpp: it
// walks directly away from the player, veering slightly to one side.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_move.h"
#include "char_shots.h"
#include "enemy_ai.h"
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

// Enemy_KeepDistanceFromPlayer constants. CharState::values indices (the first is FLEE_VALUE_WAIT).
const int KEEP_VALUE_PAUSING = 1;      // 1 = pausing instead of moving
const int KEEP_VALUE_PAUSE_FRAMES = 2; // frames left of the pause
const int KEEP_VALUE_RUNNING = 3;      // 0 = walking, 1 = running (after the player was far), 2 = standing
const int KEEP_STATE_WALKING = 0;
const int KEEP_STATE_RUNNING = 1;
const int KEEP_STATE_STANDING = 2;
// Speed of the step column of g_shotParams the move eases.
const int KEEP_PARAM_SPEED = 0;
const float KEEP_STEP_FAR = 3.0f;            // wanted speed while backing off slowly
const float KEEP_STEP_NEAR = 10.0f;          // wanted speed when near or running
const float KEEP_SPEED_EASE = 0.15f;         // the speed eases this much a frame
const float KEEP_CLOSE_RANGE = 220.0f;       // within this the step is stretched and the enemy turns round
const float KEEP_CLOSE_STRETCH = 1.25f;      // step factor within KEEP_CLOSE_RANGE
const float KEEP_LEAVE_RANGE = 500.0f;       // the move ends when the player is farther than this
const float KEEP_RUN_BELOW = 150.0f;         // running starts below this distance
const float KEEP_WALK_ABOVE = 250.0f;        // walking resumes below this distance (between 200 and 250)
const float KEEP_RUN_ABOVE = 300.0f;         // running starts above this distance
const float KEEP_STAND_BELOW = 230.0f;       // standing between KEEP_CLOSE_RANGE and this distance
const float KEEP_WALK_STEP = 5.0f;           // below this step length the walk-in-place motion plays
const int KEEP_ROLL_PAUSE = 300;             // 1-in-300 roll for a pause with a throw motion
const int KEEP_PAUSE_LONG = 0x92;            // frames of the first throw pause
const int KEEP_PAUSE_SHORT = 0x5d;           // frames of the second throw pause
// Motions: walking in place, walking, standing, the two throws.
const int KEEP_MOTION_WALK_IN_PLACE = 0;
const int KEEP_MOTION_WALK = 1;
const int KEEP_MOTION_STAND = 2;
const int KEEP_MOTION_THROW_A = 5;
const int KEEP_MOTION_THROW_B = 6;
// Odd or even pick of the throw motion.
const int KEEP_THROW_CHOICES = 2;
// Half a turn the heading gets while the enemy turns round.
const float KEEP_HEADING_HALF_TURN = 3.1415f;

// Moves relative to the player by distance: with `running` 0 and the player more than 200 units away it backs off
// slowly (3), nearer or with `running` 1 faster (10), at exactly 200 with `running` 2 it stands. The step eases (0.15
// a frame) towards its wanted speed and is stretched by 1.25 within 220 units; it faces the player (turned fully
// round within 220). `running` follows the distance (1 below 150 or above 300, 0 between 150 and 250, 2 and the
// motion stops between 220 and 230 units). In state 2 a 1-in-300 roll every so often pauses it with a throw motion
// (5 or 6) for 146 or 93 frames. Returns 1 to end the move when the player is more than 500 units away, at random
// with `arg` (1 in 400) or when the step leaves its range; else 0. While pausing it only counts down. Called by the
// enemy AI updaters.
extern "C" int Enemy_KeepDistanceFromPlayer(int index, int arg)
{
    Vec4 step;
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    g_charState[index].values[FLEE_VALUE_WAIT] = 0;
    step.Fill(0.0f);
    player.Copy(g_characterModels[FLEE_PLAYER_SLOT].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    float distance = toPlayer.Length2D();
    if (g_charState[index].values[KEEP_VALUE_PAUSING] == 0)
    {
        if (g_charState[index].values[KEEP_VALUE_RUNNING] == KEEP_STATE_WALKING && distance > FLEE_MAX_RANGE)
        {
            step.y = -KEEP_STEP_FAR;
            step.RotateZ(-g_characterModels[index].heading);
        }
        else if (g_charState[index].values[KEEP_VALUE_RUNNING] == KEEP_STATE_RUNNING || distance < FLEE_MAX_RANGE)
        {
            step.y = -KEEP_STEP_NEAR;
            step.RotateZ(-g_characterModels[index].heading);
        }
        else
            step.Fill(0.0f);
        if (step.Length() > g_shotParams[index][KEEP_PARAM_SPEED])
        {
            g_shotParams[index][KEEP_PARAM_SPEED] += KEEP_SPEED_EASE;
            if (distance < FLEE_MAX_RANGE)
                g_shotParams[index][KEEP_PARAM_SPEED] += KEEP_SPEED_EASE;
        }
        else if (step.Length() < g_shotParams[index][KEEP_PARAM_SPEED])
        {
            g_shotParams[index][KEEP_PARAM_SPEED] -= KEEP_SPEED_EASE;
            if (distance < FLEE_MAX_RANGE)
                g_shotParams[index][KEEP_PARAM_SPEED] += KEEP_SPEED_EASE;
        }
        if (step.x != 0.0f || step.y != 0.0f || step.z != 0.0f)
        {
            step.Normalized3();
            step.Copy(step * g_shotParams[index][KEEP_PARAM_SPEED]);
        }
        if (distance < KEEP_CLOSE_RANGE)
        {
            float stretch = KEEP_CLOSE_STRETCH;
            step.Copy(step * stretch);
        }
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] += step.x;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y] += step.y;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] += FLEE_PROBE_HEIGHT;
        if (g_characterModels[index].IsClearAny(0) != 0)
            g_characterModels[index].IsClearOfMapLimited(0);
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] -= step.x;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y] -= step.y;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] -= FLEE_PROBE_HEIGHT;
        if (distance > KEEP_LEAVE_RANGE || (arg != 0 && Random() % FLEE_ROLL_RANGE == 0) ||
            Char_OutsideLimit(index, step) != 0)
            return 1;
        g_characterModels[index].targetHeading = atan2f(toPlayer.x, toPlayer.y) + KEEP_HEADING_HALF_TURN;
        if (distance < KEEP_CLOSE_RANGE)
            g_characterModels[index].targetHeading = g_characterModels[index].targetHeading + FLEE_TURN_PAD;
        if (g_enemiesFrozen == 0)
        {
            Character_SetMove(index, step.x, step.y, 0);
            g_characterModels[index].move.x *= g_characterModels[index].speedScale;
            g_characterModels[index].move.y = g_characterModels[index].speedScale * g_characterModels[index].move.y;
        }
        if (distance < KEEP_RUN_BELOW)
            g_charState[index].values[KEEP_VALUE_RUNNING] = KEEP_STATE_RUNNING;
        else if (distance < FLEE_MAX_RANGE)
            g_charState[index].values[KEEP_VALUE_RUNNING] = KEEP_STATE_WALKING;
        else if (distance < KEEP_WALK_ABOVE)
            g_charState[index].values[KEEP_VALUE_RUNNING] = KEEP_STATE_WALKING;
        else if (distance > KEEP_RUN_ABOVE)
            g_charState[index].values[KEEP_VALUE_RUNNING] = KEEP_STATE_RUNNING;
        if (distance > KEEP_CLOSE_RANGE && distance < KEEP_STAND_BELOW)
        {
            g_characterModels[index].velocity.Fill(0.0f);
            g_characterModels[index].move.Fill(0.0f);
            g_charState[index].values[KEEP_VALUE_RUNNING] = KEEP_STATE_STANDING;
        }
        if (step.Length() < KEEP_WALK_STEP)
            g_characterModels[index].PlayMotion(KEEP_MOTION_WALK_IN_PLACE, 0, FLEE_MOTION_FRAMES, 1.0f);
        else
            g_characterModels[index].PlayMotion(KEEP_MOTION_WALK, 0, FLEE_MOTION_FRAMES, 1.0f);
        if (g_charState[index].values[KEEP_VALUE_RUNNING] == KEEP_STATE_STANDING)
            g_characterModels[index].PlayMotion(KEEP_MOTION_STAND, 0, FLEE_MOTION_FRAMES, 1.0f);
        if ((Random() % KEEP_ROLL_PAUSE != 0 || g_charState[index].values[KEEP_VALUE_RUNNING] == KEEP_STATE_STANDING) &&
            Random() % KEEP_ROLL_PAUSE == 0 && g_charState[index].values[KEEP_VALUE_RUNNING] == KEEP_STATE_STANDING)
        {
            switch (Random() % KEEP_THROW_CHOICES)
            {
            case 0:
                g_characterModels[index].PlayMotion(KEEP_MOTION_THROW_A, 0, FLEE_MOTION_FRAMES, 1.0f);
                g_charState[index].values[KEEP_VALUE_PAUSE_FRAMES] = KEEP_PAUSE_LONG;
                g_charState[index].values[KEEP_VALUE_PAUSING] = 1;
                return 0;
            case 1:
                g_characterModels[index].PlayMotion(KEEP_MOTION_THROW_B, 0, FLEE_MOTION_FRAMES, 1.0f);
                g_charState[index].values[KEEP_VALUE_PAUSE_FRAMES] = KEEP_PAUSE_SHORT;
                g_charState[index].values[KEEP_VALUE_PAUSING] = 1;
                return 0;
            }
        }
        return 0;
    }
    if (g_charState[index].values[KEEP_VALUE_PAUSING] == 1)
    {
        if (g_charState[index].values[KEEP_VALUE_PAUSE_FRAMES] > 0)
            g_charState[index].values[KEEP_VALUE_PAUSE_FRAMES]--;
        if (g_charState[index].values[KEEP_VALUE_PAUSE_FRAMES] == 0)
            g_charState[index].values[KEEP_VALUE_PAUSING] = 0;
    }
    return 0;
}
