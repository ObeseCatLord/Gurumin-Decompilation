// An enemy that stalks the player and lunges at him with a swing of its hand (EN build; JP: decomp/src/enemy_lunger.cpp).
// It follows the same stalk / circle / retreat steps as the fire breather (enemy_breather.cpp) but closes in much
// further and, when told to react, runs one of two melee lunges that leave a streak and a swing effect.
#include <math.h>

#include "anim_model.h"
#include "barrage_enemy.h"
#include "breath_enemy.h"
#include "char.h"
#include "char_dispatch.h"
#include "char_move.h"
#include "char_shots.h"
#include "collide_query.h"
#include "effect_manager.h"
#include "effect_options.h"
#include "engine.h"
#include "lunge_enemy.h"
#include "node.h"
#include "scene_start.h"
#include "vec4.h"

// CharState::values indices the lunger keeps its state in.
const int LUNGE_VALUE_WAIT = 0;             // frames in the current step
const int LUNGE_VALUE_TALK = 1;             // talk id (-1 = unset)
const int LUNGE_VALUE_RANGE = 2;            // reaction range (-2 = unset)
const int LUNGE_VALUE_STRAFE = 3;           // circling mode: 0 = left, 1 = right, 2 = hold
const int LUNGE_VALUE_STALLED = 4;          // 1 = the last step was blocked (no ground or a wall ahead)
const int LUNGE_VALUE_RETREAT_FRAMES = 9;   // frames left in which a retreat is cut short

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

// The swing effects pass the hand node's matrix in the `value` argument of Char_StartEffect, which the effect record
// holds as an int (the parameter is declared int, so the pointer is cast).
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
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    float distance = (player - self).Length2D();
    g_charTalkPending[index] = 0;
    if (g_charToldToReact[index] != 0 && g_charState[index].state != 0)
    {
        g_charState[index].state = Random() % LUNGE_ATTACK_ODDS != 0 ? LUNGE_STEP_ATTACK_A : LUNGE_STEP_ATTACK_B;
        g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
    }
    g_charToldToReact[index] = 0;
    if (Slot_HasNoGround(index, &self) != 0)
        g_charState[index].values[LUNGE_VALUE_STALLED] = 1;
    if (g_charHitLevel[index] > LUNGE_LEVEL_LIMIT)
        g_charHitLevel[index] = LUNGE_LEVEL_LIMIT;
    if (g_charRebindNode[index] != 0)
    {
        g_charRebindNode[index] = 0;
        g_charNodes[index][0] = g_characterModels[index].root->Find(LUNGE_NULL_NODE, 0);
    }
    if (g_charHitLevel[index] == 0)
    {
        if (g_charState[index].state == LUNGE_STEP_START)
        {
            g_charNodes[index][0] = g_characterModels[index].root->Find(LUNGE_NULL_NODE, 0);
            g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
            if (g_charState[index].values[LUNGE_VALUE_TALK] == 0)
                g_charState[index].values[LUNGE_VALUE_TALK] = LUNGE_UNSET_TALK;
            if (g_charState[index].values[LUNGE_VALUE_RANGE] == 0)
                g_charState[index].values[LUNGE_VALUE_RANGE] = LUNGE_UNSET_RANGE;
            g_charState[index].state = LUNGE_STEP_STALK;
        }
        else if (g_charState[index].state == LUNGE_STEP_STALK)
        {
            float dy = player.y - self.y;
            float dx = player.x - self.x;
            g_characterModels[index].targetHeading = (float)atan2((double)dx, (double)dy) + LUNGE_PI;
            if (distance > LUNGE_FAR)
            {
                move.y = LUNGE_WALK_SPEED;
                move.RotateZ(-g_characterModels[index].heading);
                moving = 1;
                motion = LUNGE_MOTION_WALK;
                move.z = LUNGE_PROBE_HEIGHT;
                if (g_characterModels[index].idleMotionEnabled != 0)
                {
                    if (g_characterModels[index].IsClearOfMapLimited(&move) != 0 &&
                        g_characterModels[index].IsClearAny(&move) != 0 && g_slotNoGround[index] == 0)
                    {
                        g_charState[index].values[LUNGE_VALUE_STALLED] = 0;
                    }
                    else if (g_charState[index].values[LUNGE_VALUE_STALLED] != 0)
                    {
                        g_charState[index].state = LUNGE_STEP_RETREAT;
                        g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
                    }
                    else
                    {
                        g_characterModels[index].AddMotionSpeed(LUNGE_JUMP_SPEED);
                        g_charState[index].values[LUNGE_VALUE_STALLED] = 1;
                    }
                }
                move.z = 0;
            }
            else
            {
                g_charState[index].state = LUNGE_STEP_CIRCLE;
                g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
            }
        }
        else if (g_charState[index].state == LUNGE_STEP_CIRCLE)
        {
            g_characterModels[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + LUNGE_PI;
            if (g_charState[index].values[LUNGE_VALUE_STRAFE] == LUNGE_STRAFE_LEFT)
            {
                move.y = LUNGE_CIRCLE_SPEED;
                move.RotateZ(LUNGE_FIFTH_TURN - g_characterModels[index].heading);
                moving = 1;
                motion = 0;
            }
            else if (g_charState[index].values[LUNGE_VALUE_STRAFE] == LUNGE_STRAFE_RIGHT)
            {
                move.y = LUNGE_CIRCLE_SPEED;
                move.RotateZ(-g_characterModels[index].heading - LUNGE_FIFTH_TURN);
                moving = 1;
                motion = 0;
            }
            else
                motion = 2;
            int waited = g_charState[index].values[LUNGE_VALUE_WAIT] + 1;
            g_charState[index].values[LUNGE_VALUE_WAIT] = waited;
            if (waited > LUNGE_STRAFE_AFTER && Random() % LUNGE_STRAFE_ROLL_RANGE < LUNGE_STRAFE_ROLL)
            {
                g_charState[index].values[LUNGE_VALUE_STRAFE] = Random() % LUNGE_STRAFE_MODES;
                g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
            }
            if (g_slotNoGround[index] != 0 || Random() % LUNGE_ROLL_RANGE < LUNGE_ALARM_ROLL)
                g_charToldToReact[index] = 1;
            if (distance > LUNGE_CIRCLE_FAR)
            {
                g_charState[index].state = LUNGE_STEP_STALK;
                g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
            }
            if (distance < LUNGE_CIRCLE_NEAR && Random() % LUNGE_ROLL_RANGE < LUNGE_RETREAT_ROLL)
            {
                g_charState[index].state = LUNGE_STEP_RETREAT;
                g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
            }
        }
        else if (g_charState[index].state == LUNGE_STEP_ATTACK_A)
        {
            motion = LUNGE_A_MOTION;
            if (g_characterModels[index].motionFrame == LUNGE_A_WINDUP_FRAME)
            {
                g_charState[index].values[LUNGE_VALUE_WAIT] = 1;
                Character_PlaySound(index, LUNGE_A_WINDUP_SOUND, 0, 0);
            }
            if (g_charState[index].values[LUNGE_VALUE_WAIT] != 0)
                g_charState[index].values[LUNGE_VALUE_WAIT]++;
            if (g_charState[index].values[LUNGE_VALUE_WAIT] < LUNGE_TURN_FRAMES)
                g_characterModels[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + LUNGE_PI;
            if (g_charState[index].values[LUNGE_VALUE_WAIT] >= LUNGE_A_DASH_FROM)
            {
                if (g_charState[index].values[LUNGE_VALUE_WAIT] < LUNGE_A_KICK_COUNT)
                {
                    move.y = LUNGE_DASH_SLOW;
                    move.RotateZ(-g_characterModels[index].heading);
                    moving = 1;
                }
                else if (g_charState[index].values[LUNGE_VALUE_WAIT] == LUNGE_A_KICK_COUNT)
                {
                    move.y = LUNGE_DASH_FAST;
                    move.RotateZ(-g_characterModels[index].heading);
                    moving = 1;
                    g_characterModels[index].AddMotionSpeed(LUNGE_BOOST_SPEED);
                }
                else if (g_charState[index].values[LUNGE_VALUE_WAIT] < LUNGE_A_RUN_END)
                {
                    move.y = LUNGE_DASH_FASTEST;
                    if (g_charState[index].values[LUNGE_VALUE_WAIT] >= LUNGE_A_FASTEST_UNTIL)
                        move.y = LUNGE_DASH_FAST;
                    move.RotateZ(-g_characterModels[index].heading);
                    moving = 1;
                    EffectOptions options;
                    options.m_0000 = 0;
                    options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
                    options.scale = LUNGE_A_TRAIL_SIZE;
                    options.m_000e = LUNGE_TRAIL_MODE;
                    Vec4 offset;
                    if (g_charNodes[index][0] != 0)
                    {
                        options.owner = index + EFFECT_OWNER_BASE;
                        Char_StartEffect(index, LUNGE_TRAIL_EFFECT, offset, -1, -1, &g_charNodes[index][0]->worldMatrix, 0,
                                         0, 0, &options);
                    }
                }
                else if (g_charState[index].values[LUNGE_VALUE_WAIT] == LUNGE_DASH_END)
                {
                    g_charState[index].state = LUNGE_STEP_STALK;
                    g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
                }
            }
            if (g_characterModels[index].motionFrame == LUNGE_A_SWING_FRAME && g_charNodes[index][0] != 0 &&
                g_characterModels[index].root->Find(LUNGE_HAND_NODE, 0) != 0)
            {
                Matrix4 swing;
                EffectOptions options;
                options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
                options.m_0000 = 0;
                options.scale = LUNGE_A_TRAIL_SIZE;
                options.m_000e = LUNGE_TRAIL_MODE;
                options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
                Vec4 offset;
                swing = RotationZ(g_characterModels[index].heading);
                options.owner = index + EFFECT_OWNER_BASE;
                Char_StartEffect(index, LUNGE_A_SWING_EFFECT, offset, -1, -1, &g_charNodes[index][0]->worldMatrix,
                                 (int)&g_characterModels[index].root->Find(LUNGE_HAND_NODE, 0)->worldMatrix, 0, &swing,
                                 &options);
            }
        }
        else if (g_charState[index].state == LUNGE_STEP_ATTACK_B)
        {
            motion = LUNGE_B_MOTION;
            if (g_characterModels[index].motionFrame == LUNGE_B_WINDUP_FRAME)
            {
                g_charState[index].values[LUNGE_VALUE_WAIT] = 1;
                Character_PlaySound(index, LUNGE_B_WINDUP_SOUND, 0, 0);
            }
            if (g_charState[index].values[LUNGE_VALUE_WAIT] != 0)
                g_charState[index].values[LUNGE_VALUE_WAIT]++;
            if (g_charState[index].values[LUNGE_VALUE_WAIT] < LUNGE_TURN_FRAMES)
                g_characterModels[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + LUNGE_PI;
            if (g_charState[index].values[LUNGE_VALUE_WAIT] >= LUNGE_B_DASH_FROM)
            {
                if (g_charState[index].values[LUNGE_VALUE_WAIT] < LUNGE_B_KICK_COUNT)
                {
                    move.y = LUNGE_DASH_SLOW;
                    move.RotateZ(-g_characterModels[index].heading);
                    moving = 1;
                }
                else if (g_charState[index].values[LUNGE_VALUE_WAIT] == LUNGE_B_KICK_COUNT)
                {
                    move.y = LUNGE_DASH_FAST;
                    move.RotateZ(-g_characterModels[index].heading);
                    moving = 1;
                    g_characterModels[index].AddMotionSpeed(LUNGE_BOOST_SPEED);
                }
                else if (g_charState[index].values[LUNGE_VALUE_WAIT] < LUNGE_B_RUN_END)
                {
                    move.y = LUNGE_DASH_FASTEST;
                    if (g_charState[index].values[LUNGE_VALUE_WAIT] >= LUNGE_B_FASTEST_UNTIL)
                        move.y = LUNGE_DASH_FAST;
                    move.RotateZ(-g_characterModels[index].heading);
                    moving = 1;
                    EffectOptions options;
                    options.m_0000 = 0;
                    options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
                    options.scale = LUNGE_B_TRAIL_SIZE;
                    options.m_000e = LUNGE_TRAIL_MODE;
                    Vec4 offset;
                    if (g_charNodes[index][0] != 0)
                    {
                        options.owner = index + EFFECT_OWNER_BASE;
                        Char_StartEffect(index, LUNGE_TRAIL_EFFECT, offset, -1, -1, &g_charNodes[index][0]->worldMatrix, 0,
                                         0, 0, &options);
                    }
                }
                else if (g_charState[index].values[LUNGE_VALUE_WAIT] == LUNGE_DASH_END)
                {
                    g_charState[index].state = LUNGE_STEP_STALK;
                    g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
                }
            }
            if (g_characterModels[index].motionFrame == LUNGE_B_SWING_FRAME && g_charNodes[index][0] != 0 &&
                g_characterModels[index].root->Find(LUNGE_HAND_NODE, 0) != 0)
            {
                Vec4 offset;
                EffectOptions options;
                Matrix4 swing;
                options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
                options.m_0000 = 0;
                options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
                options.scale = LUNGE_B_TRAIL_SIZE;
                options.m_000e = LUNGE_TRAIL_MODE;
                swing = RotationZ(g_characterModels[index].heading);
                options.owner = index + EFFECT_OWNER_BASE;
                Char_StartEffect(index, LUNGE_B_SWING_EFFECT, offset, -1, -1, &g_charNodes[index][0]->worldMatrix,
                                 (int)&g_characterModels[index].root->Find(LUNGE_HAND_NODE, 0)->worldMatrix, 0, &swing,
                                 &options);
            }
        }
        else if (g_charState[index].state == LUNGE_STEP_RETREAT)
        {
            if (g_charState[index].values[LUNGE_VALUE_WAIT] == 0)
            {
                int tries = 0;
                do
                {
                    g_shotParams[index][0] =
                        (float)(Random() % LUNGE_RETREAT_SPREAD - LUNGE_RETREAT_HALF) * LUNGE_PI / LUNGE_HALF_TURN +
                        LUNGE_PI;
                    if (Char_ProbeGround(index, &self, LUNGE_PROBE_DISTANCE) != 0)
                        break;
                    tries++;
                } while (tries < LUNGE_PROBE_TRIES);
                if (tries == LUNGE_PROBE_TRIES)
                {
                    g_charState[index].values[LUNGE_VALUE_RETREAT_FRAMES] = LUNGE_FAILED_RETREAT;
                    g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
                    g_charState[index].state = LUNGE_STEP_STALK;
                    return 0;
                }
            }
            if (g_charState[index].values[LUNGE_VALUE_WAIT] == LUNGE_RETREAT_HOP_FRAME)
            {
                Character_PlaySound(index, LUNGE_HOP_SOUND, 0, 0);
                g_characterModels[index].AddMotionSpeed(LUNGE_HOP_SPEED);
            }
            if (g_charState[index].values[LUNGE_VALUE_WAIT] < LUNGE_RETREAT_SLOW_END ||
                g_charState[index].values[LUNGE_VALUE_WAIT] > 0xf)
                move.y = LUNGE_RETREAT_SLOW;
            else if (g_charState[index].values[LUNGE_VALUE_WAIT] < LUNGE_RETREAT_FAST_END)
                move.y = LUNGE_RETREAT_FAST;
            move.RotateZ(g_shotParams[index][0] - g_characterModels[index].heading);
            moving = 1;
            motion = LUNGE_MOTION_RETREAT;
            int retreated = g_charState[index].values[LUNGE_VALUE_WAIT] + 1;
            g_charState[index].values[LUNGE_VALUE_WAIT] = retreated;
            if (retreated > LUNGE_RETREAT_FRAMES)
            {
                g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
                g_charState[index].state = LUNGE_STEP_STALK;
            }
        }
    }
    if (g_charState[index].state < 2 && g_greetingFrames != 0 && Random() % LUNGE_ROLL_RANGE < LUNGE_GREETING_ROLL)
    {
        g_charState[index].state = LUNGE_STEP_RETREAT;
        g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
    }
    if (moving != 0)
    {
        if (g_slotNoGround[index] != 0)
        {
            float scale = LUNGE_NO_GROUND_SCALE;
            move.Copy(move * scale);
        }
        Character_SetMove(index, move.x, move.y, move.z);
        g_shotSlots[index].position.Copy(move);
        g_shotSlots[index].position.z = 0;
    }
    if (motion > -1)
        g_characterModels[index].PlayMotion(motion, 0, 5, 1.0f);
    if (g_charState[index].values[LUNGE_VALUE_RETREAT_FRAMES] != 0)
    {
        g_charState[index].values[LUNGE_VALUE_RETREAT_FRAMES]--;
        if (g_charState[index].state == LUNGE_STEP_RETREAT)
        {
            g_charState[index].values[LUNGE_VALUE_WAIT] = 0;
            g_charState[index].state = LUNGE_STEP_STALK;
        }
    }
    return 0;
}
