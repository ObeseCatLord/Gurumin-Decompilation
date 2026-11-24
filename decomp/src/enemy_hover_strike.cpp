// A floating enemy that keeps its distance from the player and calls effects down on him (behaviours 8 and 9 of
// Enemy_UpdateBehaviour 0x100bcb30; behaviour 9 adds a second burst around the enemy itself). It drifts on a velocity
// that it nudges towards a height 75 to 150 units above the ground (or above the player when it finds none) and
// towards or away from the player, closing in beyond 500 units, backing off inside 450 and circling in between. Now
// and then it raises its hands for 63 frames and calls the effects down at the player's position.

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
#include "model_bank.h"
#include "world.h"

#include "action_select_vars.h"
#include "enemy_chase_hop.h"
#include "enemy_hover_strike.h"
#include "shadow_ground.h"

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define HOVSTRIKE_PI 3.1415f
#define HOVSTRIKE_NULL_NODE "@efc_Null"
#define HOVSTRIKE_HAND_NODE "@Bip01 R Finger0"
#define HOVSTRIKE_HAND_SLOT 5             // DAT_1021bdf8[index][5] = the hand node
#define HOVSTRIKE_UNSET_TALK (-1)
#define HOVSTRIKE_UNSET_RANGE (-2)
#define HOVSTRIKE_MAX_LEVEL 10
#define HOVSTRIKE_GROUND_PERIOD 15        // frames between ground probes
#define HOVSTRIKE_PROBE_DEPTH 10000.0f
#define HOVSTRIKE_HEIGHT_ABOVE_PLAYER 75  // hover height above the player when there is no ground
#define HOVSTRIKE_LOW_HEIGHT 75           // it rises to this far above the ground ...
#define HOVSTRIKE_HIGH_HEIGHT 150         // ... and sinks from this far
#define HOVSTRIKE_GROUP_OFFSET 2
#define HOVSTRIKE_GROUP_SPLIT 0x10
#define HOVSTRIKE_GROUP_LAST 0x1f
#define HOVSTRIKE_GROUP_WRAP 15
#define HOVSTRIKE_STEP_START 0
#define HOVSTRIKE_STEP_HOVER 1
#define HOVSTRIKE_STEP_CALL 2
#define HOVSTRIKE_STEP_RECOVER 3
#define HOVSTRIKE_STEP_DRIFT 0xf          // a short drift away (set when greetings are on)
#define HOVSTRIKE_ACCEL 0.3f
#define HOVSTRIKE_DRAG 0.95f
#define HOVSTRIKE_TOP_SPEED 16.0f
#define HOVSTRIKE_NEAR_RANGE 450.0f
#define HOVSTRIKE_FAR_RANGE 500.0f
#define HOVSTRIKE_CIRCLE_SPEED 16.0f
#define HOVSTRIKE_CIRCLE_FRAMES 0x78
#define HOVSTRIKE_CALL_ROLL_RANGE 10000
#define HOVSTRIKE_CALL_ROLL 100
#define HOVSTRIKE_CALL_RANGE 1500.0f
#define HOVSTRIKE_MOTION_HOVER 2
#define HOVSTRIKE_MOTION_CALL 0xe
#define HOVSTRIKE_MOTION_RECOVER 0x10
#define HOVSTRIKE_BLEND_FRAMES 5
#define HOVSTRIKE_RECOVER_FRAME 0x1a3     // the motion frame that ends the recovery
#define HOVSTRIKE_CALL_SOUND_FRAME 0xd
#define HOVSTRIKE_CALL_SOUND 0x1c8
#define HOVSTRIKE_CHARGE_FROM 0xc         // the charge effects run for the frames after this ...
#define HOVSTRIKE_RISE_UNTIL 0x41         // ... the rising ones (every tenth frame, at 3) until this
#define HOVSTRIKE_RISE_PERIOD 10
#define HOVSTRIKE_RISE_PHASE 3
#define HOVSTRIKE_RISE_EFFECT 0x2454
#define HOVSTRIKE_RISE_LIFT 100.0f
#define HOVSTRIKE_BODY_UNTIL 0x4b         // ... and the body ones (every other frame) until this
#define HOVSTRIKE_BODY_EFFECT 0x66d
#define HOVSTRIKE_FLAG_FOLLOW_OWNER 2
#define HOVSTRIKE_STRIKE_FRAME 0x2b
#define HOVSTRIKE_STRIKE_SOUND 0x1c9
#define HOVSTRIKE_STRIKE_EFFECT_A 0x67f
#define HOVSTRIKE_STRIKE_EFFECT_B 0x680
#define HOVSTRIKE_STRIKE_EFFECT_C 0x681
#define HOVSTRIKE_STRIKE_DELAY_B 7
#define HOVSTRIKE_STRIKE_DELAY_C 0xf
#define HOVSTRIKE_STRIKE_SCALE 1.0f
#define HOVSTRIKE_STRIKE_MODE 3
#define HOVSTRIKE_CALL_END 0x3f
#define HOVSTRIKE_DRIFT_FRAMES 0x1e
#define HOVSTRIKE_DRIFT_BACK (-40.0f)
#define HOVSTRIKE_DRIFT_TURN 1.5708f
#define HOVSTRIKE_DRIFT_LIFT_RANGE 10
#define HOVSTRIKE_DRIFT_DRAG 0.975f
#define HOVSTRIKE_FLOOR_LIMIT (-20.0f)    // it never sinks faster than this

#define HOVSTRIKE_MOVE_SET 1              // action: hand the velocity to Character_SetMove
#define HOVSTRIKE_MOVE_FLY 2              // action: move the model by the velocity (when the way is clear)

// 0x100ade30 (Enemy_HoverAndCallStrike). Every frame the model is unlocked from the game's own moves (mode 0, locked)
// and every 15 frames it probes the ground below it (hoverBase = ground height, hoverGrounded = 1; without ground
// and when DAT_10db9274 is set the player's height + 75) and puts its shadow on the ground. Step 0 finds its nodes and
// starts step 1. Step 1 (hover): accelerates its velocity (the LastPosition step) up when it is lower than
// hoverBase + 75 * hoverGrounded and down when higher than hoverBase + 75 * (2 - hoverGrounded), and sideways towards
// the player when he is farther than 500 units, away from him when nearer than 450, else circling (16 units, the
// side changes after 120 frames), with drag 0.95 and a speed limit of 16; it faces the player and, one frame in a
// hundred, when he is within 1500 units and it has seen him (DAT_1030b8f8), starts step 2. Step 2 (call): motion 0xe
// for 63 frames, a sound on frame 13, rising effects from its own position every tenth frame, body effects every
// other frame, and on frame 43 three effects (0x67f, 0x680, 0x681) at the player's position (behaviour 9: two more
// around itself at hoverBase). Step 3 (recover): motion 0x10 until its frame 0x1a3. Step 0xf (drift): when greetings
// are on the enemy slides 40 units away to one side for 30 frames, slowing down. The velocity is handed to
// Character_SetMove (steps 1, 2 and 3) or applied directly (step 0xf). Where it has no ground and DAT_10db9274 is
// clear it heads for the x of its anchor point (DAT_10802bf8 position) in both axes: the original reads position.x
// where it means position.y, and this keeps that. Returns 0.
int Enemy_HoverAndCallStrike(int index, int variant)
{
    Vec4 player;
    Vec4 self;
    Vec4 out;
    int motion = -1;
    int action = 0;
    int restart = 0;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float amount = (player - self).Length2D();
    DAT_10daaae8[index].m_0141 = 0;
    if (DAT_102550e0[index] > HOVSTRIKE_MAX_LEVEL)
        DAT_102550e0[index] = HOVSTRIKE_MAX_LEVEL;
    int group = DAT_102199b8[index];
    if (group == 1)
        DAT_10db6ee8[index].alpha = 1.0f;
    else
    {
        if (group > HOVSTRIKE_GROUP_SPLIT)
            DAT_10db6ee8[index].alpha = (float)((HOVSTRIKE_GROUP_LAST - group) % HOVSTRIKE_GROUP_WRAP) / (float)HOVSTRIKE_GROUP_WRAP;
        else
            DAT_10db6ee8[index].alpha = (float)((group - HOVSTRIKE_GROUP_OFFSET) % HOVSTRIKE_GROUP_WRAP) / (float)HOVSTRIKE_GROUP_WRAP;
    }
    DAT_10255a88[index] = 0;
    DAT_10210b30[index] = 0;
    DAT_10db6ee8[index].mode = 0;
    DAT_10db6ee8[index].locked = 1;
    if ((int)DAT_104c0380 % HOVSTRIKE_GROUND_PERIOD == 0)
    {
        Vec4 ground;
        if (FUN_10024750(index, &ground, HOVSTRIKE_PROBE_DEPTH, 1, 0, 0, 0) != 0)
        {
            DAT_1020b430[index].hoverBase = (int)ground.z;
            DAT_1020b430[index].hoverGrounded = 1;
            DAT_10802bf8[index].position.Copy(ground);
        }
        else if (DAT_10db9274 != 0)
        {
            DAT_1020b430[index].hoverBase = (int)player.z + HOVSTRIKE_HEIGHT_ABOVE_PLAYER;
            DAT_1020b430[index].hoverGrounded = 0;
        }
        if (DAT_102022b8 != 0)
            PlaceShadowOnGround(index);
    }
    if (DAT_10468730[index] != 0)
    {
        DAT_10468730[index] = 0;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(HOVSTRIKE_NULL_NODE, 0);
    }
    if (DAT_102550e0[index] == 0)
    {
        if (DAT_1020b430[index].state == HOVSTRIKE_STEP_START)
        {
            DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(HOVSTRIKE_NULL_NODE, 0);
            DAT_1021bdf8[index][HOVSTRIKE_HAND_SLOT] = DAT_10db6ee8[index].root->Find(HOVSTRIKE_HAND_NODE, 0);
            DAT_1020b430[index].wait = 0;
            if (DAT_1020b430[index].talk == 0)
                DAT_1020b430[index].talk = HOVSTRIKE_UNSET_TALK;
            if (DAT_1020b430[index].range == 0)
                DAT_1020b430[index].range = HOVSTRIKE_UNSET_RANGE;
            DAT_1020b430[index].state = HOVSTRIKE_STEP_HOVER;
            DAT_10db6ee8[index].m_0012 = 2;
            if (DAT_102022b8 != 0)
                PlaceShadowOnGround(index);
            DAT_10802bf8[index].step.Fill(0.0f);
        }
        else if (DAT_1020b430[index].state == HOVSTRIKE_STEP_HOVER)
        {
            DAT_1020b430[index].wait++;
            Vec4 move;
            float low = (float)(DAT_1020b430[index].hoverGrounded * HOVSTRIKE_LOW_HEIGHT + DAT_1020b430[index].hoverBase);
            if (self.z < low)
                DAT_10802bf8[index].step.z = DAT_10802bf8[index].step.z + HOVSTRIKE_ACCEL;
            else
            {
                float high = (float)((2 - DAT_1020b430[index].hoverGrounded) * HOVSTRIKE_LOW_HEIGHT +
                                     DAT_1020b430[index].hoverBase);
                if (self.z > high)
                    DAT_10802bf8[index].step.z = DAT_10802bf8[index].step.z - HOVSTRIKE_ACCEL;
            }
            out.z = DAT_10802bf8[index].step.z;
            if (amount > HOVSTRIKE_FAR_RANGE)
                move.Copy(player - self);
            else if (amount < HOVSTRIKE_NEAR_RANGE)
                move.Copy(self - player);
            else
            {
                move.x = HOVSTRIKE_CIRCLE_SPEED;
                if (DAT_1020b430[index].circleSide == 0)
                    move.x = -HOVSTRIKE_CIRCLE_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                if (DAT_1020b430[index].wait > HOVSTRIKE_CIRCLE_FRAMES)
                {
                    DAT_1020b430[index].circleSide = FUN_1013f880() % 2;
                    DAT_1020b430[index].wait = 0;
                }
            }
            if (DAT_1020b430[index].hoverGrounded == 0)
            {
                if (DAT_10db9274 != 0)
                {
                    move.x = player.x - self.x;
                    move.y = player.y - self.y;
                    DAT_1020b430[index].hoverBase = (int)(player.z + (float)HOVSTRIKE_HEIGHT_ABOVE_PLAYER);
                }
                else
                {
                    move.x = DAT_10802bf8[index].position.x - self.x;
                    move.y = DAT_10802bf8[index].position.x - self.y;
                }
            }
            if (move.x > 0.0f)
                DAT_10802bf8[index].step.x = DAT_10802bf8[index].step.x + HOVSTRIKE_ACCEL;
            else if (move.x < 0.0f)
                DAT_10802bf8[index].step.x = DAT_10802bf8[index].step.x - HOVSTRIKE_ACCEL;
            if (move.y > 0.0f)
                DAT_10802bf8[index].step.y = DAT_10802bf8[index].step.y + HOVSTRIKE_ACCEL;
            else if (move.y < 0.0f)
                DAT_10802bf8[index].step.y = DAT_10802bf8[index].step.y - HOVSTRIKE_ACCEL;
            out.Copy(DAT_10802bf8[index].step * HOVSTRIKE_DRAG);
            if (out.Length() > HOVSTRIKE_TOP_SPEED)
            {
                out.Normalized3();
                out.Copy(out * HOVSTRIKE_TOP_SPEED);
            }
            DAT_10802bf8[index].step.Copy(out);
            float delta = player.y - self.y;
            action = HOVSTRIKE_MOVE_SET;
            float dx = player.x - self.x;
            delta = (float)atan2((double)dx, (double)delta);
            motion = HOVSTRIKE_MOTION_HOVER;
            DAT_10db6ee8[index].targetHeading = delta + HOVSTRIKE_PI;
            if (FUN_1013f880() % HOVSTRIKE_CALL_ROLL_RANGE < HOVSTRIKE_CALL_ROLL)
            {
                if ((player - self).Length() < HOVSTRIKE_CALL_RANGE && DAT_1030b8f8[index] != 0)
                {
                    DAT_1020b430[index].state = HOVSTRIKE_STEP_CALL;
                    DAT_1020b430[index].wait = 0;
                }
            }
        }
        else if (DAT_1020b430[index].state == HOVSTRIKE_STEP_CALL)
        {
            motion = HOVSTRIKE_MOTION_CALL;
            if (++DAT_1020b430[index].wait == 1)
                restart = 1;
            else if (DAT_1020b430[index].wait == HOVSTRIKE_CALL_SOUND_FRAME)
                FUN_10065b10(index, HOVSTRIKE_CALL_SOUND, 0, 0);
            if (DAT_1020b430[index].wait > HOVSTRIKE_CHARGE_FROM && DAT_1020b430[index].wait < HOVSTRIKE_RISE_UNTIL &&
                DAT_1020b430[index].wait % HOVSTRIKE_RISE_PERIOD == HOVSTRIKE_RISE_PHASE)
            {
                EffectOptions options;
                Vec4 offset;
                Matrix4 shot;
                options.flags |= HOVSTRIKE_FLAG_FOLLOW_OWNER;
                offset.Fill(0.0f);
                shot.SetIdentity();
                shot.SetTranslation(self);
                shot.m[14] += HOVSTRIKE_RISE_LIFT;
                FUN_1001d8b0(index, HOVSTRIKE_RISE_EFFECT, offset, -1, -1, 0, 0, 0, &shot, &options);
            }
            if (DAT_1020b430[index].wait > HOVSTRIKE_CHARGE_FROM && DAT_1020b430[index].wait < HOVSTRIKE_BODY_UNTIL &&
                DAT_1020b430[index].wait % 2 != 0)
            {
                EffectOptions options;
                Vec4 offset;
                Matrix4 shot;
                options.flags |= HOVSTRIKE_FLAG_FOLLOW_OWNER;
                offset.Fill(0.0f);
                shot.SetIdentity();
                shot.SetTranslation(self);
                FUN_1001d8b0(index, HOVSTRIKE_BODY_EFFECT, offset, -1, -1, 0, 0, 0, &shot, &options);
            }
            if (DAT_1020b430[index].wait == HOVSTRIKE_STRIKE_FRAME)
            {
                Vec4 offset;
                Matrix4 shot;
                EffectOptions options;
                offset.Fill(0.0f);
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = HOVSTRIKE_STRIKE_SCALE;
                options.m_000e = HOVSTRIKE_STRIKE_MODE;
                shot.SetIdentity();
                shot.SetTranslation(player);
                FUN_10065b10(index, HOVSTRIKE_STRIKE_SOUND, 0, 0);
                FUN_1001d8b0(index, HOVSTRIKE_STRIKE_EFFECT_A, offset, -1, -1, 0, 0, 0, &shot, &options);
                FUN_1001d8b0(index, HOVSTRIKE_STRIKE_EFFECT_B, offset, -1, -1, 0, 0, HOVSTRIKE_STRIKE_DELAY_B, &shot,
                             &options);
                FUN_1001d8b0(index, HOVSTRIKE_STRIKE_EFFECT_C, offset, -1, -1, 0, 0, HOVSTRIKE_STRIKE_DELAY_C, &shot,
                             &options);
                if (variant != 0)
                {
                    FUN_10065b10(0, HOVSTRIKE_STRIKE_SOUND, 0, 0);
                    shot.SetIdentity();
                    shot.SetTranslation(self);
                    shot.m[14] = (float)DAT_1020b430[index].hoverBase;
                    FUN_1001d8b0(index, HOVSTRIKE_STRIKE_EFFECT_A, offset, -1, -1, 0, 0, 0, &shot, &options);
                    FUN_1001d8b0(index, HOVSTRIKE_STRIKE_EFFECT_B, offset, -1, -1, 0, 0, HOVSTRIKE_STRIKE_DELAY_B,
                                 &shot, &options);
                }
            }
            if (DAT_1020b430[index].wait == HOVSTRIKE_CALL_END)
            {
                DAT_1020b430[index].state = HOVSTRIKE_STEP_RECOVER;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == HOVSTRIKE_STEP_RECOVER)
        {
            motion = HOVSTRIKE_MOTION_RECOVER;
            if (++DAT_1020b430[index].wait == 1)
                restart = 1;
            if (DAT_10db6ee8[index].motionFrame == HOVSTRIKE_RECOVER_FRAME)
            {
                DAT_1020b430[index].state = HOVSTRIKE_STEP_HOVER;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == HOVSTRIKE_STEP_DRIFT)
        {
            if (++DAT_1020b430[index].wait == 1)
            {
                DAT_10802bf8[index].step.Fill(0.0f);
                DAT_10802bf8[index].step.y = HOVSTRIKE_DRIFT_BACK;
                float turn = FUN_1013f880() % 2 != 0 ? HOVSTRIKE_DRIFT_TURN : -HOVSTRIKE_DRIFT_TURN;
                DAT_10802bf8[index].step.RotateZ(turn - DAT_10db6ee8[index].targetHeading);
                float lift = (float)(FUN_1013f880() % HOVSTRIKE_DRIFT_LIFT_RANGE);
                DAT_10802bf8[index].step.z = lift + DAT_10802bf8[index].step.z;
            }
            Vec4 move;
            move.Copy(DAT_10802bf8[index].step * HOVSTRIKE_DRIFT_DRAG);
            float limit = (float)DAT_1020b430[index].hoverBase + (float)HOVSTRIKE_HIGH_HEIGHT;
            if (move.z + self.z > limit)
                move.z = limit - self.z;
            if (DAT_1020b430[index].hoverGrounded == 0)
            {
                if (DAT_10db9274 != 0)
                {
                    Vec4 &anchor = DAT_10802bf8[index].position;
                    float playerDistance = (player - self).Length();
                    if ((anchor - self).Length() > playerDistance)
                    {
                        move.x = player.x - self.x;
                        move.y = player.y - self.y;
                    }
                    else
                    {
                        move.x = DAT_10802bf8[index].position.x - self.x;
                        move.y = DAT_10802bf8[index].position.x - self.y;
                    }
                }
                else
                {
                    move.x = DAT_10802bf8[index].position.x - self.x;
                    move.y = DAT_10802bf8[index].position.x - self.y;
                }
                move.Normalized3();
                float speed = DAT_10802bf8[index].step.Length();
                move.Copy(move * speed);
            }
            DAT_10802bf8[index].step.Copy(move);
            out.Copy(move);
            action = HOVSTRIKE_MOVE_FLY;
            if (DAT_1020b430[index].wait == HOVSTRIKE_DRIFT_FRAMES)
            {
                DAT_1020b430[index].state = HOVSTRIKE_STEP_HOVER;
                DAT_1020b430[index].wait = 0;
            }
            DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + HOVSTRIKE_PI;
        }
    }
    if (DAT_1020b430[index].state < HOVSTRIKE_STEP_CALL && DAT_104c00e0 != 0)
    {
        DAT_1020b430[index].state = HOVSTRIKE_STEP_DRIFT;
        DAT_1020b430[index].wait = 0;
    }
    if (action == HOVSTRIKE_MOVE_SET)
    {
        if (DAT_1020b430[index].hoverGrounded == 0)
        {
            float limit = (float)DAT_1020b430[index].hoverBase + (float)HOVSTRIKE_LOW_HEIGHT;
            if (out.z + self.z > limit)
            {
                out.z = limit - self.z;
                if (out.z < HOVSTRIKE_FLOOR_LIMIT)
                    out.z = HOVSTRIKE_FLOOR_LIMIT;
            }
        }
        else
        {
            float limit = (float)DAT_1020b430[index].hoverBase + (float)HOVSTRIKE_HIGH_HEIGHT;
            if (out.z + self.z > limit)
            {
                out.z = limit - self.z;
                if (out.z < HOVSTRIKE_FLOOR_LIMIT)
                    out.z = HOVSTRIKE_FLOOR_LIMIT;
            }
        }
        FUN_100241e0(index, out.x, out.y, out.z);
    }
    else if (action == HOVSTRIKE_MOVE_FLY)
    {
        if (DAT_1020b430[index].hoverGrounded == 0)
        {
            float limit = (float)DAT_1020b430[index].hoverBase + (float)HOVSTRIKE_LOW_HEIGHT;
            if (out.z + self.z > limit)
            {
                out.z = limit - self.z;
                if (out.z < HOVSTRIKE_FLOOR_LIMIT)
                    out.z = HOVSTRIKE_FLOOR_LIMIT;
            }
        }
        else
        {
            float limit = (float)DAT_1020b430[index].hoverBase + (float)HOVSTRIKE_HIGH_HEIGHT;
            if (out.z + self.z > limit)
            {
                out.z = limit - self.z;
                if (out.z < HOVSTRIKE_FLOOR_LIMIT)
                    out.z = HOVSTRIKE_FLOOR_LIMIT;
            }
        }
        if (DAT_10db6ee8[index].IsClearOfMapLimited(&out) != 0 && DAT_10db6ee8[index].IsClearAny(&out) != 0)
        {
            out.Copy(out + self);
            DAT_10db6ee8[index].SetPosition(out.x, out.y, out.z);
        }
    }
    if (motion > -1)
        DAT_10db6ee8[index].PlayMotion(motion, restart, HOVSTRIKE_BLEND_FRAMES, 1.0f);
    return 0;
}
