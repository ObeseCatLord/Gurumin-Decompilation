// An enemy that stalks the player and breathes fire at him (EN build; JP: decomp/src/enemy_breather.cpp). Far from
// the player it walks up to him, nearer it circles about at a distance, and now and then it stands still and
// breathes (a beam effect from its "@efc_Null" node aimed at the player's chest). A blocked step or a roll makes it
// jump back, and being told to react (g_charToldToReact) sends it to the breath at once.
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
#include "node.h"
#include "scene_start.h"
#include "sound_delayed.h"
#include "vec4.h"

// CharState::values indices the breather keeps its state in.
const int BREATH_VALUE_WAIT = 0;             // frames in the current step
const int BREATH_VALUE_TALK = 1;             // talk id (-1 = unset)
const int BREATH_VALUE_RANGE = 2;            // reaction range (-2 = unset)
const int BREATH_VALUE_STRAFE = 3;           // circling mode: 0 = left, 1 = right, 2..4 = hold
const int BREATH_VALUE_STALLED = 4;          // 1 = the last step was blocked (no ground or a wall ahead)
const int BREATH_VALUE_RETREAT_FRAMES = 9;   // frames left in which a retreat is cut short

#define BREATH_NULL_NODE "@efc_Null"
#define BREATH_STEP_START 0
#define BREATH_STEP_STALK 1            // faces the player and walks up to him while he is far
#define BREATH_STEP_CIRCLE 2           // keeps its distance, strafing; goes back to stalking when he is far
#define BREATH_STEP_BREATHE 3          // stands and breathes
#define BREATH_STEP_RETREAT 5          // jumps back a random way
#define BREATH_PI 3.1415f
#define BREATH_HALF_TURN 180.0f
#define BREATH_FIFTH_TURN 1.2566f      // the angle a circling step is rotated by
#define BREATH_CHEST_HEIGHT 75.0f      // the breath aims this high above the player's feet
#define BREATH_QUARTER_TURN 1.5708f
#define BREATH_FAR 800.0f              // stalking turns into circling when the player is nearer than this
#define BREATH_CIRCLE_FAR 1000.0f      // circling turns back into stalking beyond this
#define BREATH_CIRCLE_NEAR 700.0f      // circling may turn into a retreat nearer than this
#define BREATH_WALK_SPEED (-12.0f)
#define BREATH_CIRCLE_SPEED (-6.0f)
#define BREATH_PROBE_HEIGHT 30.0f      // the move vector's height while a step is tested for ground and walls
#define BREATH_NO_GROUND_SCALE 0.1f    // a character with no ground below it moves at a tenth of the speed
#define BREATH_RETREAT_SLOW (-5.0f)
#define BREATH_RETREAT_FAST (-35.0f)
#define BREATH_JUMP_SPEED 30.0f
#define BREATH_HOP_SPEED 20.0f
#define BREATH_PROBE_DISTANCE 200.0f
#define BREATH_PROBE_TRIES 5
#define BREATH_RETREAT_FRAMES 0x14
#define BREATH_RETREAT_SLOW_END 5
#define BREATH_RETREAT_FAST_END 0x10
#define BREATH_RETREAT_HOP_FRAME 6
#define BREATH_FAILED_RETREAT 10
#define BREATH_RETREAT_SPREAD 0xb4
#define BREATH_RETREAT_HALF 0x5a
#define BREATH_MOTION_WALK 1
#define BREATH_MOTION_STAND 0x12
#define BREATH_MOTION_EXHALE 0x13
#define BREATH_MOTION_END 0x14
#define BREATH_MOTION_RETREAT 0x11
#define BREATH_FRAME_START 0x1c6       // motion frame at which the breath starts
#define BREATH_FRAME_END 0x1c7         // motion frame at which the breath ends
#define BREATH_TURN_FRAMES 10          // frames it keeps turning to the player before breathing
#define BREATH_REST_FRAMES 0x14        // after this frame it may pick an end of the breath
#define BREATH_ENDING 20000            // wait value while the breath winds down
#define BREATH_END_ROLL_RANGE 1000
#define BREATH_END_ROLL 100
#define BREATH_STAND_ROLL 50
#define BREATH_BEAM_EFFECT 0x3e9
#define BREATH_BEAM_SOUND 0x1cb
#define BREATH_SMOKE_EFFECT 0x3ec
#define BREATH_SMOKE_FRAME 0x14
#define BREATH_SMOKE_SOUND 0x1cc
#define BREATH_FLAME_EFFECT 0x36fd
#define BREATH_FLAME_SOUND 0x1ca
#define BREATH_EFFECT_SCALE 1.2f
#define BREATH_AIM_MIN_HEIGHT 75.0f
#define BREATH_AIM_MIN_LENGTH 1.0f
#define BREATH_HOP_SOUND 0x1c6
#define BREATH_STRAFE_MODES 5
#define BREATH_STRAFE_AFTER 0x3c
#define BREATH_STRAFE_ROLL_RANGE 1000
#define BREATH_STRAFE_ROLL 0x50
#define BREATH_STRAFE_LEFT 0
#define BREATH_STRAFE_RIGHT 1
#define BREATH_ROLL_RANGE 10000
#define BREATH_ALARM_ROLL 200
#define BREATH_RETREAT_ROLL 400
#define BREATH_GREETING_ROLL 1000
#define BREATH_UNSET_TALK (-1)
#define BREATH_UNSET_RANGE (-2)
#define BREATH_LEVEL_LIMIT 10

// 0x100ac440 (Enemy_StalkAndBreathe). Does the common upkeep first: it clears the shadow flag, turns a reaction
// request into the breath (state 3) and records a missing ground. While g_charHitLevel is set nothing more happens.
// State 0 binds the "@efc_Null" node. State 1 turns to the player and, beyond 800 units, walks towards him (a
// blocked step jumps and then retreats, state 5); nearer it goes to state 2, which turns to the player with a random
// sideways step (a new mode about every 1.3 s) and may start the breath at random, go back to stalking beyond 1000
// units or retreat inside 700. State 3 stands for ten frames facing the player, then fires the beam, smoke and flame
// effects aimed at his chest, plays the matching sounds and waits for its end motion. State 5 picks a free random
// direction, jumps and walks there. Every state may produce a move, which is applied at the end together with the
// motion. Returns 0.
int Enemy_StalkAndBreathe(int index)
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
        g_charState[index].state = BREATH_STEP_BREATHE;
        g_charState[index].values[BREATH_VALUE_WAIT] = 0;
    }
    g_charToldToReact[index] = 0;
    if (Slot_HasNoGround(index, &self) != 0)
        g_charState[index].values[BREATH_VALUE_STALLED] = 1;
    if (g_charHitLevel[index] > BREATH_LEVEL_LIMIT)
        g_charHitLevel[index] = BREATH_LEVEL_LIMIT;
    if (g_charRebindNode[index] != 0)
    {
        g_charRebindNode[index] = 0;
        g_charNodes[index][0] = g_characterModels[index].root->Find(BREATH_NULL_NODE, 0);
    }
    float aimPitch = 0.0f;   // pitch of the breath towards the player's chest
    if (g_charHitLevel[index] == 0)
    {
        if (g_charState[index].state == BREATH_STEP_START)
        {
            g_charNodes[index][0] = g_characterModels[index].root->Find(BREATH_NULL_NODE, 0);
            g_charState[index].values[BREATH_VALUE_WAIT] = 0;
            if (g_charState[index].values[BREATH_VALUE_TALK] == 0)
                g_charState[index].values[BREATH_VALUE_TALK] = BREATH_UNSET_TALK;
            if (g_charState[index].values[BREATH_VALUE_RANGE] == 0)
                g_charState[index].values[BREATH_VALUE_RANGE] = BREATH_UNSET_RANGE;
            g_charState[index].state = BREATH_STEP_STALK;
        }
        else if (g_charState[index].state == BREATH_STEP_STALK)
        {
            float dy = player.y - self.y;
            float dx = player.x - self.x;
            g_characterModels[index].targetHeading = (float)atan2((double)dx, (double)dy) + BREATH_PI;
            if (distance > BREATH_FAR)
            {
                move.y = BREATH_WALK_SPEED;
                move.RotateZ(-g_characterModels[index].heading);
                moving = 1;
                motion = BREATH_MOTION_WALK;
                move.z = BREATH_PROBE_HEIGHT;
                if (g_characterModels[index].idleMotionEnabled != 0)
                {
                    if (g_characterModels[index].IsClearOfMapLimited(&move) != 0 &&
                        g_characterModels[index].IsClearAny(&move) != 0 && g_slotNoGround[index] == 0)
                    {
                        g_charState[index].values[BREATH_VALUE_STALLED] = 0;
                    }
                    else if (g_charState[index].values[BREATH_VALUE_STALLED] != 0)
                    {
                        g_charState[index].state = BREATH_STEP_RETREAT;
                        g_charState[index].values[BREATH_VALUE_WAIT] = 0;
                    }
                    else
                    {
                        g_characterModels[index].AddMotionSpeed(BREATH_JUMP_SPEED);
                        g_charState[index].values[BREATH_VALUE_STALLED] = 1;
                    }
                }
                move.z = 0;
            }
            else
            {
                g_charState[index].state = BREATH_STEP_CIRCLE;
                g_charState[index].values[BREATH_VALUE_WAIT] = 0;
            }
        }
        else if (g_charState[index].state == BREATH_STEP_CIRCLE)
        {
            g_characterModels[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + BREATH_PI;
            if (g_charState[index].values[BREATH_VALUE_STRAFE] == BREATH_STRAFE_LEFT)
            {
                move.y = BREATH_CIRCLE_SPEED;
                move.RotateZ(BREATH_FIFTH_TURN - g_characterModels[index].heading);
                moving = 1;
                motion = 0;
            }
            else if (g_charState[index].values[BREATH_VALUE_STRAFE] == BREATH_STRAFE_RIGHT)
            {
                move.y = BREATH_CIRCLE_SPEED;
                move.RotateZ(-g_characterModels[index].heading - BREATH_FIFTH_TURN);
                moving = 1;
                motion = 0;
            }
            else
                motion = 2;
            int waited = g_charState[index].values[BREATH_VALUE_WAIT] + 1;
            g_charState[index].values[BREATH_VALUE_WAIT] = waited;
            if (waited > BREATH_STRAFE_AFTER &&
                Random() % BREATH_STRAFE_ROLL_RANGE < BREATH_STRAFE_ROLL)
            {
                g_charState[index].values[BREATH_VALUE_STRAFE] = Random() % BREATH_STRAFE_MODES;
                g_charState[index].values[BREATH_VALUE_WAIT] = 0;
            }
            if (g_slotNoGround[index] != 0 || Random() % BREATH_ROLL_RANGE < BREATH_ALARM_ROLL)
                g_charToldToReact[index] = 1;
            if (distance > BREATH_CIRCLE_FAR)
            {
                g_charState[index].state = BREATH_STEP_STALK;
                g_charState[index].values[BREATH_VALUE_WAIT] = 0;
            }
            if (distance < BREATH_CIRCLE_NEAR && Random() % BREATH_ROLL_RANGE < BREATH_RETREAT_ROLL)
            {
                g_charState[index].state = BREATH_STEP_RETREAT;
                g_charState[index].values[BREATH_VALUE_WAIT] = 0;
            }
        }
        else if (g_charState[index].state == BREATH_STEP_BREATHE)
        {
            motion = BREATH_MOTION_STAND;
            if (g_characterModels[index].motionFrame == BREATH_FRAME_START)
                g_charState[index].values[BREATH_VALUE_WAIT] = 1;
            if (g_charState[index].values[BREATH_VALUE_WAIT] != 0)
                g_charState[index].values[BREATH_VALUE_WAIT]++;
            if (g_charState[index].values[BREATH_VALUE_WAIT] < BREATH_TURN_FRAMES)
            {
                g_characterModels[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + BREATH_PI;
            }
            else if (g_charState[index].values[BREATH_VALUE_WAIT] == BREATH_TURN_FRAMES)
            {
                if (g_charNodes[index][0] != 0)
                {
                    EffectOptions options;
                    Matrix4 shot;
                    Vec4 offset;
                    options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
                    options.scale = BREATH_EFFECT_SCALE;
                    Vec4 target;
                    target.Copy(player);
                    target.z = target.z + BREATH_CHEST_HEIGHT;
                    offset.Copy(target - g_charNodes[index][0]->worldMatrix.GetTranslation());
                    if ((float)fabs(offset.z) > BREATH_AIM_MIN_HEIGHT && offset.Length() > BREATH_AIM_MIN_LENGTH)
                    {
                        if (offset.z > 0.0f)
                            aimPitch = acosf(offset.Length2D() / offset.Length());
                        else
                            aimPitch = -acosf(offset.Length2D() / offset.Length());
                    }
                    offset.Fill(0.0f);
                    shot = RotationX(aimPitch) * RotationZ(g_characterModels[index].targetHeading);
                    shot.SetTranslation(g_charNodes[index][0]->worldMatrix.GetTranslation());
                    options.owner = index + EFFECT_OWNER_BASE;
                    Char_StartEffect(index, BREATH_BEAM_EFFECT, offset, -1, -1, &g_charNodes[index][0]->worldMatrix, 0, 0,
                                 &shot, &options);
                    Character_PlaySound(index, BREATH_BEAM_SOUND, 0, 0);
                    if (g_charBehaviour[index] != 0)
                    {
                        Char_StartEffect(index, BREATH_SMOKE_EFFECT, offset, -1, -1, &g_charNodes[index][0]->worldMatrix,
                                     0, BREATH_SMOKE_FRAME, &shot, &options);
                        shot = RotationX(aimPitch - BREATH_QUARTER_TURN) * RotationZ(g_characterModels[index].targetHeading);
                        shot.SetTranslation(g_charNodes[index][0]->worldMatrix.GetTranslation());
                        Char_StartEffect(index, BREATH_FLAME_EFFECT, offset, -1, -1, &g_charNodes[index][0]->worldMatrix,
                                     0, BREATH_SMOKE_FRAME, &shot, &options);
                        Sound_PlayDelayed(BREATH_SMOKE_SOUND, BREATH_SMOKE_FRAME, &self);
                        Sound_PlayDelayed(BREATH_FLAME_SOUND, BREATH_SMOKE_FRAME, &self);
                    }
                    else
                    {
                        Char_StartEffect(index, BREATH_SMOKE_EFFECT, offset, -1, -1, &g_charNodes[index][0]->worldMatrix,
                                     0, BREATH_SMOKE_FRAME, &shot, &options);
                        Sound_PlayDelayed(BREATH_SMOKE_SOUND, BREATH_SMOKE_FRAME, &self);
                    }
                }
            }
            else if (g_charState[index].values[BREATH_VALUE_WAIT] <= BREATH_REST_FRAMES || g_charState[index].values[BREATH_VALUE_WAIT] >= BREATH_ENDING)
            {
                if (g_charState[index].values[BREATH_VALUE_WAIT] > BREATH_ENDING - 1)
                {
                    motion = BREATH_MOTION_END;
                    if (g_characterModels[index].motionFrame == BREATH_FRAME_END)
                    {
                        g_charState[index].values[BREATH_VALUE_WAIT] = 0;
                        g_charState[index].state = BREATH_STEP_STALK;
                    }
                }
            }
            else
            {
                if (Random() % BREATH_END_ROLL_RANGE < BREATH_END_ROLL)
                {
                    g_charState[index].values[BREATH_VALUE_WAIT] = BREATH_ENDING;
                    motion = BREATH_MOTION_END;
                }
                else if (Random() % BREATH_END_ROLL_RANGE < BREATH_STAND_ROLL)
                {
                    g_charState[index].values[BREATH_VALUE_WAIT] = 1;
                    motion = BREATH_MOTION_EXHALE;
                }
            }
        }
        else if (g_charState[index].state == BREATH_STEP_RETREAT)
        {
            if (g_charState[index].values[BREATH_VALUE_WAIT] == 0)
            {
                int tries = 0;
                do
                {
                    g_shotParams[index][0] =
                        (float)(Random() % BREATH_RETREAT_SPREAD - BREATH_RETREAT_HALF) * BREATH_PI /
                            BREATH_HALF_TURN +
                        BREATH_PI;
                    if (Char_ProbeGround(index, &self, BREATH_PROBE_DISTANCE) != 0)
                        break;
                    tries++;
                } while (tries < BREATH_PROBE_TRIES);
                if (tries == BREATH_PROBE_TRIES)
                {
                    g_charState[index].values[BREATH_VALUE_RETREAT_FRAMES] = BREATH_FAILED_RETREAT;
                    g_charState[index].values[BREATH_VALUE_WAIT] = 0;
                    g_charState[index].state = BREATH_STEP_STALK;
                    return 0;
                }
            }
            if (g_charState[index].values[BREATH_VALUE_WAIT] == BREATH_RETREAT_HOP_FRAME)
            {
                Character_PlaySound(index, BREATH_HOP_SOUND, 0, 0);
                g_characterModels[index].AddMotionSpeed(BREATH_HOP_SPEED);
            }
            if (g_charState[index].values[BREATH_VALUE_WAIT] < BREATH_RETREAT_SLOW_END || g_charState[index].values[BREATH_VALUE_WAIT] > 0xf)
                move.y = BREATH_RETREAT_SLOW;
            else if (g_charState[index].values[BREATH_VALUE_WAIT] < BREATH_RETREAT_FAST_END)
                move.y = BREATH_RETREAT_FAST;
            move.RotateZ(g_shotParams[index][0] - g_characterModels[index].heading);
            moving = 1;
            motion = BREATH_MOTION_RETREAT;
            int retreated = g_charState[index].values[BREATH_VALUE_WAIT] + 1;
            g_charState[index].values[BREATH_VALUE_WAIT] = retreated;
            if (retreated > BREATH_RETREAT_FRAMES)
            {
                g_charState[index].values[BREATH_VALUE_WAIT] = 0;
                g_charState[index].state = BREATH_STEP_STALK;
            }
        }
    }
    if (g_charState[index].state < 2 && g_greetingFrames != 0 && Random() % BREATH_ROLL_RANGE < BREATH_GREETING_ROLL)
    {
        g_charState[index].state = BREATH_STEP_RETREAT;
        g_charState[index].values[BREATH_VALUE_WAIT] = 0;
    }
    if (moving != 0)
    {
        if (g_slotNoGround[index] != 0)
        {
            float scale = BREATH_NO_GROUND_SCALE;
            move.Copy(move * scale);
        }
        Character_SetMove(index, move.x, move.y, move.z);
        g_shotSlots[index].position.Copy(move);
        g_shotSlots[index].position.z = 0;
    }
    if (motion > -1)
        g_characterModels[index].PlayMotion(motion, 0, 5, 1.0f);
    if (g_charState[index].values[BREATH_VALUE_RETREAT_FRAMES] != 0)
    {
        g_charState[index].values[BREATH_VALUE_RETREAT_FRAMES]--;
        if (g_charState[index].state == BREATH_STEP_RETREAT)
        {
            g_charState[index].values[BREATH_VALUE_WAIT] = 0;
            g_charState[index].state = BREATH_STEP_STALK;
        }
    }
    return 0;
}
