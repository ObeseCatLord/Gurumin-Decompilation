// An enemy that stalks the player and, when told to react, dashes at him with a swing of its hand (0x100af1b0,
// behaviour number 7 of Enemy_UpdateBehaviour 0x100bcb30). It has the same stalk / circle / retreat steps as the
// hand-lunger (enemy_lunger.cpp) but its two attacks move on fixed frames of their own motion and leave streaks
// between its attached nodes.

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
#include "dash_slash_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1001a450(int slot, const Vec4 *point);                     // Slot_HasNoGround
extern "C" int FUN_1001a590(int slot, const Vec4 *unused, float distance);    // ground probe along the slot's angle
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            const Matrix4 *target, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define DASH_NULL_NODE "@efc_Null"
#define DASH_HAND_NODE_START "@Bip01 R Finger0"   // looked up when the behaviour starts
#define DASH_HAND_NODE "Bip01 R Finger0"          // looked up again when the swing starts
#define DASH_HAND_SLOT 5                          // DAT_1021bdf8[index][5] = the hand node
#define DASH_STEP_START 0
#define DASH_STEP_STALK 1              // faces the player and walks up to him while he is far
#define DASH_STEP_CIRCLE 2             // keeps its distance, strafing; goes back to stalking when he is far
#define DASH_STEP_ATTACK_A 3           // the first attack (chosen three times in four)
#define DASH_STEP_ATTACK_B 4           // the second attack
#define DASH_STEP_RETREAT 5            // jumps back a random way
#define DASH_PI 3.1415f
#define DASH_HALF_TURN 180.0f
#define DASH_FIFTH_TURN 1.2566f        // the angle a circling step is rotated by
#define DASH_FAR 400.0f                // stalking turns into circling when the player is nearer than this
#define DASH_CIRCLE_FAR 450.0f         // circling turns back into stalking beyond this
#define DASH_CIRCLE_NEAR 350.0f        // circling may turn into a retreat nearer than this
#define DASH_WALK_SPEED (-14.0f)
#define DASH_CIRCLE_SPEED (-4.0f)
#define DASH_PROBE_HEIGHT 30.0f        // the move vector's height while a step is tested for ground and walls
#define DASH_NO_GROUND_SCALE 0.1f      // a character with no ground below it moves at a tenth of the speed
#define DASH_RETREAT_SLOW (-5.0f)
#define DASH_RETREAT_FAST (-25.0f)
#define DASH_JUMP_SPEED 40.0f
#define DASH_HOP_SPEED 20.0f
#define DASH_PROBE_DISTANCE 200.0f
#define DASH_PROBE_TRIES 5
#define DASH_RETREAT_FRAMES 0x14
#define DASH_RETREAT_SLOW_END 5
#define DASH_RETREAT_FAST_END 0x10
#define DASH_RETREAT_HOP_FRAME 6
#define DASH_FAILED_RETREAT 10
#define DASH_RETREAT_SPREAD 0xb4
#define DASH_RETREAT_HALF 0x5a
#define DASH_MOTION_WALK 1
#define DASH_MOTION_RETREAT 0x11
#define DASH_HOP_SOUND 0x1c6
#define DASH_ATTACK_ODDS 4             // a reaction picks the first attack unless a roll of this comes up 0
#define DASH_STRAFE_MODES 3
#define DASH_STRAFE_AFTER 0x3c
#define DASH_STRAFE_ROLL_RANGE 1000
#define DASH_STRAFE_ROLL 0x50
#define DASH_STRAFE_LEFT 0
#define DASH_STRAFE_RIGHT 1
#define DASH_ROLL_RANGE 10000
#define DASH_ALARM_ROLL 100
#define DASH_RETREAT_ROLL 400
#define DASH_GREETING_ROLL 250
#define DASH_UNSET_TALK (-1)
#define DASH_UNSET_RANGE (-2)
#define DASH_TRAIL_EFFECT 0x455        // the streaks left while it dashes
#define DASH_TRAIL_SIZE 2.0f
#define DASH_TRAIL_MODE 5
#define DASH_ATTACK_SPEED 0.75f        // motion speed of both attacks
#define DASH_SWING_EFFECT 0x370c       // the swing from the hand node
#define DASH_DASH_SLOW (-15.0f)
#define DASH_DASH_FAST (-25.0f)
#define DASH_DASH_FASTEST (-30.0f)
#define DASH_BOOST_SPEED 12.0f

// The first attack: motion 8. The sound plays at motion frame 0x13e, it turns to the player before frame 0x138,
// dashes from frame 0x13d (slow, a boost on 0x13f, then fast with streaks, a little slower from 0x143), starts the
// swing on frame 0x140 and is over at frame 0x156.
#define DASH_A_MOTION 8
#define DASH_A_SOUND_FRAME 0x13e
#define DASH_A_SOUND 0x1cf
#define DASH_A_TURN_END 0x138
#define DASH_A_DASH_FROM 0x13d
#define DASH_A_BOOST_FRAME 0x13f
#define DASH_A_SWING_FRAME 0x140
#define DASH_A_SLOWER_FROM 0x143
#define DASH_A_DASH_END 0x146
#define DASH_A_OVER_FRAME 0x156
// The second attack: motion 6, sound at 0xf6, turns before 0xef, dashes from 0xf6 (boost on 0xf8, swing on 0xf9,
// streaks until 0x101), over at 0x111. It slows down by its own count (from frame 251 of the attack), not by the motion frame.
#define DASH_B_MOTION 6
#define DASH_B_SOUND_FRAME 0xf6
#define DASH_B_SOUND 0x1d0
#define DASH_B_TURN_END 0xef
#define DASH_B_DASH_FROM 0xf6
#define DASH_B_BOOST_FRAME 0xf8
#define DASH_B_SWING_FRAME 0xf9
#define DASH_B_DASH_END 0x101
#define DASH_B_SLOWER_FROM 0xfb
#define DASH_B_OVER_FRAME 0x111

// 0x100af1b0 (Enemy_StalkAndDashSlash). The sibling of Enemy_StalkAndLunge: stalks (it comes within 400 units),
// circles, retreats and reacts the same way. A reaction request (or a roll while circling) starts state 3 (three
// times in four) or state 4, which play their own motion slowed to three quarters and act on fixed frames of it:
// turn to the player, a sound, a walk, a boost, then a run that leaves streaks (from the "@efc_Null" node, from the
// hand node and from the point between them), and on one frame a swing effect from the hand node. The attack ends
// on the last frame of its motion. Every state may produce a move, which is applied at the end together with the
// motion. Returns 0.
int Enemy_StalkAndDashSlash(int index)
{
    Vec4 player;
    Vec4 self;
    Vec4 move;
    int motion = -1;
    int moving = 0;
    int restart = 0;
    float speed = 1.0f;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float amount = (player - self).Length2D();
    float delta;   // the player's y offset while stalking, later the no-ground slow-down factor
    float scratch; // the x offset while stalking, the constants passed by reference, the retreat angle
    DAT_10255a88[index] = 0;
    if (DAT_10210b30[index] != 0 && DAT_1020b430[index].state != 0)
    {
        DAT_1020b430[index].state = FUN_1013f880() % DASH_ATTACK_ODDS != 0 ? DASH_STEP_ATTACK_A : DASH_STEP_ATTACK_B;
        DAT_1020b430[index].wait = 0;
    }
    DAT_10210b30[index] = 0;
    if (FUN_1001a450(index, &self) != 0)
        DAT_1020b430[index].stalled = 1;
    if (DAT_102550e0[index] > 0)
        DAT_102550e0[index] = 0;
    if (DAT_10468730[index] != 0)
    {
        DAT_10468730[index] = 0;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(DASH_NULL_NODE, 0);
    }
    if (DAT_102550e0[index] == 0)
    {
        if (DAT_1020b430[index].state == DASH_STEP_START)
        {
            DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(DASH_NULL_NODE, 0);
            DAT_1021bdf8[index][DASH_HAND_SLOT] = DAT_10db6ee8[index].root->Find(DASH_HAND_NODE_START, 0);
            DAT_1020b430[index].wait = 0;
            if (DAT_1020b430[index].talk == 0)
                DAT_1020b430[index].talk = DASH_UNSET_TALK;
            if (DAT_1020b430[index].range == 0)
                DAT_1020b430[index].range = DASH_UNSET_RANGE;
            DAT_1020b430[index].state = DASH_STEP_STALK;
        }
        else if (DAT_1020b430[index].state == DASH_STEP_STALK)
        {
            delta = player.y - self.y;
            scratch = player.x - self.x;
            scratch = (float)atan2((double)scratch, (double)delta);
            DAT_10db6ee8[index].targetHeading = scratch + DASH_PI;
            if (amount > DASH_FAR)
            {
                move.y = DASH_WALK_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                moving = 1;
                motion = DASH_MOTION_WALK;
                move.z = DASH_PROBE_HEIGHT;
                if (DAT_10db6ee8[index].m_238c != 0)
                {
                    if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) != 0 &&
                        DAT_10db6ee8[index].IsClearAny(&move) != 0 && DAT_10467920[index] == 0)
                    {
                        DAT_1020b430[index].stalled = 0;
                    }
                    else if (DAT_1020b430[index].stalled != 0)
                    {
                        DAT_1020b430[index].state = DASH_STEP_RETREAT;
                        DAT_1020b430[index].wait = 0;
                    }
                    else
                    {
                        DAT_10db6ee8[index].AddMotionSpeed(DASH_JUMP_SPEED);
                        DAT_1020b430[index].stalled = 1;
                    }
                }
                move.z = 0;
            }
            else
            {
                DAT_1020b430[index].state = DASH_STEP_CIRCLE;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == DASH_STEP_CIRCLE)
        {
            DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + DASH_PI;
            if (DAT_1020b430[index].strafeMode == DASH_STRAFE_LEFT)
            {
                move.y = DASH_CIRCLE_SPEED;
                move.RotateZ(DASH_FIFTH_TURN - DAT_10db6ee8[index].heading);
                moving = 1;
                motion = 0;
            }
            else if (DAT_1020b430[index].strafeMode == DASH_STRAFE_RIGHT)
            {
                move.y = DASH_CIRCLE_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading - DASH_FIFTH_TURN);
                moving = 1;
                motion = 0;
            }
            else
                motion = 2;
            if (++DAT_1020b430[index].wait > DASH_STRAFE_AFTER &&
                FUN_1013f880() % DASH_STRAFE_ROLL_RANGE < DASH_STRAFE_ROLL)
            {
                DAT_1020b430[index].strafeMode = FUN_1013f880() % DASH_STRAFE_MODES;
                DAT_1020b430[index].wait = 0;
            }
            if (DAT_10467920[index] != 0 || FUN_1013f880() % DASH_ROLL_RANGE < DASH_ALARM_ROLL)
                DAT_10210b30[index] = 1;
            if (amount > DASH_CIRCLE_FAR)
            {
                DAT_1020b430[index].state = DASH_STEP_STALK;
                DAT_1020b430[index].wait = 0;
            }
            if (amount < DASH_CIRCLE_NEAR && FUN_1013f880() % DASH_ROLL_RANGE < DASH_RETREAT_ROLL)
            {
                DAT_1020b430[index].state = DASH_STEP_RETREAT;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == DASH_STEP_ATTACK_A)
        {
            motion = DASH_A_MOTION;
            speed = DASH_ATTACK_SPEED;
            if (++DAT_1020b430[index].wait == 1)
            {
                restart = 1;
                DAT_1020b430[index].swingStarted = 0;
            }
            if (DAT_10db6ee8[index].motionFrame == DASH_A_SOUND_FRAME)
                FUN_10065b10(index, DASH_A_SOUND, 0, 0);
            if (DAT_10db6ee8[index].motionFrame < DASH_A_TURN_END)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + DASH_PI;
            int frame = DAT_10db6ee8[index].motionFrame;
            if (frame >= DASH_A_DASH_FROM)
            {
                if (frame < DASH_A_BOOST_FRAME)
                {
                    move.y = DASH_DASH_SLOW;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                }
                else if (frame == DASH_A_BOOST_FRAME)
                {
                    move.y = DASH_DASH_FAST;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    DAT_10db6ee8[index].AddMotionSpeed(DASH_BOOST_SPEED);
                }
                else if (frame < DASH_A_DASH_END)
                {
                    move.y = DASH_DASH_FASTEST;
                    if (frame >= DASH_A_SLOWER_FROM)
                        move.y = DASH_DASH_FAST;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    EffectOptions options;
                    options.m_0000 = 0;
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0008 = DASH_TRAIL_SIZE;
                    options.m_000e = DASH_TRAIL_MODE;
                    Vec4 offset;
                    if (DAT_1021bdf8[index][0] != 0)
                    {
                        Matrix4 trail;
                        trail.SetIdentity();
                        options.owner = index + EFFECT_OWNER_BASE;
                        FUN_1001d8b0(index, DASH_TRAIL_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0,
                                     0, 0, &options);
                        if (DAT_1021bdf8[index][DASH_HAND_SLOT] != 0)
                        {
                            scratch = 0.5f;
                            offset.Copy((DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() +
                                         DAT_1021bdf8[index][DASH_HAND_SLOT]->worldMatrix.GetTranslation()) *
                                        scratch);
                            trail.SetTranslation(offset);
                            scratch = 0.0f;
                            offset.Fill(scratch);
                            FUN_1001d8b0(index, DASH_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &trail, &options);
                            FUN_1001d8b0(index, DASH_TRAIL_EFFECT, offset, -1, -1,
                                         &DAT_1021bdf8[index][DASH_HAND_SLOT]->worldMatrix, 0, 0, 0, &options);
                        }
                    }
                }
                else if (frame == DASH_A_OVER_FRAME)
                {
                    DAT_1020b430[index].state = DASH_STEP_STALK;
                    DAT_1020b430[index].wait = 0;
                }
            }
            if (DAT_10db6ee8[index].motionFrame == DASH_A_SWING_FRAME && DAT_1020b430[index].swingStarted == 0)
            {
                DAT_1020b430[index].swingStarted = 1;
                if (DAT_1021bdf8[index][0] != 0 && DAT_10db6ee8[index].root->Find(DASH_HAND_NODE, 0) != 0)
                {
                    Matrix4 swing;
                    EffectOptions options;
                    options.followOwner = 1;
                    Vec4 offset;
                    swing = RotationZ(DAT_10db6ee8[index].heading);
                    options.owner = index + EFFECT_OWNER_BASE;
                    FUN_1001d8b0(index, DASH_SWING_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                                 &DAT_10db6ee8[index].root->Find(DASH_HAND_NODE, 0)->worldMatrix, 0, &swing, &options);
                }
            }
        }
        else if (DAT_1020b430[index].state == DASH_STEP_ATTACK_B)
        {
            motion = DASH_B_MOTION;
            speed = DASH_ATTACK_SPEED;
            if (++DAT_1020b430[index].wait == 1)
            {
                restart = 1;
                DAT_1020b430[index].swingStarted = 0;
            }
            if (DAT_10db6ee8[index].motionFrame == DASH_B_SOUND_FRAME)
                FUN_10065b10(index, DASH_B_SOUND, 0, 0);
            if (DAT_10db6ee8[index].motionFrame < DASH_B_TURN_END)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + DASH_PI;
            int frame = DAT_10db6ee8[index].motionFrame;
            if (frame >= DASH_B_SOUND_FRAME)
            {
                if (frame < DASH_B_BOOST_FRAME)
                {
                    move.y = DASH_DASH_SLOW;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                }
                else if (frame == DASH_B_BOOST_FRAME)
                {
                    move.y = DASH_DASH_FAST;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    DAT_10db6ee8[index].AddMotionSpeed(DASH_BOOST_SPEED);
                }
                else if (frame < DASH_B_DASH_END)
                {
                    move.y = DASH_DASH_FASTEST;
                    if (DAT_1020b430[index].wait >= DASH_B_SLOWER_FROM)
                        move.y = DASH_DASH_FAST;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    EffectOptions options;
                    options.m_0000 = 0;
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0008 = DASH_TRAIL_SIZE;
                    options.m_000e = DASH_TRAIL_MODE;
                    Vec4 offset;
                    if (DAT_1021bdf8[index][0] != 0)
                    {
                        Matrix4 trail;
                        trail.SetIdentity();
                        options.owner = index + EFFECT_OWNER_BASE;
                        FUN_1001d8b0(index, DASH_TRAIL_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0,
                                     0, 0, &options);
                        if (DAT_1021bdf8[index][DASH_HAND_SLOT] != 0)
                        {
                            scratch = 0.5f;
                            offset.Copy((DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() +
                                         DAT_1021bdf8[index][DASH_HAND_SLOT]->worldMatrix.GetTranslation()) *
                                        scratch);
                            trail.SetTranslation(offset);
                            scratch = 0.0f;
                            offset.Fill(scratch);
                            FUN_1001d8b0(index, DASH_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &trail, &options);
                            FUN_1001d8b0(index, DASH_TRAIL_EFFECT, offset, -1, -1,
                                         &DAT_1021bdf8[index][DASH_HAND_SLOT]->worldMatrix, 0, 0, 0, &options);
                        }
                    }
                }
                else if (frame == DASH_B_OVER_FRAME)
                {
                    DAT_1020b430[index].state = DASH_STEP_STALK;
                    DAT_1020b430[index].wait = 0;
                }
            }
            if (DAT_10db6ee8[index].motionFrame == DASH_B_SWING_FRAME && DAT_1020b430[index].swingStarted == 0)
            {
                DAT_1020b430[index].swingStarted = 1;
                if (DAT_1021bdf8[index][0] != 0 && DAT_10db6ee8[index].root->Find(DASH_HAND_NODE, 0) != 0)
                {
                    Matrix4 swing;
                    EffectOptions options;
                    options.followOwner = 1;
                    Vec4 offset;
                    swing = RotationZ(DAT_10db6ee8[index].heading);
                    options.owner = index + EFFECT_OWNER_BASE;
                    FUN_1001d8b0(index, DASH_SWING_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                                 &DAT_10db6ee8[index].root->Find(DASH_HAND_NODE, 0)->worldMatrix, 0, &swing, &options);
                }
            }
        }
        else if (DAT_1020b430[index].state == DASH_STEP_RETREAT)
        {
            if (DAT_1020b430[index].wait == 0)
            {
                int tries = 0;
                do
                {
                    int roll = FUN_1013f880() % DASH_RETREAT_SPREAD - DASH_RETREAT_HALF;
                    scratch = (float)roll;
                    DAT_10255160[index][0] = scratch * DASH_PI / DASH_HALF_TURN + DASH_PI;
                    if (FUN_1001a590(index, &self, DASH_PROBE_DISTANCE) != 0)
                        break;
                    tries++;
                } while (tries < DASH_PROBE_TRIES);
                if (tries == DASH_PROBE_TRIES)
                {
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].retreatFrames = DASH_FAILED_RETREAT;
                    DAT_1020b430[index].state = DASH_STEP_STALK;
                    return 0;
                }
            }
            if (DAT_1020b430[index].wait == DASH_RETREAT_HOP_FRAME)
            {
                FUN_10065b10(index, DASH_HOP_SOUND, 0, 0);
                DAT_10db6ee8[index].AddMotionSpeed(DASH_HOP_SPEED);
            }
            if (DAT_1020b430[index].wait < DASH_RETREAT_SLOW_END || DAT_1020b430[index].wait > 0xf)
                move.y = DASH_RETREAT_SLOW;
            else if (DAT_1020b430[index].wait < DASH_RETREAT_FAST_END)
                move.y = DASH_RETREAT_FAST;
            move.RotateZ(DAT_10255160[index][0] - DAT_10db6ee8[index].heading);
            moving = 1;
            motion = DASH_MOTION_RETREAT;
            if (++DAT_1020b430[index].wait > DASH_RETREAT_FRAMES)
            {
                DAT_1020b430[index].state = DASH_STEP_STALK;
                DAT_1020b430[index].wait = 0;
            }
        }
    }
    if (DAT_1020b430[index].state < 2 && DAT_104c00e0 != 0 && FUN_1013f880() % DASH_ROLL_RANGE < DASH_GREETING_ROLL)
    {
        DAT_1020b430[index].state = DASH_STEP_RETREAT;
        DAT_1020b430[index].wait = 0;
    }
    if (moving != 0)
    {
        if (DAT_10467920[index] != 0)
        {
            delta = DASH_NO_GROUND_SCALE;
            move.Copy(move * delta);
        }
        FUN_100241e0(index, move.x, move.y, move.z);
        DAT_10802bf8[index].position.Copy(move);
        DAT_10802bf8[index].position.z = 0;
    }
    if (motion > -1)
        DAT_10db6ee8[index].PlayMotion(motion, restart, 5, speed);
    if (DAT_1020b430[index].retreatFrames != 0)
    {
        DAT_1020b430[index].retreatFrames--;
        if (DAT_1020b430[index].state == DASH_STEP_RETREAT)
        {
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = DASH_STEP_STALK;
        }
    }
    return 0;
}
