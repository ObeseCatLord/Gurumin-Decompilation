// An enemy that pulses, wanders and charges (EN build; JP: decomp/src/enemy_pulse_charge.cpp, kind 0x42). It idles with a
// squash-and-stretch pulse, now and then walks off in a random direction (stopping at walls), turns on the player when
// it faces them within 1500 units and, once hit, charges forward for a while. After a charge it ignores the player
// until it is hit again. It leaves a trail effect now and then and wears its node's pulse matrix like Char_PulseScale.
#include <math.h>
#include <stdlib.h>

#include "anim_model.h"
#include "anti_tamper.h"
#include "char.h"
#include "char_inline.h"
#include "char_move.h"
#include "effect_manager.h"
#include "effect_options.h"
#include "enemy_pulse_charge.h"
#include "engine.h"
#include "node.h"
#include "vec4.h"
#include "visibility.h"

// CharState::values indices the pulsing enemy keeps its state in.
const int CHARGER_VALUE_WAIT = 0;         // frames in the current step
const int CHARGER_VALUE_SQUASH = 1;       // squash step: 0 = none, 1..6 running (the node's pulse matrix)
const int CHARGER_VALUE_CALMED = 2;       // 1 after a charge: it ignores the player until it is hit again
const int CHARGER_VALUE_TRAIL = 3;        // frames the trail effect is still left at the pelvis
const int CHARGER_VALUE_AGE = 4;          // frames the enemy has been updated

#define CHARGER_PELVIS_NODE "Bip01 Pelvis"
#define CHARGER_STEP_IDLE 0
#define CHARGER_STEP_WAIT 1            // standing: after 120 frames a 2 % chance per frame to wander
#define CHARGER_STEP_WANDER 2
#define CHARGER_STEP_CHARGE 3          // turned on the player: waits to be hit, then charges
#define CHARGER_SQUASH_STEPS 7         // the squash takes six steps; the seventh restores the node's matrix
#define CHARGER_SQUASH_CENTRE 3
#define CHARGER_SQUASH_LAST 6
#define CHARGER_SQUASH_DIVISOR 3.0f
#define CHARGER_SQUASH_RANGE 0.6f
#define CHARGER_SQUASH_BASE 0.4f
#define CHARGER_SQUASH_WIDEN 2.0f
#define CHARGER_SQUASH_HEIGHT 0.7f
#define CHARGER_SQUASH_FLOOR 0.3f
#define CHARGER_WAIT_FRAMES 0x78
#define CHARGER_WANDER_ROLL_RANGE 10000
#define CHARGER_WANDER_ROLL 200
#define CHARGER_HEADING_STEPS 0x168
#define CHARGER_PI 3.1415f
#define CHARGER_TWO_PI 6.283f
#define CHARGER_HALF_TURN 180.0f
#define CHARGER_PROBE_LIFT 15.0f
#define CHARGER_PROBE_LENGTH (-250.0f)
#define CHARGER_PROBE_MASK 4
#define CHARGER_WANDER_SPEED (-5.0f)
#define CHARGER_CHARGE_SPEED (-30.0f)
#define CHARGER_SPEED_HARD_SCALE 0.8f    // difficulty 1
#define CHARGER_SPEED_HARDER_SCALE 1.2f  // difficulty 2
#define CHARGER_WANDER_CHECK_EVERY 5
#define CHARGER_WALL_WAIT_FRAMES 0x5a
#define CHARGER_WANDER_STOP_ROLL 100
#define CHARGER_REPICK_AFTER_FRAMES 0x5a
#define CHARGER_CHARGE_START 100
#define CHARGER_CHARGE_END 0x8b
#define CHARGER_CHARGE_MOTION 4
#define CHARGER_IDLE_MOTION 2
#define CHARGER_MOTION_BLEND 5
#define CHARGER_CHARGE_MOTION_SPEED 15.0f
#define CHARGER_CHARGE_HOP 5.0f
#define CHARGER_RECOVER_BLEND 8
#define CHARGER_CHARGE_BLEND 5
#define CHARGER_RECOVER_MOTION_SPEED 27.5f
#define CHARGER_GROUND_PROBE (-30.0f)
#define CHARGER_SPEED_UP 1.8f
#define CHARGER_SLOW_DOWN 0.32f
#define CHARGER_CHARGE_SPEED_UP 2.4f
#define CHARGER_CHARGE_SLOW_DOWN 0.58f
#define CHARGER_AIM_ANGLE 0.7854f
#define CHARGER_AIM_RANGE 1500.0f
#define CHARGER_TRAIL_EFFECT 0x7c0
#define CHARGER_TRAIL_LIFT 40.0f
#define CHARGER_TRAIL_FRAMES 0x3c
#define CHARGER_TRAIL_AFTER_HIT 0x78
#define CHARGER_TRAIL_ROLL 25
#define CHARGER_CALL_SOUND 0x121
#define CHARGER_WANDER_PICKS 4         // after a stop: roll of this many, 1 = wait, else wander on

// Enemy_PulseAndCharge. Does nothing once the character is gone (g_charPhase) or while it is flagged in g_charHitLevel. A
// hit starts the squash (squash step 1..6, the node's pulse matrix); at the last step the model recovers if it was
// struck. State 0 binds the root and "Bip01 Pelvis" nodes; state 1 waits 120 frames, then 2 % a frame it wanders
// (state 2): the first frame picks a random heading and looks 250 units along it for a wall, later frames walk along it
// (slower or faster with the difficulty) and every fifth frame look ahead, a wall or a step into the unknown (or 1 % a
// frame after 90 frames) picks wait or wander again. Facing the player within 45 degrees and 1500 units turns it to
// state 3: it faces the player until hit, then charges (30 units a frame for 40 frames) and goes back to wandering,
// calmed. While the trail runs it leaves effect 0x7c0 at its pelvis every frame; 1 in 400 frames it calls out and
// starts 60 of them. Returns 0. Called by the per-kind updater (kind 0x42).
int Enemy_PulseAndCharge(int index)
{
    if (g_charPhase[index] != 0)
        return 0;
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    float distance = (player - self).Length();
    (player - self).Length2D();
    if (g_charState[index].values[CHARGER_VALUE_SQUASH] != 0)
    {
        float t = (float)abs(CHARGER_SQUASH_CENTRE - g_charState[index].values[CHARGER_VALUE_SQUASH]) /
                      CHARGER_SQUASH_DIVISOR * CHARGER_SQUASH_RANGE +
                  CHARGER_SQUASH_BASE;
        if (g_charNodes[index][0] != 0)
        {
            float widen = CHARGER_SQUASH_WIDEN - t;
            g_charNodes[index][0]->matrix = ScaleMatrix(widen, widen, t * CHARGER_SQUASH_HEIGHT + CHARGER_SQUASH_FLOOR);
        }
        if (++g_charState[index].values[CHARGER_VALUE_SQUASH] == CHARGER_SQUASH_STEPS)
        {
            g_charState[index].values[CHARGER_VALUE_SQUASH] = 0;
            if (g_charNodes[index][0] != 0)
                g_charNodes[index][0]->matrix.SetIdentity();
        }
    }
    if (g_charHitLevel[index] != 0)
        return 0;
    int skipIdle = 0;   // set by the hit and charge steps: they replace the idle behaviour of the frame
    if (g_charState[index].state == CHARGER_STEP_IDLE)
    {
        g_charNodes[index][0] = g_characterModels[index].root;
        g_charNodes[index][1] = g_characterModels[index].root->Find(CHARGER_PELVIS_NODE, 0);
        if (g_charNodes[index][0] != 0)
            g_charNodes[index][0]->matrix.SetIdentity();
        g_charState[index].state = CHARGER_STEP_WAIT;
        g_charState[index].values[CHARGER_VALUE_WAIT] = 0;
    }
    else if (g_charState[index].state == CHARGER_STEP_WAIT)
    {
        int waited = g_charState[index].values[CHARGER_VALUE_WAIT] + 1;
        g_charState[index].values[CHARGER_VALUE_WAIT] = waited;
        if (waited > CHARGER_WAIT_FRAMES &&
            Random() % CHARGER_WANDER_ROLL_RANGE < CHARGER_WANDER_ROLL)
        {
            g_charState[index].state = CHARGER_STEP_WANDER;
            g_charState[index].values[CHARGER_VALUE_WAIT] = 0;
        }
    }
    else if (g_charState[index].state == CHARGER_STEP_WANDER)
    {
        if (g_charState[index].values[CHARGER_VALUE_WAIT] == 0)
        {
            float degrees = (float)(Random() % CHARGER_HEADING_STEPS);
            g_characterModels[index].targetHeading = degrees * CHARGER_PI / CHARGER_HALF_TURN;
            Vec4 from;
            Vec4 to;
            Vec4 hit;
            from.Copy(self);
            from.z = from.z + CHARGER_PROBE_LIFT;
            to.y = CHARGER_PROBE_LENGTH;
            to.RotateZ(-g_characterModels[index].targetHeading);
            to.Copy(to + from);
            g_collisionFilter = g_characterModels[index].collisionFilter;
            if (g_mapRootNode != 0 && g_mapRootNode->Collide(0, CHARGER_PROBE_MASK, &from, &to, &hit) != 0)
            {
                g_charState[index].state = CHARGER_STEP_WAIT;
                g_charState[index].values[CHARGER_VALUE_WAIT] = CHARGER_WALL_WAIT_FRAMES;
            }
            g_collisionFilter = 0;
        }
        else
        {
            Vec4 step;
            step.y = CHARGER_WANDER_SPEED;
            step.RotateZ(-g_characterModels[index].targetHeading);
            if (g_guardedValue8 == 1)
            {
                float scale = CHARGER_SPEED_HARD_SCALE;
                step.Copy(step * scale);
            }
            else if (g_guardedValue8 == 2)
            {
                float scale = CHARGER_SPEED_HARDER_SCALE;
                step.Copy(step * scale);
            }
            Character_SetMove(index, step.x, step.y, 0);
            if (g_charState[index].values[CHARGER_VALUE_WAIT] % CHARGER_WANDER_CHECK_EVERY == 0)
            {
                Vec4 from;
                Vec4 to;
                Vec4 hit;
                from.Copy(self);
                from.z = from.z + CHARGER_PROBE_LIFT;
                to.y = CHARGER_PROBE_LENGTH;
                to.RotateZ(-g_characterModels[index].targetHeading);
                to.Copy(to + from);
                g_collisionFilter = g_characterModels[index].collisionFilter;
                if ((g_charState[index].values[CHARGER_VALUE_WAIT] > CHARGER_REPICK_AFTER_FRAMES &&
                     Random() % CHARGER_WANDER_ROLL_RANGE < CHARGER_WANDER_STOP_ROLL) ||
                    (g_mapRootNode != 0 && g_mapRootNode->Collide(0, CHARGER_PROBE_MASK, &from, &to, &hit) != 0) ||
                    g_characterModels[index].IsClearAny(&step) == 0)
                {
                    g_charState[index].state = Random() % CHARGER_WANDER_PICKS;
                    if (g_charState[index].state != CHARGER_STEP_WAIT)
                        g_charState[index].state = CHARGER_STEP_WANDER;
                    g_charState[index].values[CHARGER_VALUE_WAIT] = -1;
                }
                g_collisionFilter = 0;
            }
        }
        g_charState[index].values[CHARGER_VALUE_WAIT]++;
    }
    else if (g_charState[index].state == CHARGER_STEP_CHARGE)
    {
        if (g_charState[index].values[CHARGER_VALUE_WAIT] < CHARGER_CHARGE_START)
        {
            g_characterModels[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CHARGER_PI;
            if (g_charState[index].values[CHARGER_VALUE_SQUASH] != 0 && g_characterModels[index].idleMotionEnabled != 0)
            {
                g_charState[index].values[CHARGER_VALUE_WAIT] = CHARGER_CHARGE_START;
                g_characterModels[index].PlayMotion(CHARGER_CHARGE_MOTION, 0, CHARGER_MOTION_BLEND, 1.0f);
                g_characterModels[index].AddMotionSpeed(CHARGER_CHARGE_MOTION_SPEED);
                g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] += CHARGER_CHARGE_HOP;
                g_characterModels[index].idleMotionEnabled = 0;
                g_characterModels[index].speedChangeFrames = 1;
                g_characterModels[index].blendFrames = CHARGER_CHARGE_BLEND;
                g_charState[index].values[CHARGER_VALUE_TRAIL] = CHARGER_TRAIL_AFTER_HIT;
                Character_PlaySound(index, CHARGER_CALL_SOUND, 0, 0);
                skipIdle = 1;
            }
        }
        else if (g_charState[index].values[CHARGER_VALUE_WAIT] > CHARGER_CHARGE_START - 1)
        {
            if (g_characterModels[index].idleMotionEnabled == 0)
            {
                if (g_characterModels[index].verticalSpeed > 0.0f)
                    g_characterModels[index].verticalSpeed += CHARGER_CHARGE_SPEED_UP;
                else
                    g_characterModels[index].m_237c -= CHARGER_CHARGE_SLOW_DOWN;
            }
            g_characterModels[index].PlayMotion(CHARGER_IDLE_MOTION, 0, CHARGER_MOTION_BLEND, 1.0f);
            Vec4 step;
            step.y = CHARGER_CHARGE_SPEED;
            step.RotateZ(-g_characterModels[index].targetHeading);
            if (g_guardedValue8 == 1)
            {
                float scale = CHARGER_SPEED_HARD_SCALE;
                step.Copy(step * scale);
            }
            else if (g_guardedValue8 == 2)
            {
                float scale = CHARGER_SPEED_HARDER_SCALE;
                step.Copy(step * scale);
            }
            Character_SetMove(index, step.x, step.y, 0);
            int charged = g_charState[index].values[CHARGER_VALUE_WAIT] + 1;
            g_charState[index].values[CHARGER_VALUE_WAIT] = charged;
            if (charged > CHARGER_CHARGE_END)
            {
                g_charState[index].state = CHARGER_STEP_WANDER;
                g_charState[index].values[CHARGER_VALUE_WAIT] = 0;
                g_charState[index].values[CHARGER_VALUE_CALMED] = 1;
            }
            skipIdle = 1;
        }
    }
    if (skipIdle == 0)
    {
        if (g_charState[index].values[CHARGER_VALUE_SQUASH] == CHARGER_SQUASH_LAST &&
            g_characterModels[index].idleMotionEnabled != 0)
        {
            g_characterModels[index].AddMotionSpeed(CHARGER_RECOVER_MOTION_SPEED);
            g_characterModels[index].blendFrames = CHARGER_RECOVER_BLEND;
            g_charState[index].values[CHARGER_VALUE_CALMED] = 0;
        }
        else
        {
            if (g_characterModels[index].blendFrames == 0)
            {
                Vec4 probe;
                probe.z = CHARGER_GROUND_PROBE;
                if (g_charState[index].values[CHARGER_VALUE_SQUASH] == 0 &&
                    g_characterModels[index].IsClearOfMapLimited(&probe) == 0)
                    Char_PulseScale(index);
            }
            if (g_characterModels[index].verticalSpeed > 0.0f)
            {
                if (g_characterModels[index].idleMotionEnabled == 0)
                    g_characterModels[index].verticalSpeed += CHARGER_SPEED_UP;
                else if (g_charState[index].values[CHARGER_VALUE_SQUASH] == 0)
                    Char_PulseScale(index);
            }
            else if (g_characterModels[index].idleMotionEnabled == 0)
                g_characterModels[index].m_237c -= CHARGER_SLOW_DOWN;
            else if (g_charState[index].values[CHARGER_VALUE_SQUASH] == 0)
                Char_PulseScale(index);
        }
        g_characterModels[index].PlayMotion(CHARGER_IDLE_MOTION, 0, CHARGER_MOTION_BLEND, 1.0f);
    }
    if (g_charState[index].state < CHARGER_STEP_CHARGE && g_charState[index].values[CHARGER_VALUE_CALMED] == 0)
    {
        float turn = atan2f(player.x - self.x, player.y - self.y) + CHARGER_PI;
        turn = g_characterModels[index].targetHeading - turn;
        while (turn > CHARGER_PI)
            turn -= CHARGER_TWO_PI;
        while (turn < -CHARGER_PI)
            turn += CHARGER_TWO_PI;
        if ((float)fabs(turn) < CHARGER_AIM_ANGLE && distance < CHARGER_AIM_RANGE)
        {
            g_charState[index].state = CHARGER_STEP_CHARGE;
            g_charState[index].values[CHARGER_VALUE_WAIT] = 0;
            g_characterModels[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CHARGER_PI;
        }
    }
    if (g_charState[index].values[CHARGER_VALUE_TRAIL] != 0)
    {
        Vec4 offset;
        Matrix4 matrix;
        matrix.SetIdentity();
        matrix.SetTranslation(self);
        matrix.m[MATRIX_TRANSLATION_Z] += CHARGER_TRAIL_LIFT;
        EffectOptions options;
        options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
        options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
        options.scale = 1.0f;
        options.m_000e = 3;
        if (g_charNodes[index][1] != 0)
            Char_StartEffectInline(index, CHARGER_TRAIL_EFFECT, offset, -1, -1, &g_charNodes[index][1]->worldMatrix, 0, 0,
                                   &matrix, &options);
        else
            Char_StartEffectInline(index, CHARGER_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
        g_charState[index].values[CHARGER_VALUE_TRAIL]--;
    }
    if (g_charState[index].values[CHARGER_VALUE_TRAIL] == 0)
    {
        if (Random() % CHARGER_WANDER_ROLL_RANGE < CHARGER_TRAIL_ROLL)
        {
            g_charState[index].values[CHARGER_VALUE_TRAIL] = CHARGER_TRAIL_FRAMES;
            Character_PlaySound(index, CHARGER_CALL_SOUND, 0, 0);
        }
    }
    g_charState[index].values[CHARGER_VALUE_AGE]++;
    return 0;
}
