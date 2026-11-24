// An enemy that stalks the player and lunges at him with a swing of its hand (0x100ad0f0, a per-kind behaviour of
// 0x100f9670). It follows the same stalk / circle / retreat steps as the fire breather (enemy_breather.cpp) but closes
// in much further and, when told to react, runs one of two melee lunges that leave a streak and a swing effect.

#include <math.h>

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
#include "last_position.h"
#include "map_vars.h"
#include "world.h"

#include "barrage_enemy.h"
#include "lunge_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1001a450(int slot, const Vec4 *point);                     // Slot_HasNoGround
extern "C" int FUN_1001a590(int slot, const Vec4 *unused, float distance);    // ground probe along the slot's angle
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            const Matrix4 *target, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define LUNGE_NULL_NODE "@efc_Null"
#define LUNGE_HAND_NODE "Bip01 R Finger0"
#define LUNGE_STEP_START 0
#define LUNGE_STEP_STALK 1             // faces the player and walks up to him while he is far
#define LUNGE_STEP_CIRCLE 2            // keeps its distance, strafing; goes back to stalking when he is far
#define LUNGE_STEP_ATTACK_A 3          // the first lunge (chosen three times in four)
#define LUNGE_STEP_ATTACK_B 4          // the second lunge
#define LUNGE_STEP_RETREAT 5           // jumps back a random way
#define LUNGE_PI 3.1415f
#define LUNGE_HALF_TURN 180.0f
#define LUNGE_FIFTH_TURN 1.2566f       // the angle a circling step is rotated by
#define LUNGE_FAR 350.0f               // stalking turns into circling when the player is nearer than this
#define LUNGE_CIRCLE_FAR 370.0f        // circling turns back into stalking beyond this
#define LUNGE_CIRCLE_NEAR 250.0f       // circling may turn into a retreat nearer than this
#define LUNGE_WALK_SPEED (-12.0f)
#define LUNGE_CIRCLE_SPEED (-6.0f)
#define LUNGE_PROBE_HEIGHT 30.0f       // the move vector's height while a step is tested for ground and walls
#define LUNGE_NO_GROUND_SCALE 0.1f     // a character with no ground below it moves at a tenth of the speed
#define LUNGE_RETREAT_SLOW (-5.0f)
#define LUNGE_RETREAT_FAST (-35.0f)
#define LUNGE_JUMP_SPEED 30.0f
#define LUNGE_HOP_SPEED 20.0f
#define LUNGE_PROBE_DISTANCE 200.0f
#define LUNGE_PROBE_TRIES 5
#define LUNGE_RETREAT_FRAMES 0x14
#define LUNGE_RETREAT_SLOW_END 5
#define LUNGE_RETREAT_FAST_END 0x10
#define LUNGE_RETREAT_HOP_FRAME 6
#define LUNGE_FAILED_RETREAT 10
#define LUNGE_RETREAT_SPREAD 0xb4
#define LUNGE_RETREAT_HALF 0x5a
#define LUNGE_MOTION_WALK 1
#define LUNGE_MOTION_RETREAT 0x11
#define LUNGE_HOP_SOUND 0x1c6
#define LUNGE_ATTACK_ODDS 4            // a reaction picks the first lunge unless a roll of this comes up 0
#define LUNGE_STRAFE_MODES 3
#define LUNGE_STRAFE_AFTER 0x3c
#define LUNGE_STRAFE_ROLL_RANGE 1000
#define LUNGE_STRAFE_ROLL 0x50
#define LUNGE_STRAFE_LEFT 0
#define LUNGE_STRAFE_RIGHT 1
#define LUNGE_ROLL_RANGE 10000
#define LUNGE_ALARM_ROLL 100
#define LUNGE_RETREAT_ROLL 400
#define LUNGE_GREETING_ROLL 1000
#define LUNGE_UNSET_TALK (-1)
#define LUNGE_UNSET_RANGE (-2)
#define LUNGE_LEVEL_LIMIT 10
#define LUNGE_TURN_FRAMES 5            // counts after the wind-up frame in which it still turns to the player
#define LUNGE_DASH_END 0x28            // count at which an attack is over
#define LUNGE_TRAIL_EFFECT 0x455       // the streak left while it lunges
#define LUNGE_TRAIL_MODE 5
#define LUNGE_FOLLOW_OWNER 2           // EffectOptions flag: the effect follows its owner
#define LUNGE_BOOST_SPEED 15.0f        // the motion speed added when the dash gets fast
#define LUNGE_DASH_SLOW (-15.0f)
#define LUNGE_DASH_FAST (-25.0f)
#define LUNGE_DASH_FASTEST (-30.0f)

// The first lunge: wind-up frame 0x131 of motion 8, the dash runs from count 11 to 27 and the swing effect starts
// at motion frame 0x13b.
#define LUNGE_A_MOTION 8
#define LUNGE_A_WINDUP_FRAME 0x131
#define LUNGE_A_WINDUP_SOUND 0x1d2
#define LUNGE_A_DASH_FROM 11
#define LUNGE_A_KICK_COUNT 0xd         // counts 11 and 12 are the slow dash, this one the kick-off, then it runs
#define LUNGE_A_RUN_END 0x1c
#define LUNGE_A_FASTEST_UNTIL 0xf      // the fastest dash lasts until this count
#define LUNGE_A_TRAIL_SIZE 1.4f
#define LUNGE_A_SWING_FRAME 0x13b
#define LUNGE_A_SWING_EFFECT 0x370b
// The second lunge: wind-up frame 0xeb of motion 6, the dash runs from count 10 to 22, swing at motion frame 0xf4.
#define LUNGE_B_MOTION 6
#define LUNGE_B_WINDUP_FRAME 0xeb
#define LUNGE_B_WINDUP_SOUND 0x1d3
#define LUNGE_B_DASH_FROM 10
#define LUNGE_B_KICK_COUNT 0xc
#define LUNGE_B_RUN_END 0x17
#define LUNGE_B_FASTEST_UNTIL 0xe
#define LUNGE_B_TRAIL_SIZE 2.0f
#define LUNGE_B_SWING_FRAME 0xf4
#define LUNGE_B_SWING_EFFECT 0x3708

// 0x100ad0f0 (Enemy_StalkAndLunge). The sibling of Enemy_StalkAndBreathe with the same stalking, circling and
// retreating steps (it comes much closer: stalking ends at 350 units) but a melee attack instead of the breath.
// A reaction request (or a roll while circling) starts state 3 (three times in four) or state 4: it stands in its
// wind-up motion until the motion reaches the wind-up frame, plays a sound, keeps turning to the player for five
// counts, then dashes at him (slow, a boost, then fast) leaving a streak from the "@efc_Null" node, and starts a
// swing effect from "Bip01 R Finger0" at one frame of the motion. The attack ends at count 40. Every state may
// produce a move, which is applied at the end together with the motion. Returns 0.
int Enemy_StalkAndLunge(int index)
{
    Vec4 player;
    Vec4 self;
    Vec4 move;
    int motion = -1;
    int moving = 0;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float amount = (player - self).Length2D();
    float delta;   // the player's y offset while stalking, later the no-ground slow-down factor
    DAT_10255a88[index] = 0;
    if (DAT_10210b30[index] != 0 && DAT_1020b430[index].state != 0)
    {
        DAT_1020b430[index].state = FUN_1013f880() % LUNGE_ATTACK_ODDS != 0 ? LUNGE_STEP_ATTACK_A : LUNGE_STEP_ATTACK_B;
        DAT_1020b430[index].wait = 0;
    }
    DAT_10210b30[index] = 0;
    if (FUN_1001a450(index, &self) != 0)
        DAT_1020b430[index].stalled = 1;
    if (DAT_102550e0[index] > LUNGE_LEVEL_LIMIT)
        DAT_102550e0[index] = LUNGE_LEVEL_LIMIT;
    if (DAT_10468730[index] != 0)
    {
        DAT_10468730[index] = 0;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(LUNGE_NULL_NODE, 0);
    }
    if (DAT_102550e0[index] == 0)
    {
        if (DAT_1020b430[index].state == LUNGE_STEP_START)
        {
            DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(LUNGE_NULL_NODE, 0);
            DAT_1020b430[index].wait = 0;
            if (DAT_1020b430[index].talk == 0)
                DAT_1020b430[index].talk = LUNGE_UNSET_TALK;
            if (DAT_1020b430[index].range == 0)
                DAT_1020b430[index].range = LUNGE_UNSET_RANGE;
            DAT_1020b430[index].state = LUNGE_STEP_STALK;
        }
        else if (DAT_1020b430[index].state == LUNGE_STEP_STALK)
        {
            delta = player.y - self.y;
            float dx = player.x - self.x;
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)dx, (double)delta) + LUNGE_PI;
            if (amount > LUNGE_FAR)
            {
                move.y = LUNGE_WALK_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                moving = 1;
                motion = LUNGE_MOTION_WALK;
                move.z = LUNGE_PROBE_HEIGHT;
                if (DAT_10db6ee8[index].m_238c != 0)
                {
                    if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) != 0 &&
                        DAT_10db6ee8[index].IsClearAny(&move) != 0 && DAT_10467920[index] == 0)
                    {
                        DAT_1020b430[index].stalled = 0;
                    }
                    else if (DAT_1020b430[index].stalled != 0)
                    {
                        DAT_1020b430[index].state = LUNGE_STEP_RETREAT;
                        DAT_1020b430[index].wait = 0;
                    }
                    else
                    {
                        DAT_10db6ee8[index].AddMotionSpeed(LUNGE_JUMP_SPEED);
                        DAT_1020b430[index].stalled = 1;
                    }
                }
                move.z = 0;
            }
            else
            {
                DAT_1020b430[index].state = LUNGE_STEP_CIRCLE;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == LUNGE_STEP_CIRCLE)
        {
            DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + LUNGE_PI;
            if (DAT_1020b430[index].strafeMode == LUNGE_STRAFE_LEFT)
            {
                move.y = LUNGE_CIRCLE_SPEED;
                move.RotateZ(LUNGE_FIFTH_TURN - DAT_10db6ee8[index].heading);
                moving = 1;
                motion = 0;
            }
            else if (DAT_1020b430[index].strafeMode == LUNGE_STRAFE_RIGHT)
            {
                move.y = LUNGE_CIRCLE_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading - LUNGE_FIFTH_TURN);
                moving = 1;
                motion = 0;
            }
            else
                motion = 2;
            if (++DAT_1020b430[index].wait > LUNGE_STRAFE_AFTER &&
                FUN_1013f880() % LUNGE_STRAFE_ROLL_RANGE < LUNGE_STRAFE_ROLL)
            {
                DAT_1020b430[index].strafeMode = FUN_1013f880() % LUNGE_STRAFE_MODES;
                DAT_1020b430[index].wait = 0;
            }
            if (DAT_10467920[index] != 0 || FUN_1013f880() % LUNGE_ROLL_RANGE < LUNGE_ALARM_ROLL)
                DAT_10210b30[index] = 1;
            if (amount > LUNGE_CIRCLE_FAR)
            {
                DAT_1020b430[index].state = LUNGE_STEP_STALK;
                DAT_1020b430[index].wait = 0;
            }
            if (amount < LUNGE_CIRCLE_NEAR && FUN_1013f880() % LUNGE_ROLL_RANGE < LUNGE_RETREAT_ROLL)
            {
                DAT_1020b430[index].state = LUNGE_STEP_RETREAT;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == LUNGE_STEP_ATTACK_A)
        {
            motion = LUNGE_A_MOTION;
            if (DAT_10db6ee8[index].motionFrame == LUNGE_A_WINDUP_FRAME)
            {
                DAT_1020b430[index].wait = 1;
                FUN_10065b10(index, LUNGE_A_WINDUP_SOUND, 0, 0);
            }
            if (DAT_1020b430[index].wait != 0)
                DAT_1020b430[index].wait++;
            if (DAT_1020b430[index].wait < LUNGE_TURN_FRAMES)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + LUNGE_PI;
            if (DAT_1020b430[index].wait >= LUNGE_A_DASH_FROM)
            {
                if (DAT_1020b430[index].wait < LUNGE_A_KICK_COUNT)
                {
                    move.y = LUNGE_DASH_SLOW;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                }
                else if (DAT_1020b430[index].wait == LUNGE_A_KICK_COUNT)
                {
                    move.y = LUNGE_DASH_FAST;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    DAT_10db6ee8[index].AddMotionSpeed(LUNGE_BOOST_SPEED);
                }
                else if (DAT_1020b430[index].wait < LUNGE_A_RUN_END)
                {
                    move.y = LUNGE_DASH_FASTEST;
                    if (DAT_1020b430[index].wait >= LUNGE_A_FASTEST_UNTIL)
                        move.y = LUNGE_DASH_FAST;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    EffectOptions options;
                    options.m_0000 = 0;
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0008 = LUNGE_A_TRAIL_SIZE;
                    options.m_000e = LUNGE_TRAIL_MODE;
                    Vec4 offset;
                    if (DAT_1021bdf8[index][0] != 0)
                    {
                        options.owner = index + EFFECT_OWNER_BASE;
                        FUN_1001d8b0(index, LUNGE_TRAIL_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0,
                                     0, 0, &options);
                    }
                }
                else if (DAT_1020b430[index].wait == LUNGE_DASH_END)
                {
                    DAT_1020b430[index].state = LUNGE_STEP_STALK;
                    DAT_1020b430[index].wait = 0;
                }
            }
            if (DAT_10db6ee8[index].motionFrame == LUNGE_A_SWING_FRAME && DAT_1021bdf8[index][0] != 0 &&
                DAT_10db6ee8[index].root->Find(LUNGE_HAND_NODE, 0) != 0)
            {
                Matrix4 swing;
                EffectOptions options;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0000 = 0;
                options.m_0008 = LUNGE_A_TRAIL_SIZE;
                options.m_000e = LUNGE_TRAIL_MODE;
                options.followOwner = 1;
                Vec4 offset;
                swing = RotationZ(DAT_10db6ee8[index].heading);
                options.owner = index + EFFECT_OWNER_BASE;
                FUN_1001d8b0(index, LUNGE_A_SWING_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                             &DAT_10db6ee8[index].root->Find(LUNGE_HAND_NODE, 0)->worldMatrix, 0, &swing, &options);
            }
        }
        else if (DAT_1020b430[index].state == LUNGE_STEP_ATTACK_B)
        {
            motion = LUNGE_B_MOTION;
            if (DAT_10db6ee8[index].motionFrame == LUNGE_B_WINDUP_FRAME)
            {
                DAT_1020b430[index].wait = 1;
                FUN_10065b10(index, LUNGE_B_WINDUP_SOUND, 0, 0);
            }
            if (DAT_1020b430[index].wait != 0)
                DAT_1020b430[index].wait++;
            if (DAT_1020b430[index].wait < LUNGE_TURN_FRAMES)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + LUNGE_PI;
            if (DAT_1020b430[index].wait >= LUNGE_B_DASH_FROM)
            {
                if (DAT_1020b430[index].wait < LUNGE_B_KICK_COUNT)
                {
                    move.y = LUNGE_DASH_SLOW;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                }
                else if (DAT_1020b430[index].wait == LUNGE_B_KICK_COUNT)
                {
                    move.y = LUNGE_DASH_FAST;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    DAT_10db6ee8[index].AddMotionSpeed(LUNGE_BOOST_SPEED);
                }
                else if (DAT_1020b430[index].wait < LUNGE_B_RUN_END)
                {
                    move.y = LUNGE_DASH_FASTEST;
                    if (DAT_1020b430[index].wait >= LUNGE_B_FASTEST_UNTIL)
                        move.y = LUNGE_DASH_FAST;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    EffectOptions options;
                    options.m_0000 = 0;
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0008 = LUNGE_B_TRAIL_SIZE;
                    options.m_000e = LUNGE_TRAIL_MODE;
                    Vec4 offset;
                    if (DAT_1021bdf8[index][0] != 0)
                    {
                        options.owner = index + EFFECT_OWNER_BASE;
                        FUN_1001d8b0(index, LUNGE_TRAIL_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0,
                                     0, 0, &options);
                    }
                }
                else if (DAT_1020b430[index].wait == LUNGE_DASH_END)
                {
                    DAT_1020b430[index].state = LUNGE_STEP_STALK;
                    DAT_1020b430[index].wait = 0;
                }
            }
            if (DAT_10db6ee8[index].motionFrame == LUNGE_B_SWING_FRAME && DAT_1021bdf8[index][0] != 0 &&
                DAT_10db6ee8[index].root->Find(LUNGE_HAND_NODE, 0) != 0)
            {
                Vec4 offset;
                EffectOptions options;
                Matrix4 swing;
                options.followOwner = 1;
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = LUNGE_B_TRAIL_SIZE;
                options.m_000e = LUNGE_TRAIL_MODE;
                swing = RotationZ(DAT_10db6ee8[index].heading);
                options.owner = index + EFFECT_OWNER_BASE;
                FUN_1001d8b0(index, LUNGE_B_SWING_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                             &DAT_10db6ee8[index].root->Find(LUNGE_HAND_NODE, 0)->worldMatrix, 0, &swing, &options);
            }
        }
        else if (DAT_1020b430[index].state == LUNGE_STEP_RETREAT)
        {
            if (DAT_1020b430[index].wait == 0)
            {
                int tries = 0;
                do
                {
                    DAT_10255160[index][0] =
                        (float)(FUN_1013f880() % LUNGE_RETREAT_SPREAD - LUNGE_RETREAT_HALF) * LUNGE_PI /
                            LUNGE_HALF_TURN +
                        LUNGE_PI;
                    if (FUN_1001a590(index, &self, LUNGE_PROBE_DISTANCE) != 0)
                        break;
                    tries++;
                } while (tries < LUNGE_PROBE_TRIES);
                if (tries == LUNGE_PROBE_TRIES)
                {
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].retreatFrames = LUNGE_FAILED_RETREAT;
                    DAT_1020b430[index].state = LUNGE_STEP_STALK;
                    return 0;
                }
            }
            if (DAT_1020b430[index].wait == LUNGE_RETREAT_HOP_FRAME)
            {
                FUN_10065b10(index, LUNGE_HOP_SOUND, 0, 0);
                DAT_10db6ee8[index].AddMotionSpeed(LUNGE_HOP_SPEED);
            }
            if (DAT_1020b430[index].wait < LUNGE_RETREAT_SLOW_END || DAT_1020b430[index].wait > 0xf)
                move.y = LUNGE_RETREAT_SLOW;
            else if (DAT_1020b430[index].wait < LUNGE_RETREAT_FAST_END)
                move.y = LUNGE_RETREAT_FAST;
            move.RotateZ(DAT_10255160[index][0] - DAT_10db6ee8[index].heading);
            moving = 1;
            motion = LUNGE_MOTION_RETREAT;
            if (++DAT_1020b430[index].wait > LUNGE_RETREAT_FRAMES)
            {
                DAT_1020b430[index].state = LUNGE_STEP_STALK;
                DAT_1020b430[index].wait = 0;
            }
        }
    }
    if (DAT_1020b430[index].state < 2 && DAT_104c00e0 != 0 && FUN_1013f880() % LUNGE_ROLL_RANGE < LUNGE_GREETING_ROLL)
    {
        DAT_1020b430[index].state = LUNGE_STEP_RETREAT;
        DAT_1020b430[index].wait = 0;
    }
    if (moving != 0)
    {
        if (DAT_10467920[index] != 0)
        {
            delta = LUNGE_NO_GROUND_SCALE;
            move.Copy(move * delta);
        }
        FUN_100241e0(index, move.x, move.y, move.z);
        DAT_10802bf8[index].position.Copy(move);
        DAT_10802bf8[index].position.z = 0;
    }
    if (motion > -1)
        DAT_10db6ee8[index].PlayMotion(motion, 0, 5, 1.0f);
    if (DAT_1020b430[index].retreatFrames != 0)
    {
        DAT_1020b430[index].retreatFrames--;
        if (DAT_1020b430[index].state == LUNGE_STEP_RETREAT)
        {
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = LUNGE_STEP_STALK;
        }
    }
    return 0;
}
