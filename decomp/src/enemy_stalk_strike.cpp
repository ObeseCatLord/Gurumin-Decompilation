// An enemy that stalks the player and answers with one of three dash attacks or a slam (0x100a9240, a per-kind
// behaviour of 0x100bcb30). It follows the stalk / circle / retreat steps of the lunging enemy (enemy_lunger.cpp), but
// when told to react it picks from more attacks, and when greetings are on it may wave and back off instead.

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
#include "strike_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1001a450(int slot, const Vec4 *point);                     // Slot_HasNoGround
extern "C" int FUN_1001a590(int slot, const Vec4 *unused, float distance);    // ground probe along the slot's angle
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            const Matrix4 *target, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define STRIKE_NULL_NODE "@efc_Null"
#define STRIKE_LEFT_HAND_NODE "@Bip01 L Finger0"   // looked up when the behaviour starts
#define STRIKE_HAND_NODE "Bip01 R Finger0"
#define STRIKE_STEP_START 0
#define STRIKE_STEP_STALK 1            // faces the player and walks up to him while he is far
#define STRIKE_STEP_CIRCLE 2           // keeps its distance, circling; goes back to stalking when he is far
#define STRIKE_STEP_DASH_A 3           // the three dash attacks
#define STRIKE_STEP_DASH_B 4
#define STRIKE_STEP_DASH_C 5
#define STRIKE_STEP_SLAM 6             // stamps, sliding back, with a burst of effects
#define STRIKE_STEP_RETREAT 0xf        // jumps back a random way
#define STRIKE_STEP_WAVE 0x10          // waves at the player (greetings on)
#define STRIKE_PI 3.1415f
#define STRIKE_HALF_TURN 180.0f
#define STRIKE_TURN_FULL 6.283f
#define STRIKE_WAVE_TURN_LIMIT 0.616853f   // squared angle beyond which it gives up waving
#define STRIKE_CIRCLE_ANGLE 0.17453f   // a circling step is the offset to the player turned by this much
#define STRIKE_CIRCLE_STEP 8.0f        // length of a circling step
#define STRIKE_STALK_FAR 280.0f        // stalking turns into circling when the player is nearer than this
#define STRIKE_CIRCLE_FAR 300.0f       // circling turns back into stalking beyond this
#define STRIKE_CIRCLE_NEAR 250.0f      // circling may turn into a retreat nearer than this
#define STRIKE_REACT_FAR 400.0f        // a reaction only attacks when the player is nearer than this
#define STRIKE_GREET_RANGE 500.0f      // a greeting (wave or retreat) only starts when the player is nearer than this
#define STRIKE_WALK_SPEED (-16.0f)
#define STRIKE_PROBE_HEIGHT 30.0f      // the move vector's height while a step is tested for ground and walls
#define STRIKE_NO_GROUND_SCALE 0.1f    // a character with no ground below it moves at a tenth of the speed
#define STRIKE_JUMP_SPEED 30.0f
#define STRIKE_HOP_SPEED 20.0f
#define STRIKE_BOOST_SPEED 15.0f
#define STRIKE_ATTACK_FIRST 3
#define STRIKE_ATTACK_COUNT 3          // a reaction picks one of the three dash attacks
#define STRIKE_VARIANT_ATTACK_COUNT 4  // ... or, for the variant, also the slam
#define STRIKE_ROLL_RANGE 10000
#define STRIKE_REACT_ROLL 9000         // a reaction picks an attack with this chance (out of STRIKE_ROLL_RANGE)
#define STRIKE_ALARM_ROLL 200
#define STRIKE_RETREAT_ROLL 400
#define STRIKE_STRAFE_MODES 3
#define STRIKE_STRAFE_AFTER 0x3c
#define STRIKE_STRAFE_ROLL_RANGE 1000
#define STRIKE_STRAFE_ROLL 0x50
#define STRIKE_STRAFE_LEFT 0
#define STRIKE_STRAFE_RIGHT 1
#define STRIKE_ATTACK_AGAIN_ODDS 0x14  // after an attack it starts another one unless a roll of this comes up 0
#define STRIKE_AREA_FIRST 0x2b         // map areas in which greetings are off
#define STRIKE_AREA_LAST 0x2e
#define STRIKE_AREA_SHY_A 0x30         // map areas in which it retreats instead of waving
#define STRIKE_AREA_SHY_B 0x13
#define STRIKE_UNSET_TALK (-1)
#define STRIKE_UNSET_RANGE (-1)
#define STRIKE_LEVEL_LIMIT 10
#define STRIKE_FRAME_UNSTARTED (-1)    // wait value after an attack: the next step starts over
#define STRIKE_MOTION_WALK 1
#define STRIKE_MOTION_CIRCLE 0
#define STRIKE_MOTION_HOLD 2
#define STRIKE_MOTION_SLAM 5
#define STRIKE_MOTION_RETREAT 0x11
#define STRIKE_MOTION_WAVE 0x15
#define STRIKE_PROBE_DISTANCE 200.0f
#define STRIKE_PROBE_TRIES 5
#define STRIKE_RETREAT_SPREAD 0xb4
#define STRIKE_RETREAT_HALF 0x5a
#define STRIKE_RETREAT_HOP_FRAME 6
#define STRIKE_RETREAT_SLOW (-5.0f)
#define STRIKE_RETREAT_FAST (-35.0f)
#define STRIKE_RETREAT_SLOW_END 5
#define STRIKE_RETREAT_FAST_END 0x10
#define STRIKE_RETREAT_FRAMES 0x14
#define STRIKE_FAILED_RETREAT 10
#define STRIKE_HOP_SOUND 0x1c6
#define STRIKE_WAVE_FRAMES 0x1e
#define STRIKE_TRAIL_EFFECT 0x455      // the streak left while it dashes and slams
#define STRIKE_TRAIL_SIZE 1.2f
#define STRIKE_TRAIL_MODE 5
#define STRIKE_SWING_EFFECT 0x370c
#define STRIKE_TURN_FRAMES 5           // counts after the wind-up frame in which it still turns to the player
#define STRIKE_DASH_END 0x28           // count at which a dash attack is over
#define STRIKE_DASH_SLOW (-7.0f)
#define STRIKE_DASH_KICK (-12.0f)
#define STRIKE_DASH_FAST (-15.0f)
#define STRIKE_DASH_STEADY (-12.0f)

// Dash attack A: wind-up frame 0x131 of motion 8; the dash runs from count 11 to 27 and the swing effect starts at
// motion frame 0x13b.
#define STRIKE_A_MOTION 8
#define STRIKE_A_WINDUP_FRAME 0x131
#define STRIKE_A_WINDUP_SOUND 0x1d5
#define STRIKE_A_DASH_FROM 11          // the dash starts at this count
#define STRIKE_A_KICK_COUNT 0xd
#define STRIKE_A_RUN_END 0x1c
#define STRIKE_A_STEADY_FROM 0xf
#define STRIKE_A_SWING_FRAME 0x13b
// Dash attack B: wind-up frame 0xeb of motion 6, dash from count 10 to 22, swing at motion frame 0xf4.
#define STRIKE_B_MOTION 6
#define STRIKE_B_WINDUP_FRAME 0xeb
#define STRIKE_B_WINDUP_SOUND 0x1d6
#define STRIKE_B_DASH_FROM 10
#define STRIKE_B_KICK_COUNT 0xc
#define STRIKE_B_RUN_END 0x17
#define STRIKE_B_STEADY_FROM 0xe
#define STRIKE_B_SWING_FRAME 0xf4
// Dash attack C: wind-up frame 0x117 of motion 7, short dash, swing at motion frame 0x118, over at count 20.
#define STRIKE_C_MOTION 7
#define STRIKE_C_WINDUP_FRAME 0x117
#define STRIKE_C_WINDUP_SOUND 0x1d4
#define STRIKE_C_KICK_COUNT 2
#define STRIKE_C_RUN_END 0xd
#define STRIKE_C_STEADY_FROM 4
#define STRIKE_C_DASH_END 0x14
#define STRIKE_C_SWING_FRAME 0x118
// The slam: motion 5; it stamps at frame 0x9b (sound, then a slide back that decays), the effect burst runs while
// the motion is between frames 0x9b and 0xba, and it ends at frame 0xbd.
#define STRIKE_SLAM_STAMP_FRAME 0x9b
#define STRIKE_SLAM_END_BURST 0xba
#define STRIKE_SLAM_END_FRAME 0xbd
#define STRIKE_SLAM_SOUND 0x120
#define STRIKE_SLAM_SLIDE 20.0f
#define STRIKE_SLAM_DECAY 0.95f
#define STRIKE_SLAM_EFFECT_B 0x370d
#define STRIKE_SLAM_EFFECT_C 0x370e
#define STRIKE_SLAM_C_PERIOD 7

// 0x100a9240 (Enemy_StalkAndStrike). Like Enemy_StalkAndLunge but with other distances (it stalks to 280 units,
// circles by turning the offset to the player about 10 degrees a frame, retreats inside 250 and attacks inside 400).
// A reaction request picks state 3, 4 or 5 (a dash attack: wind-up motion, sound, turn to the player for five
// counts, dash with a streak, a swing effect from "Bip01 R Finger0") or, for the variant, also state 6 (the slam).
// With greetings on it may instead retreat (state 0xf) or, for the variant, wave (state 0x10). Every state may
// produce a move, which is applied at the end together with the motion. Returns 0.
int Enemy_StalkAndStrike(int index, int variant)
{
    Vec4 player;
    Vec4 self;
    Vec4 move;
    int motion = -1;
    int restart = 0;
    int moving = 0;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float amount = (player - self).Length2D();
    DAT_10255a88[index] = 0;
    if (DAT_10210b30[index] != 0 && DAT_1020b430[index].state != 0)
    {
        if (DAT_104c00e0 == 0 || ((unsigned int)DAT_104c00e8 >= STRIKE_AREA_FIRST && (unsigned int)DAT_104c00e8 <= STRIKE_AREA_LAST))
        {
            if (amount < STRIKE_REACT_FAR)
            {
                if (variant != 0)
                    DAT_1020b430[index].state = FUN_1013f880() % STRIKE_VARIANT_ATTACK_COUNT + STRIKE_ATTACK_FIRST;
                else
                    DAT_1020b430[index].state = FUN_1013f880() % STRIKE_ATTACK_COUNT + STRIKE_ATTACK_FIRST;
            }
            else
                DAT_1020b430[index].state = STRIKE_STEP_STALK;
            DAT_1020b430[index].wait = 0;
        }
        else if (FUN_1013f880() % STRIKE_ROLL_RANGE < STRIKE_REACT_ROLL)
        {
            if (variant != 0 && FUN_1013f880() % 2 != 0 && DAT_104c00e8 != STRIKE_AREA_SHY_A &&
                DAT_104c00e8 != STRIKE_AREA_SHY_B)
                DAT_1020b430[index].state = STRIKE_STEP_WAVE;
            else
                DAT_1020b430[index].state = STRIKE_STEP_RETREAT;
            DAT_1020b430[index].wait = 0;
        }
    }
    DAT_10210b30[index] = 0;
    if (FUN_1001a450(index, &self) != 0)
        DAT_1020b430[index].stalled = 1;
    if (DAT_102550e0[index] > STRIKE_LEVEL_LIMIT)
        DAT_102550e0[index] = STRIKE_LEVEL_LIMIT;
    if (DAT_10468730[index] != 0)
    {
        DAT_10468730[index] = 0;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(STRIKE_NULL_NODE, 0);
    }
    if (DAT_102550e0[index] == 0)
    {
        if (DAT_1020b430[index].state == STRIKE_STEP_START)
        {
            DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(STRIKE_NULL_NODE, 0);
            DAT_1020b430[index].wait = 0;
            if (DAT_1020b430[index].talk == 0)
                DAT_1020b430[index].talk = STRIKE_UNSET_TALK;
            if (DAT_1020b430[index].range == 0)
                DAT_1020b430[index].range = STRIKE_UNSET_RANGE;
            DAT_1020b430[index].state = STRIKE_STEP_STALK;
            DAT_1021bdf8[index][4] = DAT_10db6ee8[index].root->Find(STRIKE_LEFT_HAND_NODE, 0);
        }
        else if (DAT_1020b430[index].state == STRIKE_STEP_STALK)
        {
            motion = STRIKE_MOTION_WALK;
            if (++DAT_1020b430[index].wait == 1 && DAT_10db6ee8[index].motion->layer != STRIKE_MOTION_WALK)
                restart = 1;
            float dy = player.y - self.y;
            float dx = player.x - self.x;
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)dx, (double)dy) + STRIKE_PI;
            if (amount > STRIKE_STALK_FAR)
            {
                move.y = STRIKE_WALK_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                moving = 1;
                motion = STRIKE_MOTION_WALK;
                move.z = STRIKE_PROBE_HEIGHT;
                if (DAT_10db6ee8[index].m_238c != 0)
                {
                    if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) != 0 &&
                        DAT_10db6ee8[index].IsClearAny(&move) != 0 && DAT_10467920[index] == 0)
                    {
                        DAT_1020b430[index].stalled = 0;
                    }
                    else if (DAT_1020b430[index].stalled != 0)
                    {
                        DAT_1020b430[index].state = STRIKE_STEP_RETREAT;
                        DAT_1020b430[index].wait = 0;
                    }
                    else
                    {
                        DAT_10db6ee8[index].AddMotionSpeed(STRIKE_JUMP_SPEED);
                        DAT_1020b430[index].stalled = 1;
                    }
                }
                move.z = 0;
            }
            else
            {
                DAT_1020b430[index].state = STRIKE_STEP_CIRCLE;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == STRIKE_STEP_CIRCLE)
        {
            DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + STRIKE_PI;
            if (DAT_1020b430[index].strafeMode == STRIKE_STRAFE_LEFT)
            {
                move.Copy(self - player);
                move.RotateZ(STRIKE_CIRCLE_ANGLE);
                move.Copy(move + player - self);
                move.z = 0;
                if (move.Length2D() > STRIKE_CIRCLE_STEP)
                {
                    move.Normalized3();
                    move.Copy(move * STRIKE_CIRCLE_STEP);
                }
                moving = 1;
                motion = STRIKE_MOTION_CIRCLE;
            }
            else if (DAT_1020b430[index].strafeMode == STRIKE_STRAFE_RIGHT)
            {
                move.Copy(self - player);
                move.RotateZ(-STRIKE_CIRCLE_ANGLE);
                move.Copy(move + player - self);
                move.z = 0;
                if (move.Length2D() > STRIKE_CIRCLE_STEP)
                {
                    move.Normalized3();
                    move.Copy(move * STRIKE_CIRCLE_STEP);
                }
                moving = 1;
                motion = STRIKE_MOTION_CIRCLE;
            }
            else
                motion = STRIKE_MOTION_HOLD;
            if (++DAT_1020b430[index].wait > STRIKE_STRAFE_AFTER &&
                FUN_1013f880() % STRIKE_STRAFE_ROLL_RANGE < STRIKE_STRAFE_ROLL)
            {
                DAT_1020b430[index].strafeMode = FUN_1013f880() % STRIKE_STRAFE_MODES;
                DAT_1020b430[index].wait = 0;
            }
            if (DAT_10467920[index] != 0 || FUN_1013f880() % STRIKE_ROLL_RANGE < STRIKE_ALARM_ROLL)
                DAT_10210b30[index] = 1;
            if (amount > STRIKE_CIRCLE_FAR)
            {
                DAT_1020b430[index].state = STRIKE_STEP_STALK;
                DAT_1020b430[index].wait = 0;
            }
            if (amount < STRIKE_CIRCLE_NEAR && FUN_1013f880() % STRIKE_ROLL_RANGE < STRIKE_RETREAT_ROLL)
            {
                DAT_1020b430[index].state = STRIKE_STEP_RETREAT;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == STRIKE_STEP_DASH_A)
        {
            motion = STRIKE_A_MOTION;
            if (DAT_10db6ee8[index].motion->layer != STRIKE_A_MOTION && DAT_10db6ee8[index].motion->blending == 0)
                restart = 1;
            if (DAT_10db6ee8[index].motionFrame == STRIKE_A_WINDUP_FRAME)
            {
                DAT_1020b430[index].wait = 1;
                FUN_10065b10(index, STRIKE_A_WINDUP_SOUND, 0, 0);
            }
            if (DAT_1020b430[index].wait != 0)
                DAT_1020b430[index].wait++;
            if (DAT_1020b430[index].wait < STRIKE_TURN_FRAMES)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + STRIKE_PI;
            if (DAT_1020b430[index].wait >= STRIKE_A_DASH_FROM)
            {
                if (DAT_1020b430[index].wait < STRIKE_A_KICK_COUNT)
                {
                    move.y = STRIKE_DASH_SLOW;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                }
                else if (DAT_1020b430[index].wait == STRIKE_A_KICK_COUNT)
                {
                    move.y = STRIKE_DASH_KICK;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    DAT_10db6ee8[index].AddMotionSpeed(STRIKE_BOOST_SPEED);
                }
                else if (DAT_1020b430[index].wait < STRIKE_A_RUN_END)
                {
                    move.y = STRIKE_DASH_FAST;
                    if (DAT_1020b430[index].wait >= STRIKE_A_STEADY_FROM)
                        move.y = STRIKE_DASH_STEADY;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    EffectOptions options;
                    options.m_0000 = 0;
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0008 = STRIKE_TRAIL_SIZE;
                    options.m_000e = STRIKE_TRAIL_MODE;
                    Vec4 offset;
                    if (DAT_1021bdf8[index][0] != 0)
                    {
                        options.owner = index + EFFECT_OWNER_BASE;
                        FUN_1001d8b0(index, STRIKE_TRAIL_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0,
                                     0, 0, &options);
                    }
                }
                else if (DAT_1020b430[index].wait == STRIKE_DASH_END)
                {
                    DAT_1020b430[index].state = STRIKE_STEP_STALK;
                    DAT_1020b430[index].wait = 0;
                }
            }
            if (DAT_10db6ee8[index].motionFrame == STRIKE_A_SWING_FRAME && DAT_1021bdf8[index][0] != 0 &&
                DAT_10db6ee8[index].root->Find(STRIKE_HAND_NODE, 0) != 0)
            {
                Matrix4 swing;
                EffectOptions options;
                options.followOwner = 1;
                Vec4 offset;
                swing = RotationZ(DAT_10db6ee8[index].heading);
                options.owner = index + EFFECT_OWNER_BASE;
                FUN_1001d8b0(index, STRIKE_SWING_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                             &DAT_10db6ee8[index].root->Find(STRIKE_HAND_NODE, 0)->worldMatrix, 0, &swing, &options);
            }
        }
        else if (DAT_1020b430[index].state == STRIKE_STEP_DASH_B)
        {
            motion = STRIKE_B_MOTION;
            if (DAT_10db6ee8[index].motion->layer != STRIKE_B_MOTION && DAT_10db6ee8[index].motion->blending == 0)
                restart = 1;
            if (DAT_10db6ee8[index].motionFrame == STRIKE_B_WINDUP_FRAME)
            {
                DAT_1020b430[index].wait = 1;
                FUN_10065b10(index, STRIKE_B_WINDUP_SOUND, 0, 0);
            }
            if (DAT_1020b430[index].wait != 0)
                DAT_1020b430[index].wait++;
            if (DAT_1020b430[index].wait < STRIKE_TURN_FRAMES)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + STRIKE_PI;
            if (DAT_1020b430[index].wait >= STRIKE_B_DASH_FROM)
            {
                if (DAT_1020b430[index].wait < STRIKE_B_KICK_COUNT)
                {
                    move.y = STRIKE_DASH_SLOW;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                }
                else if (DAT_1020b430[index].wait == STRIKE_B_KICK_COUNT)
                {
                    move.y = STRIKE_DASH_KICK;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    DAT_10db6ee8[index].AddMotionSpeed(STRIKE_BOOST_SPEED);
                }
                else if (DAT_1020b430[index].wait < STRIKE_B_RUN_END)
                {
                    move.y = STRIKE_DASH_FAST;
                    if (DAT_1020b430[index].wait >= STRIKE_B_STEADY_FROM)
                        move.y = STRIKE_DASH_STEADY;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    moving = 1;
                    EffectOptions options;
                    options.m_0000 = 0;
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0008 = STRIKE_TRAIL_SIZE;
                    options.m_000e = STRIKE_TRAIL_MODE;
                    Vec4 offset;
                    if (DAT_1021bdf8[index][0] != 0)
                    {
                        options.owner = index + EFFECT_OWNER_BASE;
                        FUN_1001d8b0(index, STRIKE_TRAIL_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0,
                                     0, 0, &options);
                    }
                }
                else if (DAT_1020b430[index].wait == STRIKE_DASH_END)
                {
                    DAT_1020b430[index].state = STRIKE_STEP_STALK;
                    DAT_1020b430[index].wait = STRIKE_FRAME_UNSTARTED;
                }
            }
            if (DAT_10db6ee8[index].motionFrame == STRIKE_B_SWING_FRAME && DAT_1021bdf8[index][0] != 0 &&
                DAT_10db6ee8[index].root->Find(STRIKE_HAND_NODE, 0) != 0)
            {
                Matrix4 swing;
                EffectOptions options;
                options.followOwner = 1;
                Vec4 offset;
                swing = RotationZ(DAT_10db6ee8[index].heading);
                options.owner = index + EFFECT_OWNER_BASE;
                FUN_1001d8b0(index, STRIKE_SWING_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                             &DAT_10db6ee8[index].root->Find(STRIKE_HAND_NODE, 0)->worldMatrix, 0, &swing, &options);
            }
        }
        else if (DAT_1020b430[index].state == STRIKE_STEP_DASH_C)
        {
            motion = STRIKE_C_MOTION;
            if (DAT_10db6ee8[index].motion->layer != STRIKE_C_MOTION && DAT_10db6ee8[index].motion->blending == 0)
                restart = 1;
            if (DAT_10db6ee8[index].motionFrame == STRIKE_C_WINDUP_FRAME)
            {
                DAT_1020b430[index].wait = 1;
                FUN_10065b10(index, STRIKE_C_WINDUP_SOUND, 0, 0);
            }
            if (DAT_1020b430[index].wait != 0)
                DAT_1020b430[index].wait++;
            if (DAT_1020b430[index].wait < STRIKE_TURN_FRAMES)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + STRIKE_PI;
            if (DAT_1020b430[index].wait < STRIKE_C_KICK_COUNT)
            {
                move.y = STRIKE_DASH_SLOW;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                moving = 1;
            }
            else if (DAT_1020b430[index].wait == STRIKE_C_KICK_COUNT)
            {
                move.y = STRIKE_DASH_KICK;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                moving = 1;
                DAT_10db6ee8[index].AddMotionSpeed(STRIKE_BOOST_SPEED);
            }
            else if (DAT_1020b430[index].wait < STRIKE_C_RUN_END)
            {
                move.y = STRIKE_DASH_FAST;
                if (DAT_1020b430[index].wait >= STRIKE_C_STEADY_FROM)
                    move.y = STRIKE_DASH_STEADY;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                moving = 1;
                EffectOptions options;
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = STRIKE_TRAIL_SIZE;
                options.m_000e = STRIKE_TRAIL_MODE;
                Vec4 offset;
                if (DAT_1021bdf8[index][0] != 0)
                {
                    options.owner = index + EFFECT_OWNER_BASE;
                    FUN_1001d8b0(index, STRIKE_TRAIL_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0,
                                 0, &options);
                }
            }
            else if (DAT_1020b430[index].wait == STRIKE_C_DASH_END)
            {
                DAT_1020b430[index].state = STRIKE_STEP_STALK;
                DAT_1020b430[index].wait = STRIKE_FRAME_UNSTARTED;
            }
            if (DAT_10db6ee8[index].motionFrame == STRIKE_C_SWING_FRAME && DAT_1021bdf8[index][0] != 0 &&
                DAT_10db6ee8[index].root->Find(STRIKE_HAND_NODE, 0) != 0)
            {
                Matrix4 swing;
                EffectOptions options;
                options.followOwner = 1;
                Vec4 offset;
                swing = RotationZ(DAT_10db6ee8[index].heading);
                options.owner = index + EFFECT_OWNER_BASE;
                FUN_1001d8b0(index, STRIKE_SWING_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                             &DAT_10db6ee8[index].root->Find(STRIKE_HAND_NODE, 0)->worldMatrix, 0, &swing, &options);
            }
        }
        else if (DAT_1020b430[index].state == STRIKE_STEP_SLAM)
        {
            motion = STRIKE_MOTION_SLAM;
            if (++DAT_1020b430[index].wait == 1)
            {
                restart = 1;
                DAT_10255160[index][1] = 0;
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + STRIKE_PI;
            }
            if (DAT_10db6ee8[index].motionFrame == STRIKE_SLAM_STAMP_FRAME)
            {
                DAT_10255160[index][1] = STRIKE_SLAM_SLIDE;
                FUN_10065b10(index, STRIKE_SLAM_SOUND, 0, 0);
            }
            if (DAT_10db6ee8[index].motionFrame > STRIKE_SLAM_STAMP_FRAME &&
                DAT_10db6ee8[index].motionFrame < STRIKE_SLAM_END_BURST)
            {
                EffectOptions options;
                Matrix4 swing;
                Vec4 offset;
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = STRIKE_TRAIL_SIZE;
                options.m_000e = STRIKE_TRAIL_MODE;
                if (DAT_1021bdf8[index][4] != 0)
                {
                    swing = RotationZ(DAT_10db6ee8[index].heading);
                    swing.SetTranslation(DAT_1021bdf8[index][4]->worldMatrix.GetTranslation());
                    FUN_1001d8b0(index, STRIKE_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &swing, &options);
                    if (DAT_104c0380 % 2 != 0)
                        FUN_1001d8b0(index, STRIKE_SLAM_EFFECT_B, offset, -1, -1, 0, 0, 0, &swing, &options);
                    if (DAT_104c0380 % STRIKE_SLAM_C_PERIOD == 0)
                        FUN_1001d8b0(index, STRIKE_SLAM_EFFECT_C, offset, -1, -1, 0, 0, 0, &swing, &options);
                }
            }
            if (DAT_10db6ee8[index].motionFrame == STRIKE_SLAM_END_FRAME)
            {
                DAT_1020b430[index].state = STRIKE_STEP_STALK;
                DAT_1020b430[index].wait = STRIKE_FRAME_UNSTARTED;
            }
            if (DAT_10255160[index][1] > 0.0f)
            {
                move.y = -DAT_10255160[index][1];
                move.RotateZ(-DAT_10db6ee8[index].heading);
                moving = 1;
                DAT_10255160[index][1] = DAT_10255160[index][1] * STRIKE_SLAM_DECAY;
            }
        }
        else if (DAT_1020b430[index].state == STRIKE_STEP_RETREAT)
        {
            if (DAT_1020b430[index].wait == 0)
            {
                int tries = 0;
                do
                {
                    DAT_10255160[index][0] =
                        (float)(FUN_1013f880() % STRIKE_RETREAT_SPREAD - STRIKE_RETREAT_HALF) * STRIKE_PI /
                            STRIKE_HALF_TURN +
                        STRIKE_PI;
                    if (FUN_1001a590(index, &self, STRIKE_PROBE_DISTANCE) != 0)
                        break;
                    tries++;
                } while (tries < STRIKE_PROBE_TRIES);
                if (tries == STRIKE_PROBE_TRIES)
                {
                    DAT_1020b430[index].state = STRIKE_STEP_STALK;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].retreatFrames = STRIKE_FAILED_RETREAT;
                    return 0;
                }
            }
            if (DAT_1020b430[index].wait == STRIKE_RETREAT_HOP_FRAME)
            {
                FUN_10065b10(index, STRIKE_HOP_SOUND, 0, 0);
                DAT_10db6ee8[index].AddMotionSpeed(STRIKE_HOP_SPEED);
            }
            if (DAT_1020b430[index].wait < STRIKE_RETREAT_SLOW_END || DAT_1020b430[index].wait > 0xf)
                move.y = STRIKE_RETREAT_SLOW;
            else if (DAT_1020b430[index].wait < STRIKE_RETREAT_FAST_END)
                move.y = STRIKE_RETREAT_FAST;
            move.RotateZ(DAT_10255160[index][0] - DAT_10db6ee8[index].heading);
            moving = 1;
            motion = STRIKE_MOTION_RETREAT;
            if (++DAT_1020b430[index].wait > STRIKE_RETREAT_FRAMES)
            {
                DAT_1020b430[index].state = STRIKE_STEP_STALK;
                DAT_1020b430[index].wait = STRIKE_FRAME_UNSTARTED;
            }
        }
        else if (DAT_1020b430[index].state == STRIKE_STEP_WAVE)
        {
            motion = STRIKE_MOTION_WAVE;
            if (++DAT_1020b430[index].wait == 1)
                restart = 1;
            if (DAT_104c00e0 != 0 && ((unsigned int)DAT_104c00e8 < STRIKE_AREA_FIRST || (unsigned int)DAT_104c00e8 > STRIKE_AREA_LAST))
            {
                DAT_1020b430[index].wait = 2;
                float turn = atan2f(player.x - self.x, player.y - self.y) + STRIKE_PI;
                turn = turn - DAT_10db6ee8[index].heading;
                while (turn > STRIKE_PI)
                    turn = turn - STRIKE_TURN_FULL;
                while (turn < -STRIKE_PI)
                    turn = turn + STRIKE_TURN_FULL;
                if (turn * turn > STRIKE_WAVE_TURN_LIMIT || DAT_104c00e8 == STRIKE_AREA_SHY_A ||
                    DAT_104c00e8 == STRIKE_AREA_SHY_B)
                {
                    DAT_1020b430[index].state = STRIKE_STEP_RETREAT;
                    DAT_1020b430[index].wait = 0;
                }
            }
            if (DAT_1020b430[index].wait > STRIKE_WAVE_FRAMES)
            {
                DAT_1020b430[index].state = STRIKE_STEP_STALK;
                DAT_1020b430[index].wait = STRIKE_FRAME_UNSTARTED;
            }
        }
    }
    if (amount < STRIKE_GREET_RANGE && DAT_1020b430[index].state < 2 && DAT_104c00e0 != 0 &&
        ((unsigned int)DAT_104c00e8 < STRIKE_AREA_FIRST || (unsigned int)DAT_104c00e8 > STRIKE_AREA_LAST) &&
        FUN_1013f880() % STRIKE_ROLL_RANGE < STRIKE_REACT_ROLL)
    {
        if (variant == 0)
            DAT_1020b430[index].state = STRIKE_STEP_RETREAT;
        else if (FUN_1013f880() % 2 == 0 || DAT_104c00e8 == STRIKE_AREA_SHY_A || DAT_104c00e8 == STRIKE_AREA_SHY_B)
            DAT_1020b430[index].state = STRIKE_STEP_RETREAT;
        else
            DAT_1020b430[index].state = STRIKE_STEP_WAVE;
        DAT_1020b430[index].wait = 0;
    }
    if (moving != 0)
    {
        if (DAT_10467920[index] != 0)
        {
            float scale = STRIKE_NO_GROUND_SCALE;
            move.Copy(move * scale);
        }
        FUN_100241e0(index, move.x, move.y, move.z);
        DAT_10802bf8[index].position.Copy(move);
        DAT_10802bf8[index].position.z = 0;
    }
    if (motion > -1)
        DAT_10db6ee8[index].PlayMotion(motion, restart, 5, 1.0f);
    if (DAT_1020b430[index].state == STRIKE_STEP_STALK && DAT_1020b430[index].wait == STRIKE_FRAME_UNSTARTED &&
        amount < STRIKE_REACT_FAR && FUN_1013f880() % STRIKE_ATTACK_AGAIN_ODDS != 0)
    {
        int pick = FUN_1013f880();
        DAT_1020b430[index].wait = 0;
        DAT_1020b430[index].state = pick % STRIKE_ATTACK_COUNT + STRIKE_ATTACK_FIRST;
    }
    if (DAT_1020b430[index].retreatFrames != 0)
    {
        DAT_1020b430[index].retreatFrames--;
        if (DAT_1020b430[index].state == STRIKE_STEP_RETREAT)
        {
            DAT_1020b430[index].state = STRIKE_STEP_STALK;
            DAT_1020b430[index].wait = 0;
        }
    }
    return 0;
}
