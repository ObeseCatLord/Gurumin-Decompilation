// An enemy that pulses, wanders and charges (0x100a3900, kind 0x42 of the per-kind update 0x100f9670). It idles with a
// squash-and-stretch pulse, now and then walks off in a random direction (stopping at walls), turns on the player when
// it faces them within 1500 units and, once hit, charges forward for a while. After a charge it ignores the player
// until it is hit again. It leaves a trail effect now and then and wears its node's pulse matrix like Char_PulseScale.

#include <math.h>
#include <stdlib.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "effect_manager.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "map_vars.h"
#include "world.h"

#include "pulse_charger.h"

Matrix4 Scaling(float x, float y, float z);   // 0x101b13c0

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1001a390(int index);                                       // Char_PulseScale
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

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
#define CHARGER_SPEED_HARD_SCALE 0.8f  // DAT_10c635f8 1
#define CHARGER_SPEED_HARDER_SCALE 1.2f  // DAT_10c635f8 2
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

// 0x100a3900 (Enemy_PulseAndCharge). Does nothing once the character is gone (DAT_10210d30) or while it is flagged
// in DAT_102550e0. A hit starts the squash (squashStep 1..6, the node's pulse matrix); at the last step the model
// recovers if it was struck. State 0 binds the root and "Bip01 Pelvis" nodes; state 1 waits 120 frames, then 2 % a
// frame it wanders (state 2): the first frame picks a random heading and looks 250 units along it for a wall, later
// frames walk along it (slower or faster with the difficulty DAT_10c635f8) and every fifth frame look ahead, a wall or
// a step into the unknown (or 1 % a frame after 90 frames) picks wait or wander again. Facing the player within 45
// degrees and 1500 units turns it to state 3: it faces the player until hit, then charges (30 units a frame for 40
// frames) and goes back to wandering, calmed. While trailFrames runs it leaves effect 0x7c0 at its pelvis every frame;
// 1 in 400 frames it calls out and starts 60 of them. Returns 0.
int Enemy_PulseAndCharge(int index)
{
    if (DAT_10210d30[index] != 0)
        return 0;
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float distance = (player - self).Length();
    (player - self).Length2D();
    if (DAT_1020b430[index].squashStep != 0)
    {
        float t = (float)abs(CHARGER_SQUASH_CENTRE - DAT_1020b430[index].squashStep) / CHARGER_SQUASH_DIVISOR *
                      CHARGER_SQUASH_RANGE +
                  CHARGER_SQUASH_BASE;
        if (DAT_1021bdf8[index][0] != 0)
        {
            float widen = CHARGER_SQUASH_WIDEN - t;
            DAT_1021bdf8[index][0]->matrix = Scaling(widen, widen, t * CHARGER_SQUASH_HEIGHT + CHARGER_SQUASH_FLOOR);
        }
        if (++DAT_1020b430[index].squashStep == CHARGER_SQUASH_STEPS)
        {
            DAT_1020b430[index].squashStep = 0;
            if (DAT_1021bdf8[index][0] != 0)
                DAT_1021bdf8[index][0]->matrix.SetIdentity();
        }
    }
    if (DAT_102550e0[index] != 0)
        return 0;
    int skipIdle = 0;   // set by the hit and charge steps: they replace the idle behaviour of the frame
    if (DAT_1020b430[index].state == CHARGER_STEP_IDLE)
    {
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root;
        DAT_1021bdf8[index][1] = DAT_10db6ee8[index].root->Find(CHARGER_PELVIS_NODE, 0);
        if (DAT_1021bdf8[index][0] != 0)
            DAT_1021bdf8[index][0]->matrix.SetIdentity();
        DAT_1020b430[index].state = CHARGER_STEP_WAIT;
        DAT_1020b430[index].wait = 0;
    }
    else if (DAT_1020b430[index].state == CHARGER_STEP_WAIT)
    {
        if (++DAT_1020b430[index].wait > CHARGER_WAIT_FRAMES &&
            FUN_1013f880() % CHARGER_WANDER_ROLL_RANGE < CHARGER_WANDER_ROLL)
        {
            DAT_1020b430[index].state = CHARGER_STEP_WANDER;
            DAT_1020b430[index].wait = 0;
        }
    }
    else if (DAT_1020b430[index].state == CHARGER_STEP_WANDER)
    {
        if (DAT_1020b430[index].wait == 0)
        {
            float degrees = (float)(FUN_1013f880() % CHARGER_HEADING_STEPS);
            DAT_10db6ee8[index].targetHeading = degrees * CHARGER_PI / CHARGER_HALF_TURN;
            Vec4 from;
            Vec4 to;
            Vec4 hit;
            from.Copy(self);
            from.z = from.z + CHARGER_PROBE_LIFT;
            to.y = CHARGER_PROBE_LENGTH;
            to.RotateZ(-DAT_10db6ee8[index].targetHeading);
            to.Copy(to + from);
            DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
            if (DAT_10255f08 != 0 && DAT_10255f08->Collide(0, CHARGER_PROBE_MASK, &from, &to, &hit) != 0)
            {
                DAT_1020b430[index].state = CHARGER_STEP_WAIT;
                DAT_1020b430[index].wait = CHARGER_WALL_WAIT_FRAMES;
            }
            DAT_11b002c0 = 0;
        }
        else
        {
            Vec4 step;
            step.y = CHARGER_WANDER_SPEED;
            step.RotateZ(-DAT_10db6ee8[index].targetHeading);
            if (DAT_10c635f8 == 1)
            {
                float scale = CHARGER_SPEED_HARD_SCALE;
                step.Copy(step * scale);
            }
            else if (DAT_10c635f8 == 2)
            {
                float scale = CHARGER_SPEED_HARDER_SCALE;
                step.Copy(step * scale);
            }
            FUN_100241e0(index, step.x, step.y, 0);
            if (DAT_1020b430[index].wait % CHARGER_WANDER_CHECK_EVERY == 0)
            {
                Vec4 from;
                Vec4 to;
                Vec4 hit;
                from.Copy(self);
                from.z = from.z + CHARGER_PROBE_LIFT;
                to.y = CHARGER_PROBE_LENGTH;
                to.RotateZ(-DAT_10db6ee8[index].targetHeading);
                to.Copy(to + from);
                DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
                if ((DAT_1020b430[index].wait > CHARGER_REPICK_AFTER_FRAMES &&
                     FUN_1013f880() % CHARGER_WANDER_ROLL_RANGE < CHARGER_WANDER_STOP_ROLL) ||
                    (DAT_10255f08 != 0 && DAT_10255f08->Collide(0, CHARGER_PROBE_MASK, &from, &to, &hit) != 0) ||
                    DAT_10db6ee8[index].IsClearAny(&step) == 0)
                {
                    DAT_1020b430[index].state = FUN_1013f880() % 4;
                    if (DAT_1020b430[index].state != CHARGER_STEP_WAIT)
                        DAT_1020b430[index].state = CHARGER_STEP_WANDER;
                    DAT_1020b430[index].wait = -1;
                }
                DAT_11b002c0 = 0;
            }
        }
        DAT_1020b430[index].wait++;
    }
    else if (DAT_1020b430[index].state == CHARGER_STEP_CHARGE)
    {
        if (DAT_1020b430[index].wait < CHARGER_CHARGE_START)
        {
            DAT_10db6ee8[index].targetHeading =
                atan2f(player.x - self.x, player.y - self.y) + CHARGER_PI;
            if (DAT_1020b430[index].squashStep != 0 && DAT_10db6ee8[index].m_238c != 0)
            {
                DAT_1020b430[index].wait = CHARGER_CHARGE_START;
                DAT_10db6ee8[index].PlayMotion(CHARGER_CHARGE_MOTION, 0, CHARGER_MOTION_BLEND, 1.0f);
                DAT_10db6ee8[index].AddMotionSpeed(CHARGER_CHARGE_MOTION_SPEED);
                DAT_10db6ee8[index].matrix.m[14] += CHARGER_CHARGE_HOP;
                DAT_10db6ee8[index].m_238c = 0;
                DAT_10db6ee8[index].m_2388 = 1;
                DAT_10db6ee8[index].m_229c = CHARGER_CHARGE_BLEND;
                DAT_1020b430[index].trailFrames = CHARGER_TRAIL_AFTER_HIT;
                FUN_10065b10(index, CHARGER_CALL_SOUND, 0, 0);
                skipIdle = 1;
            }
        }
        else if (DAT_1020b430[index].wait > CHARGER_CHARGE_START - 1)
        {
            if (DAT_10db6ee8[index].m_238c == 0)
            {
                if (DAT_10db6ee8[index].verticalSpeed > 0.0f)
                    DAT_10db6ee8[index].verticalSpeed += CHARGER_CHARGE_SPEED_UP;
                else
                    DAT_10db6ee8[index].m_237c -= CHARGER_CHARGE_SLOW_DOWN;
            }
            DAT_10db6ee8[index].PlayMotion(CHARGER_IDLE_MOTION, 0, CHARGER_MOTION_BLEND, 1.0f);
            Vec4 step;
            step.y = CHARGER_CHARGE_SPEED;
            step.RotateZ(-DAT_10db6ee8[index].targetHeading);
            if (DAT_10c635f8 == 1)
            {
                float scale = CHARGER_SPEED_HARD_SCALE;
                step.Copy(step * scale);
            }
            else if (DAT_10c635f8 == 2)
            {
                float scale = CHARGER_SPEED_HARDER_SCALE;
                step.Copy(step * scale);
            }
            FUN_100241e0(index, step.x, step.y, 0);
            if (++DAT_1020b430[index].wait > CHARGER_CHARGE_END)
            {
                DAT_1020b430[index].state = CHARGER_STEP_WANDER;
                DAT_1020b430[index].wait = 0;
                DAT_1020b430[index].calmed = 1;
            }
            skipIdle = 1;
        }
    }
    if (skipIdle == 0)
    {
        if (DAT_1020b430[index].squashStep == CHARGER_SQUASH_LAST && DAT_10db6ee8[index].m_238c != 0)
        {
            DAT_10db6ee8[index].AddMotionSpeed(CHARGER_RECOVER_MOTION_SPEED);
            DAT_10db6ee8[index].m_229c = CHARGER_RECOVER_BLEND;
            DAT_1020b430[index].calmed = 0;
        }
        else
        {
            if (DAT_10db6ee8[index].m_229c == 0)
            {
                Vec4 probe;
                probe.z = CHARGER_GROUND_PROBE;
                if (DAT_1020b430[index].squashStep == 0 && DAT_10db6ee8[index].IsClearOfMapLimited(&probe) == 0)
                    FUN_1001a390(index);
            }
            if (DAT_10db6ee8[index].verticalSpeed > 0.0f)
            {
                if (DAT_10db6ee8[index].m_238c == 0)
                    DAT_10db6ee8[index].verticalSpeed += CHARGER_SPEED_UP;
                else if (DAT_1020b430[index].squashStep == 0)
                    FUN_1001a390(index);
            }
            else if (DAT_10db6ee8[index].m_238c == 0)
                DAT_10db6ee8[index].m_237c -= CHARGER_SLOW_DOWN;
            else if (DAT_1020b430[index].squashStep == 0)
                FUN_1001a390(index);
        }
        DAT_10db6ee8[index].PlayMotion(CHARGER_IDLE_MOTION, 0, CHARGER_MOTION_BLEND, 1.0f);
    }
    if (DAT_1020b430[index].state < CHARGER_STEP_CHARGE && DAT_1020b430[index].calmed == 0)
    {
        float turn = atan2f(player.x - self.x, player.y - self.y) + CHARGER_PI;
        turn = DAT_10db6ee8[index].targetHeading - turn;
        while (turn > CHARGER_PI)
            turn -= CHARGER_TWO_PI;
        while (turn < -CHARGER_PI)
            turn += CHARGER_TWO_PI;
        if (floorf(turn) < CHARGER_AIM_ANGLE && distance < CHARGER_AIM_RANGE)
        {
            DAT_1020b430[index].state = CHARGER_STEP_CHARGE;
            DAT_1020b430[index].wait = 0;
            DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CHARGER_PI;
        }
    }
    if (DAT_1020b430[index].trailFrames != 0)
    {
        Vec4 offset;
        Matrix4 matrix;
        matrix.SetIdentity();
        matrix.SetTranslation(self);
        matrix.m[14] += CHARGER_TRAIL_LIFT;
        EffectOptions options;
        options.followOwner = 1;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0008 = 1.0f;
        options.m_000e = 3;
        if (DAT_1021bdf8[index][1] != 0)
            FUN_1001d8b0(index, CHARGER_TRAIL_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][1]->worldMatrix, 0, 0, &matrix,
                         &options);
        else
            FUN_1001d8b0(index, CHARGER_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
        DAT_1020b430[index].trailFrames--;
    }
    if (DAT_1020b430[index].trailFrames == 0)
    {
        if (FUN_1013f880() % CHARGER_WANDER_ROLL_RANGE < CHARGER_TRAIL_ROLL)
        {
            DAT_1020b430[index].trailFrames = CHARGER_TRAIL_FRAMES;
            FUN_10065b10(index, CHARGER_CALL_SOUND, 0, 0);
        }
    }
    DAT_1020b430[index].ageFrames++;
    return 0;
}
