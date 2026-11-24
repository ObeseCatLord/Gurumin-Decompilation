// An enemy that stalks the player and breathes fire at him (0x100ac440, a per-kind behaviour of 0x100f9670). Far from
// the player it walks up to him, nearer it circles about at a distance, and now and then it stands still and
// breathes (a beam effect from its "@efc_Null" node aimed at the player's chest). A blocked step or a roll makes it
// jump back, and being told to react (DAT_10210b30) sends it to the breath at once.

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
#include "breath_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40
Matrix4 RotationX(float angle);   // 0x101b1ad0

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1001a450(int slot, const Vec4 *point);                     // Slot_HasNoGround
extern "C" int FUN_1001a590(int slot, const Vec4 *unused, float distance);    // ground probe along the slot's angle
extern "C" int FUN_10054550(int sound, int delay, const Vec4 *position);      // queued sound
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

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
// request into the breath (state 3) and records a missing ground. While DAT_102550e0 is set nothing more happens.
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
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float amount = (player - self).Length2D();   // reused: aim pitch and the no-ground slow-down below
    DAT_10255a88[index] = 0;
    if (DAT_10210b30[index] != 0 && DAT_1020b430[index].state != 0)
    {
        DAT_1020b430[index].state = BREATH_STEP_BREATHE;
        DAT_1020b430[index].wait = 0;
    }
    DAT_10210b30[index] = 0;
    if (FUN_1001a450(index, &self) != 0)
        DAT_1020b430[index].stalled = 1;
    if (DAT_102550e0[index] > BREATH_LEVEL_LIMIT)
        DAT_102550e0[index] = BREATH_LEVEL_LIMIT;
    if (DAT_10468730[index] != 0)
    {
        DAT_10468730[index] = 0;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(BREATH_NULL_NODE, 0);
    }
    if (DAT_102550e0[index] == 0)
    {
        if (DAT_1020b430[index].state == BREATH_STEP_START)
        {
            DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(BREATH_NULL_NODE, 0);
            DAT_1020b430[index].wait = 0;
            if (DAT_1020b430[index].talk == 0)
                DAT_1020b430[index].talk = BREATH_UNSET_TALK;
            if (DAT_1020b430[index].range == 0)
                DAT_1020b430[index].range = BREATH_UNSET_RANGE;
            DAT_1020b430[index].state = BREATH_STEP_STALK;
        }
        else if (DAT_1020b430[index].state == BREATH_STEP_STALK)
        {
            float dy = player.y - self.y;
            float dx = player.x - self.x;
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)dx, (double)dy) + BREATH_PI;
            if (amount > BREATH_FAR)
            {
                move.y = BREATH_WALK_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                moving = 1;
                motion = BREATH_MOTION_WALK;
                move.z = BREATH_PROBE_HEIGHT;
                if (DAT_10db6ee8[index].m_238c != 0)
                {
                    if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) != 0 &&
                        DAT_10db6ee8[index].IsClearAny(&move) != 0 && DAT_10467920[index] == 0)
                    {
                        DAT_1020b430[index].stalled = 0;
                    }
                    else if (DAT_1020b430[index].stalled != 0)
                    {
                        DAT_1020b430[index].state = BREATH_STEP_RETREAT;
                        DAT_1020b430[index].wait = 0;
                    }
                    else
                    {
                        DAT_10db6ee8[index].AddMotionSpeed(BREATH_JUMP_SPEED);
                        DAT_1020b430[index].stalled = 1;
                    }
                }
                move.z = 0;
            }
            else
            {
                DAT_1020b430[index].state = BREATH_STEP_CIRCLE;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == BREATH_STEP_CIRCLE)
        {
            DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + BREATH_PI;
            if (DAT_1020b430[index].strafeMode == BREATH_STRAFE_LEFT)
            {
                move.y = BREATH_CIRCLE_SPEED;
                move.RotateZ(BREATH_FIFTH_TURN - DAT_10db6ee8[index].heading);
                moving = 1;
                motion = 0;
            }
            else if (DAT_1020b430[index].strafeMode == BREATH_STRAFE_RIGHT)
            {
                move.y = BREATH_CIRCLE_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading - BREATH_FIFTH_TURN);
                moving = 1;
                motion = 0;
            }
            else
                motion = 2;
            if (++DAT_1020b430[index].wait > BREATH_STRAFE_AFTER &&
                FUN_1013f880() % BREATH_STRAFE_ROLL_RANGE < BREATH_STRAFE_ROLL)
            {
                DAT_1020b430[index].strafeMode = FUN_1013f880() % BREATH_STRAFE_MODES;
                DAT_1020b430[index].wait = 0;
            }
            if (DAT_10467920[index] != 0 || FUN_1013f880() % BREATH_ROLL_RANGE < BREATH_ALARM_ROLL)
                DAT_10210b30[index] = 1;
            if (amount > BREATH_CIRCLE_FAR)
            {
                DAT_1020b430[index].state = BREATH_STEP_STALK;
                DAT_1020b430[index].wait = 0;
            }
            if (amount < BREATH_CIRCLE_NEAR && FUN_1013f880() % BREATH_ROLL_RANGE < BREATH_RETREAT_ROLL)
            {
                DAT_1020b430[index].state = BREATH_STEP_RETREAT;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == BREATH_STEP_BREATHE)
        {
            motion = BREATH_MOTION_STAND;
            if (DAT_10db6ee8[index].motionFrame == BREATH_FRAME_START)
                DAT_1020b430[index].wait = 1;
            if (DAT_1020b430[index].wait != 0)
                DAT_1020b430[index].wait++;
            if (DAT_1020b430[index].wait < BREATH_TURN_FRAMES)
            {
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + BREATH_PI;
            }
            else if (DAT_1020b430[index].wait == BREATH_TURN_FRAMES)
            {
                if (DAT_1021bdf8[index][0] != 0)
                {
                    EffectOptions options;
                    Matrix4 shot;
                    Vec4 offset;
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0008 = BREATH_EFFECT_SCALE;
                    amount = 0.0f;
                    Vec4 target;
                    target.Copy(player);
                    target.z = target.z + BREATH_CHEST_HEIGHT;
                    offset.Copy(target - DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
                    if (floorf(offset.z) > BREATH_AIM_MIN_HEIGHT && offset.Length() > BREATH_AIM_MIN_LENGTH)
                    {
                        if (offset.z > 0.0f)
                            amount = acosf(offset.Length2D() / offset.Length());
                        else
                            amount = -acosf(offset.Length2D() / offset.Length());
                    }
                    offset.Fill(0.0f);
                    shot = RotationX(amount) * RotationZ(DAT_10db6ee8[index].targetHeading);
                    shot.SetTranslation(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
                    options.owner = index + EFFECT_OWNER_BASE;
                    FUN_1001d8b0(index, BREATH_BEAM_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0,
                                 &shot, &options);
                    FUN_10065b10(index, BREATH_BEAM_SOUND, 0, 0);
                    if (DAT_1020b230[index] != 0)
                    {
                        FUN_1001d8b0(index, BREATH_SMOKE_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                                     0, BREATH_SMOKE_FRAME, &shot, &options);
                        shot = RotationX(amount - BREATH_QUARTER_TURN) * RotationZ(DAT_10db6ee8[index].targetHeading);
                        shot.SetTranslation(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
                        FUN_1001d8b0(index, BREATH_FLAME_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                                     0, BREATH_SMOKE_FRAME, &shot, &options);
                        FUN_10054550(BREATH_SMOKE_SOUND, BREATH_SMOKE_FRAME, &self);
                        FUN_10054550(BREATH_FLAME_SOUND, BREATH_SMOKE_FRAME, &self);
                    }
                    else
                    {
                        FUN_1001d8b0(index, BREATH_SMOKE_EFFECT, offset, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix,
                                     0, BREATH_SMOKE_FRAME, &shot, &options);
                        FUN_10054550(BREATH_SMOKE_SOUND, BREATH_SMOKE_FRAME, &self);
                    }
                }
            }
            else if (DAT_1020b430[index].wait <= BREATH_REST_FRAMES || DAT_1020b430[index].wait >= BREATH_ENDING)
            {
                if (DAT_1020b430[index].wait > BREATH_ENDING - 1)
                {
                    motion = BREATH_MOTION_END;
                    if (DAT_10db6ee8[index].motionFrame == BREATH_FRAME_END)
                    {
                        DAT_1020b430[index].state = BREATH_STEP_STALK;
                        DAT_1020b430[index].wait = 0;
                    }
                }
            }
            else
            {
                if (FUN_1013f880() % BREATH_END_ROLL_RANGE < BREATH_END_ROLL)
                {
                    DAT_1020b430[index].wait = BREATH_ENDING;
                    motion = BREATH_MOTION_END;
                }
                else if (FUN_1013f880() % BREATH_END_ROLL_RANGE < BREATH_STAND_ROLL)
                {
                    DAT_1020b430[index].wait = 1;
                    motion = BREATH_MOTION_EXHALE;
                }
            }
        }
        else if (DAT_1020b430[index].state == BREATH_STEP_RETREAT)
        {
            if (DAT_1020b430[index].wait == 0)
            {
                int tries = 0;
                do
                {
                    DAT_10255160[index][0] =
                        (float)(FUN_1013f880() % BREATH_RETREAT_SPREAD - BREATH_RETREAT_HALF) * BREATH_PI /
                            BREATH_HALF_TURN +
                        BREATH_PI;
                    if (FUN_1001a590(index, &self, BREATH_PROBE_DISTANCE) != 0)
                        break;
                    tries++;
                } while (tries < BREATH_PROBE_TRIES);
                if (tries == BREATH_PROBE_TRIES)
                {
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].retreatFrames = BREATH_FAILED_RETREAT;
                    DAT_1020b430[index].state = BREATH_STEP_STALK;
                    return 0;
                }
            }
            if (DAT_1020b430[index].wait == BREATH_RETREAT_HOP_FRAME)
            {
                FUN_10065b10(index, BREATH_HOP_SOUND, 0, 0);
                DAT_10db6ee8[index].AddMotionSpeed(BREATH_HOP_SPEED);
            }
            if (DAT_1020b430[index].wait < BREATH_RETREAT_SLOW_END || DAT_1020b430[index].wait > 0xf)
                move.y = BREATH_RETREAT_SLOW;
            else if (DAT_1020b430[index].wait < BREATH_RETREAT_FAST_END)
                move.y = BREATH_RETREAT_FAST;
            move.RotateZ(DAT_10255160[index][0] - DAT_10db6ee8[index].heading);
            moving = 1;
            motion = BREATH_MOTION_RETREAT;
            if (++DAT_1020b430[index].wait > BREATH_RETREAT_FRAMES)
            {
                DAT_1020b430[index].state = BREATH_STEP_STALK;
                DAT_1020b430[index].wait = 0;
            }
        }
    }
    if (DAT_1020b430[index].state < 2 && DAT_104c00e0 != 0 && FUN_1013f880() % BREATH_ROLL_RANGE < BREATH_GREETING_ROLL)
    {
        DAT_1020b430[index].state = BREATH_STEP_RETREAT;
        DAT_1020b430[index].wait = 0;
    }
    if (moving != 0)
    {
        if (DAT_10467920[index] != 0)
        {
            amount = BREATH_NO_GROUND_SCALE;
            move.Copy(move * amount);
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
        if (DAT_1020b430[index].state == BREATH_STEP_RETREAT)
        {
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = BREATH_STEP_STALK;
        }
    }
    return 0;
}
