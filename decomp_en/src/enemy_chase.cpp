// An enemy chasing the player (EN build). Ported from the JP source decomp/src/enemy_chase.cpp (APP.dll 0x10027630,
// unmatched there) and the EN bytes: it runs towards the player with a speed that eases towards its wanted speed.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_move.h"
#include "char_shots.h"
#include "enemy_ai.h"
#include "enemy_flee.h"
#include "engine.h"
#include "vec4.h"

// CharState::values entries of this move.
const int CHASE_VALUE_WAIT = 0;          // frames the enemy waits
const int CHASE_VALUE_PAUSING = 1;       // 1 = pausing instead of chasing
const int CHASE_VALUE_PAUSE_FRAMES = 2;  // frames left of the pause
const int CHASE_VALUE_RUNNING = 3;       // 1 = running (after the player was far)
// Wanted step lengths while walking and while running (the y of the step before it is turned to the heading).
const float CHASE_WALK_STEP = -2.0f;
const float CHASE_RUN_STEP = -8.0f;
// The speed eases by this much a frame towards the wanted step length.
const float CHASE_SPEED_EASE = 0.1f;
// The probe is lifted by this much to see whether the way is clear.
const float CHASE_PROBE_HEIGHT = 20.0f;
// The move ends when the player is farther than this, and the random roll that ends it with `wander` is one in this.
const float CHASE_LEAVE_RANGE = 500.0f;
const int CHASE_WANDER_ROLL = 400;
// Running stops below this distance and starts above the other.
const float CHASE_RUN_STOP_RANGE = 200.0f;
const float CHASE_RUN_START_RANGE = 300.0f;
// Below this step length the walk-in-place motion plays.
const float CHASE_STILL_STEP = 5.0f;
// Heading offset from the direction to the player (the enemy faces away from it).
const float CHASE_TURN_PAD = 3.1415f;
// Motions: walking in place (layer 0) and walking (layer 1); blend start and frames.
const int CHASE_MOTION_STILL = 0;
const int CHASE_MOTION_WALK = 1;
const int CHASE_MOTION_FRAMES = 5;
// The player's character slot, and the g_shotParams column the speed is kept in.
const int CHASE_PLAYER_SLOT = 0;
const int CHASE_PARAM_SPEED = 0;

// Runs enemy `index` towards the player at a speed easing by 0.1 a frame towards 2 (8 once the player was more than
// 300 units away, back to 2 within 200), facing the player (motion 1, or 0 when barely moving). Returns 1 to end the
// move when the way 20 units up is blocked (unless `canGiveUp`), the player is more than 500 units away, at random with
// `canGiveUp` (1 in 400) or when the step leaves its range; else 0. While pausing it only counts down. Called by the
// enemy AI updaters EN 0x5ba6d0 and 0x5bab60.
extern "C" int Enemy_ChasePlayer(int index, int canGiveUp)
{
    Vec4 step;
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    int blocked = 0;
    float distance = 0.0f;
    g_charState[index].values[CHASE_VALUE_WAIT] = 0;
    step.Fill(distance);
    player.Copy(g_characterModels[CHASE_PLAYER_SLOT].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    distance = toPlayer.Length2D();
    if (g_charState[index].values[CHASE_VALUE_PAUSING] == 0)
    {
        float turn = -g_characterModels[index].heading;
        step.y = CHASE_WALK_STEP;
        if (g_charState[index].values[CHASE_VALUE_RUNNING] != 0)
            step.y = CHASE_RUN_STEP;
        step.RotateZ(turn);
        float *speed = &g_shotParams[index][CHASE_PARAM_SPEED];
        if (step.Length() > *speed)
            *speed += CHASE_SPEED_EASE;
        else if (step.Length() < *speed)
            *speed -= CHASE_SPEED_EASE;
        step.Normalized3();
        step.Copy(step * *speed);
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] += step.x;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y] += step.y;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] += CHASE_PROBE_HEIGHT;
        if (g_characterModels[index].IsClearAny(0) == 0 || g_characterModels[index].IsClearOfMapLimited(0) == 0)
            blocked = 1;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] -= step.x;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y] -= step.y;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] -= CHASE_PROBE_HEIGHT;
        if ((blocked != 0 && canGiveUp == 0) || distance > CHASE_LEAVE_RANGE ||
            (canGiveUp != 0 && Random() % CHASE_WANDER_ROLL == 0) || Char_OutsideLimit(index, step) != 0)
            return 1;
        g_characterModels[index].targetHeading = atan2f(toPlayer.x, toPlayer.y) + CHASE_TURN_PAD;
        if (g_enemiesFrozen == 0)
        {
            Character_SetMove(index, step.x, step.y, 0);
            g_characterModels[index].move.x = g_characterModels[index].speedScale * g_characterModels[index].move.x;
            g_characterModels[index].move.y *= g_characterModels[index].speedScale;
        }
        if (distance < CHASE_RUN_STOP_RANGE)
            g_charState[index].values[CHASE_VALUE_RUNNING] = 0;
        else if (distance > CHASE_RUN_START_RANGE)
            g_charState[index].values[CHASE_VALUE_RUNNING] = 1;
        if (step.Length() < CHASE_STILL_STEP)
        {
            g_characterModels[index].PlayMotion(CHASE_MOTION_STILL, 0, CHASE_MOTION_FRAMES, 1.0f);
            return 0;
        }
        g_characterModels[index].PlayMotion(CHASE_MOTION_WALK, 0, CHASE_MOTION_FRAMES, 1.0f);
        return 0;
    }
    if (g_charState[index].values[CHASE_VALUE_PAUSING] == 1)
    {
        if (g_charState[index].values[CHASE_VALUE_PAUSE_FRAMES] > 0)
            g_charState[index].values[CHASE_VALUE_PAUSE_FRAMES]--;
        if (g_charState[index].values[CHASE_VALUE_PAUSE_FRAMES] == 0)
            g_charState[index].values[CHASE_VALUE_PAUSING] = 0;
    }
    return 0;
}
