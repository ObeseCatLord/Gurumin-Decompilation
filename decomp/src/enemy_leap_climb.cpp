// The enemy that strolls about, turns towards the player, leaps at him and climbs up ledges to reach him (0x10068f70,
// a per-kind behaviour of 0x100f9670). See the comment above Enemy_LeapAndClimb for the states.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "effect_manager.h"

#include "anim_model.h"

#include "char_state.h"

#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "last_position.h"
#include "map_vars.h"
#include "model_bank.h"
#include "world.h"

#include "enemy_chase_hop.h"
#include "hop_spit_enemy.h"
#include "leap_climb_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40
Matrix4 RotationX(float angle);   // 0x101b1ad0

#define LEAP_MARKER_NODE "Dummy05"
#define LEAP_PI 3.1415f
#define LEAP_TWO_PI 6.28319f
#define LEAP_HALF_PI 1.5708f
#define LEAP_STATE_STAND 0              // strolls about, turns, picks a leap or a climb
#define LEAP_STATE_TURN 1               // turns on the spot by the angle in DAT_10255160[index][0]
#define LEAP_STATE_CROUCH 2             // winds up for 30 frames, then bursts into dust
#define LEAP_STATE_LEAP 3               // rises to the player's head height
#define LEAP_STATE_FLOAT 4              // hangs in the air for 120 frames, dust for the first 60
#define LEAP_STATE_SCOUT 5              // checks the ground ahead before a climb
#define LEAP_STATE_CLIMB_CROUCH 6
#define LEAP_STATE_CLIMB_JUMP 7         // jumps up and spits a fan at the player
#define LEAP_STATE_CLIMB_RISE 8
#define LEAP_STATE_CLIMB_GLIDE 9        // glides back to the point saved in state 5
#define LEAP_STATE_LAND 10
#define LEAP_STATE_DOWN 11              // lies on the ground, kicking about, until it may get up
#define LEAP_PIT_DEPTH 120.0f           // below the pit floor minus this the character is held up
#define LEAP_MODEL_LIFT 110.0f          // height of the model's centre above its origin
#define LEAP_DUST_EFFECT 0xab
#define LEAP_SPIT_EFFECT 0x258
#define LEAP_SPIT_EFFECT_VARIANT 0x68c
#define LEAP_SPIT_SOUND 0x2ad
#define LEAP_SPIT_SOUND_VARIANT 0x2bc
#define LEAP_LAND_SOUND 0x2ac
#define LEAP_LAND_SOUND_VARIANT 0x2bb
#define LEAP_BURST_EFFECT 0x9b
#define LEAP_BURST_SOUND 0x2ab
#define LEAP_BLEND 5
#define LEAP_MOTION_SPEED 1.0f
#define LEAP_GROUND_PERIOD 30
#define LEAP_SHADOW_LIFT 10.0f
#define LEAP_SHADOW_DEPTH 1000.0f
#define LEAP_NO_GROUND (-3500.0f)
#define LEAP_SINK_REACH 220.0f
#define LEAP_GROUND_DEPTH 10000.0f
#define LEAP_PROBE_REACH 1000.0f
#define LEAP_PROBE_AHEAD 150.0f
#define LEAP_PROBE_HEIGHT 110.0f
#define LEAP_NO_WALL 100000.0f
#define LEAP_SLIDE_SPEED 4.0f
#define LEAP_RISE_STEP 20.0f
#define LEAP_NOTICE_ANGLE 0.43633f
#define LEAP_NOTICE_ANGLE_CLOSE 0.2618f
#define LEAP_NOTICE_RANGE 1200.0f
#define LEAP_VARIANT_NOTICE 50
#define LEAP_NEAR_RANGE 500.0f
#define LEAP_ROLL_RANGE 750000
#define LEAP_REPEAT_RANGE 100000
#define LEAP_TURN_ROLL 0xa3a
#define LEAP_FULL_TURN_ROLL 0x3d5c
#define LEAP_TURN_UNIT 0.0001f
#define LEAP_VEER_FRAMES 10
#define LEAP_NEAR_WALL 200.0f
#define LEAP_TURN_BASE 0.01745f
#define LEAP_TURN_FAR 300.0f
#define LEAP_TURN_GAIN 0.2618f
#define LEAP_CROUCH_FRAMES 30
#define LEAP_CROUCH_DAMP 0.95f
#define LEAP_BURST_COUNT 10
#define LEAP_BURST_SPREAD 1000
#define LEAP_BURST_UNIT 0.01f
#define LEAP_BURST_FRAME_RANGE 15
#define LEAP_CHASE_RANGE 900.0f
#define LEAP_TURN_STEP 0.2618f
#define LEAP_HEAD_SPEED 4.0f
#define LEAP_HEAD_DIVISOR 10.0f
#define LEAP_LEAP_MAX_RISE 6.0f
#define LEAP_LEAP_MAX_DROP (-2.0f)
#define LEAP_LEAP_LEVEL 2.0f
#define LEAP_LEAP_SHRINK 0.116678f
#define LEAP_LEAP_SPEED 7.0f
#define LEAP_LEAP_FRAMES 30
#define LEAP_FLOAT_FRAMES 0x78
#define LEAP_FLOAT_DUST_FRAMES 0x3c
#define LEAP_CLIMB_HEIGHT 130.0f
#define LEAP_CLIMB_RISE_SPEED 12.0f
#define LEAP_SCOUT_FIRST (-1)
#define LEAP_SCOUT_LAST 2
#define LEAP_SCOUT_SPACING (-30.0f)
#define LEAP_SCOUT_LIFT 40.0f
#define LEAP_SCOUT_REACH 2.0f
#define LEAP_SCOUT_REACH_MID 3.0f
#define LEAP_SCOUT_REACH_FAR 4.0f
#define LEAP_REVERSE_ANGLE 3.14159f
#define LEAP_CROUCH_RISE_FROM 4
#define LEAP_CLIMB_CROUCH_FRAMES 7
#define LEAP_JUMP_LAUNCH 2
#define LEAP_JUMP_SWEEP_FRAME 7
#define LEAP_JUMP_END_FRAME 12
#define LEAP_JUMP_BOOST 3.0f
#define LEAP_JUMP_LIFT 2.5f
#define LEAP_JUMP_LIFT_BASE 30
#define LEAP_SWEEP_SPOT_LIFT 75.0f
#define LEAP_SWEEP_AIM_HEIGHT 240.0f
#define LEAP_SWEEP_ANGLE 0.2618f
#define LEAP_VARIANT_EFFECT_SIZE 3.0f
#define LEAP_RISE_LAUNCH 30.0f
#define LEAP_RISE_FALL_BOOST (-10.0f)
#define LEAP_GLIDE_RISE_BASE 27.5f
#define LEAP_GLIDE_RISE_STEP 3.5f
#define LEAP_GLIDE_MIN 0.5f
#define LEAP_GLIDE_LOW_BASE 7.5f
#define LEAP_GLIDE_LOW_STEP 2.5f
#define LEAP_GLIDE_DISTANCE 780.0f
#define LEAP_GLIDE_MOTION_SWITCH 12
#define LEAP_LAND_DAMP 0.9f
#define LEAP_LAND_FRAMES 7
#define LEAP_DOWN_KICK_RANGE 0x18
#define LEAP_DOWN_TURN_RANGE 0x168
#define LEAP_DOWN_SPEED_RANGE 0xf
#define LEAP_DOWN_SPEED_BASE 15.0f
#define LEAP_DOWN_FACE_ROLL 4
#define LEAP_DEGREES_PER_PI 180.0f
#define LEAP_BOB_PERCENT 100.0f
#define LEAP_BOB_ROLL_LOW 4000
#define LEAP_BOB_ROLL_MID 1000
#define LEAP_BOB_ROLL_HIGH 2000
#define LEAP_BOB_CHANCE_MID 400
#define LEAP_DUST_PIT_LIFT 10.0f
#define LEAP_PIT_HOVER_SPEED 10.0f
#define LEAP_HOVER_RISING 0
#define LEAP_HOVER_HOVERING 1
#define LEAP_HOVER_SINKING 2
#define LEAP_HOVER_PIT 3
#define LEAP_HOVER_HELD 4

// 0x10068f70 (Enemy_LeapAndClimb). Every frame it first puts a hit character back on the point saved in state 5,
// keeps the shadow on the ground, notes whether it is over the pit area and looks up the floor below it (the height
// is kept in floorHeight), then measures the player: the distance, the heading error and a probe of the way ahead.
// The state moves it: 0 strolls about and turns by the angle of state 1 when a roll says so, and starts a leap
// (state 2 crouches for 30 frames and bursts into dust, state 3 rises to the player's head height, state 4 hangs in
// the air for 120 frames) when it faces the player, or a climb (state 5 looks at three points ahead, state 6
// crouches, state 7 jumps and, on its 7th frame, spits a fan of effect 600 (three with DAT_10c635f8 == 2) unless a
// wall is in the way, state 8 rises, state 9 glides to the point saved in state 5, state 10 lands) when the player
// is above the pit floor. State 11 is the fall after a hit: it kicks about until it may get up. States 0 to 4 end in
// the floor-bob rule (the vertical move by its hover pattern). Every state ends in the move and the position
// history. The variant 1 reacts to hits and uses the stronger spit effect. Returns 1 while the slot is flagged gone,
// 0 otherwise.
int Enemy_LeapAndClimb(int index, int variant)
{
    Vec4 origin;
    EffectOptions options;
    Matrix4 matrix;
    int overPit = 0;
    float amount;
    DAT_10db6ee8[index].locked = 1;
    if (DAT_10210d30[index] != 0)
        return 1;
    if (DAT_10210b30[index] != 0)
    {
        if (variant != 0)
            DAT_1020b430[index].hitReacted = 1;
        if (DAT_1020b430[index].state < LEAP_STATE_CLIMB_CROUCH)
        {
            DAT_10210b30[index] = 0;
            DAT_1020b430[index].state = LEAP_STATE_STAND;
        }
        else if (DAT_1020b430[index].state != LEAP_STATE_DOWN)
        {
            DAT_10210b30[index] = 0;
            DAT_1020b430[index].state = LEAP_STATE_STAND;
            DAT_1020b430[index].turningAround = 1;
            Vec4 target;
            Vec4 current;
            Vec4 hit;
            Vec4 shift;
            current.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            target.Copy(DAT_10868838[index][0].GetTranslation());
            current.z = target.z;
            amount = 0.0f;
            shift.Fill(amount);
            shift.z = target.z - current.z;
            DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
            if ((DAT_10255f08 != 0 && DAT_10255f08->RayCast(0, 4, 0x4000000, &target, &current, &hit, 0) != 0) ||
                DAT_10db6ee8[index].IsClearOfMapLimited(&shift) == 0)
                DAT_10db6ee8[index].SetPosition(target.x, target.y, current.z);
            DAT_11b002c0 = 0;
        }
    }
    if (DAT_104c0380 % LEAP_GROUND_PERIOD == index % LEAP_GROUND_PERIOD)
    {
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        DAT_10db6ee8[index].m_0012 = 2;
        if (DAT_102022b8 != 0)
        {
            Vec4 from;
            Vec4 ground;
            Vec4 top = DAT_10db6ee8[index].matrix.GetTranslation();
            from.Copy(top);
            top.z += LEAP_SHADOW_LIFT;
            Vec4 direction = *DAT_10804bf8.LightDirection(&DAT_10db6ee8[index], &DAT_118e69e8, &DAT_118e6cb8);
            direction.Normalized3();
            float distance = LEAP_SHADOW_DEPTH;
            from.Copy(from - direction * distance);
            if (DAT_10255f08 != 0 && DAT_10255f08->CastSegment2(0, 0x4000000, &from, &top, &ground, 0) != 0)
            {
                ground.x = from.x;
                ground.y = from.y;
            }
            else
            {
                ground.x = from.x;
                ground.y = from.y;
                ground.z = LEAP_NO_GROUND;
            }
            DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
        }
        DAT_11b002c0 = 0;
    }
    int awake = DAT_104483e8[index];
    DAT_10218fc0[index] = 0;
    DAT_10db6ee8[index].mode = 2;
    if (awake == 0)
        return 0;
    int y = (int)DAT_10db6ee8[index].matrix._42;
    int x = (int)DAT_10db6ee8[index].matrix._41;
    if (x >= (int)DAT_104d29e8.x && y >= (int)DAT_104d29e8.y && x <= (int)DAT_104d29f8.x &&
        y <= (int)DAT_104d29f8.y && DAT_104bfe28 != 0)
        overPit = 1;
    amount = (float)DAT_1020b430[index].floorHeight;
    if (DAT_1020b430[index].floorKnown == 0 || amount > DAT_104bfe24)
    {
        Vec4 ground;
        Vec4 offset;
        amount = 0.0f;
        offset.Fill(amount);
        offset.z = LEAP_SINK_REACH;
        DAT_1021bdf8[index][1] = DAT_10db6ee8[index].root->Find(LEAP_MARKER_NODE, 0);
        if (FUN_10024750(index, &ground, LEAP_GROUND_DEPTH, 0, &offset, 0, 0) != 0)
        {
            DAT_1020b430[index].floorHeight = (int)ground.z;
            if (DAT_1020b430[index].floorKnown != 1)
                DAT_10db6ee8[index].mode = 1;
            DAT_1020b430[index].floorKnown = 1;
        }
        else if (DAT_1020b430[index].floorKnown != 1)
        {
            DAT_10db6ee8[index].mode = 0;
            return 0;
        }
    }
    if (DAT_10db6ee8[index].m_228c.x != 0.0f || DAT_10db6ee8[index].m_228c.y != 0.0f ||
        DAT_10db6ee8[index].m_228c.z != 0.0f)
    {
        DAT_10db6ee8[index].m_228c.x = 0.0f;
        DAT_10db6ee8[index].m_228c.y = 0.0f;
        DAT_10db6ee8[index].m_228c.z = 0.0f;
    }
    Vec4 player;
    Vec4 self;
    Vec4 diff;
    Vec4 move;
    Vec4 sight;
    Vec4 probe;
    player.Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    diff.Copy(player - self);
    float flatRange = (self - player).Length2D();
    float range = (self - player).Length();
    float turnError = DAT_10db6ee8[index].heading - (float)atan2(diff.x, diff.y) - LEAP_PI;
    turnError = (float)fabs(turnError);
    acos((double)(flatRange / range));
    amount = DAT_10db6ee8[index].matrix._43 - (float)DAT_1020b430[index].floorHeight;
    range = 0.0f;
    probe.Fill(range);
    probe.y = LEAP_PROBE_AHEAD;
    probe.RotateZ(-DAT_10db6ee8[index].heading);
    probe.z = LEAP_PROBE_HEIGHT;
    if (FUN_100242c0(index, &sight, LEAP_PROBE_REACH, 0, 0, &probe) != 0)
        range = (sight - self).Length2D();
    else
        range = LEAP_NO_WALL;
    float zero = 0.0f;
    move.Fill(zero);
    move.y = -LEAP_SLIDE_SPEED;
    move.RotateZ(-DAT_10db6ee8[index].heading);
    if (DAT_1020b430[index].state < LEAP_STATE_CLIMB_CROUCH)
    {
        DAT_10db6ee8[index].matrix._41 = move.x * DAT_10db6ee8[index].speedScale + DAT_10db6ee8[index].matrix._41;
        DAT_10db6ee8[index].matrix._42 = move.y * DAT_10db6ee8[index].speedScale + DAT_10db6ee8[index].matrix._42;
    }
    else if (DAT_1020b430[index].state < LEAP_STATE_CLIMB_RISE)
        DAT_10db6ee8[index].matrix._43 = DAT_10db6ee8[index].matrix._43 + LEAP_RISE_STEP;
    else if (DAT_1020b430[index].state < LEAP_STATE_DOWN)
    {
        zero = (float)DAT_1020b430[index].floorHeight;
        DAT_10255160[index][1] = DAT_10db6ee8[index].matrix._43;
        DAT_10db6ee8[index].matrix._43 = zero + LEAP_MODEL_LIFT;
    }
    if (DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0)
        range = 0.0f;
    if (DAT_1020b430[index].state < LEAP_STATE_CLIMB_CROUCH)
    {
        DAT_10db6ee8[index].matrix._41 = DAT_10db6ee8[index].matrix._41 - move.x * DAT_10db6ee8[index].speedScale;
        DAT_10db6ee8[index].matrix._42 = DAT_10db6ee8[index].matrix._42 - move.y * DAT_10db6ee8[index].speedScale;
    }
    else if (DAT_1020b430[index].state < LEAP_STATE_CLIMB_RISE)
        DAT_10db6ee8[index].matrix._43 = DAT_10db6ee8[index].matrix._43 - LEAP_RISE_STEP;
    else if (DAT_1020b430[index].state < LEAP_STATE_DOWN)
        DAT_10db6ee8[index].matrix._43 = DAT_10255160[index][1];
    int state = DAT_1020b430[index].state;
    if (state == LEAP_STATE_STAND)
    {
        int veer = DAT_1020b430[index].veerFrames;
        DAT_1020b430[index].leapFrames++;
        if (veer < 0)
            DAT_1020b430[index].veerFrames = veer + 1;
        else if (veer > 0)
            DAT_1020b430[index].veerFrames = veer - 1;
        float gap = LEAP_NEAR_RANGE - range;
        float gapSize = (float)fabs(gap);
        float roll = (float)(FUN_1013f880() % LEAP_ROLL_RANGE);
        if (gapSize * gap >= roll)
        {
            float *turnAngle = &DAT_10255160[index][0];
            *turnAngle = (float)(FUN_1013f880() % LEAP_FULL_TURN_ROLL) * LEAP_TURN_UNIT + LEAP_HALF_PI;
            if (DAT_1020b430[index].veerFrames > 0 && range < LEAP_NEAR_WALL)
                DAT_1020b430[index].veerFrames = LEAP_VEER_FRAMES;
            else if (FUN_1013f880() % 2 == 0 || DAT_1020b430[index].veerFrames < 0)
            {
                DAT_1020b430[index].veerFrames = -LEAP_VEER_FRAMES;
                *turnAngle = *turnAngle * -1.0f;
            }
            else
                DAT_1020b430[index].veerFrames = LEAP_VEER_FRAMES;
            DAT_1020b430[index].state = LEAP_STATE_TURN;
            DAT_1020b430[index].leapFrames = 0;
        }
        else if (FUN_1013f880() % LEAP_REPEAT_RANGE <= DAT_1020b430[index].leapFrames)
        {
            float *turnAngle = &DAT_10255160[index][0];
            *turnAngle = (float)(FUN_1013f880() % LEAP_TURN_ROLL) * LEAP_TURN_UNIT;
            if (FUN_1013f880() % 2 == 0)
            {
                DAT_1020b430[index].veerFrames = -LEAP_VEER_FRAMES;
                *turnAngle = *turnAngle * -1.0f;
            }
            DAT_1020b430[index].state = LEAP_STATE_TURN;
            DAT_1020b430[index].leapFrames = 0;
        }
        DAT_10db6ee8[index].PlayMotion(0, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
        if (DAT_104bfe24 - LEAP_PIT_DEPTH < DAT_10db6ee8[index].matrix._43)
            DAT_10db6ee8[index].mode = 1;
        if (((turnError < LEAP_NOTICE_ANGLE && DAT_10c66710 == 0) || turnError < LEAP_NOTICE_ANGLE_CLOSE ||
             (variant != 0 && FUN_1013f880() % 1000 < LEAP_VARIANT_NOTICE)) &&
            flatRange < LEAP_NOTICE_RANGE)
        {
            DAT_104483e8[index] = -1;
            if (!(DAT_10db6ee8[PLAYER_SLOT].matrix._43 > DAT_104bfe24))
            {
                if (flatRange < range)
                {
                    if (variant == 0)
                    {
                        DAT_1020b430[index].state = LEAP_STATE_CROUCH;
                        DAT_1020b430[index].leapFrames = 0;
                        DAT_1020b430[index].veerFrames = 0;
                    }
                    else if (DAT_1020b430[index].hitReacted != 0)
                    {
                        DAT_1020b430[index].state = LEAP_STATE_SCOUT;
                        DAT_1020b430[index].leapFrames = 0;
                        DAT_1020b430[index].veerFrames = 0;
                    }
                }
            }
            else if (variant == 0 || DAT_1020b430[index].hitReacted != 0)
            {
                DAT_1020b430[index].state = LEAP_STATE_SCOUT;
                DAT_1020b430[index].leapFrames = 0;
                DAT_1020b430[index].veerFrames = 0;
            }
        }
    }
    else if (state == LEAP_STATE_TURN)
    {
        float step = LEAP_TURN_BASE;
        if (range < LEAP_TURN_FAR)
        {
            if (range < 1.0f)
                range = 1.0f;
            step = LEAP_TURN_GAIN / range + LEAP_TURN_BASE;
        }
        float *turnAngle = &DAT_10255160[index][0];
        if (*turnAngle > 0.0f)
            step = step * -1.0f;
        float remaining = floorf(*turnAngle);
        if (floorf(step) <= remaining)
        {
            DAT_10db6ee8[index].targetHeading = step + DAT_10db6ee8[index].targetHeading;
            *turnAngle = step + *turnAngle;
        }
        else
        {
            step = *turnAngle;
            *turnAngle = 0.0f;
            step = step * -1.0f;
            DAT_1020b430[index].state = LEAP_STATE_STAND;
            DAT_1020b430[index].turningAround = 0;
            DAT_10db6ee8[index].targetHeading = step + DAT_10db6ee8[index].targetHeading;
        }
        DAT_10db6ee8[index].PlayMotion(0, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
        step = DAT_104bfe24 - LEAP_PIT_DEPTH;
        if (step < DAT_10db6ee8[index].matrix._43)
            DAT_10db6ee8[index].matrix._43 = step;
        if ((turnError < LEAP_NOTICE_ANGLE || variant != 0) && flatRange < LEAP_NOTICE_RANGE &&
            DAT_1030b8f8[index] != 0 && DAT_1020b430[index].turningAround == 0)
        {
            DAT_104483e8[index] = -1;
            if (!(DAT_10db6ee8[PLAYER_SLOT].matrix._43 > DAT_104bfe24))
            {
                if (flatRange < range && flatRange < LEAP_CHASE_RANGE)
                {
                    if (variant == 0)
                    {
                        DAT_1020b430[index].state = LEAP_STATE_CROUCH;
                        DAT_1020b430[index].leapFrames = 0;
                    }
                    else if (DAT_1020b430[index].hitReacted != 0)
                    {
                        DAT_1020b430[index].state = LEAP_STATE_SCOUT;
                        DAT_1020b430[index].leapFrames = 0;
                    }
                }
            }
            else if (variant == 0 || DAT_1020b430[index].hitReacted != 0)
            {
                DAT_1020b430[index].state = LEAP_STATE_SCOUT;
                DAT_1020b430[index].leapFrames = 0;
            }
        }
    }
    else if (state == LEAP_STATE_CROUCH)
    {
        DAT_104483e8[index] = -1;
        float bearing = atan2f(diff.x, diff.y);
        int frames = DAT_1020b430[index].leapFrames;
        DAT_10db6ee8[index].targetHeading = bearing + LEAP_PI;
        move.x = DAT_10802bf8[index].step.x * LEAP_CROUCH_DAMP;
        DAT_10218fc0[index] = 1;
        move.z = 0.0f;
        DAT_1020b430[index].leapFrames = frames + 1;
        move.y = DAT_10802bf8[index].step.y * LEAP_CROUCH_DAMP;
        if (frames + 1 > LEAP_CROUCH_FRAMES)
        {
            DAT_1020b430[index].state = LEAP_STATE_LEAP;
            DAT_1020b430[index].leapFrames = 0;
            Vec4 burstPosition;
            EffectOptions burstOptions;
            Matrix4 burst;
            float burstZero = 0.0f;
            burstPosition.Fill(burstZero);
            burst.SetIdentity();
            burst = DAT_10db6ee8[index].matrix;
            burst._43 = burst._43 + LEAP_MODEL_LIFT;
            burstOptions.m_0004 = 0;
            burstOptions.m_0000 = 0;
            burstOptions.flags = 1;
            burstOptions.list = 1;
            float baseX = burst._41;
            float baseY = burst._42;
            float baseZ = burst._43;
            for (int burstIndex = LEAP_BURST_COUNT; burstIndex != 0; burstIndex--)
            {
                burst._41 = (float)(FUN_1013f880() % LEAP_BURST_SPREAD - LEAP_BURST_SPREAD / 2) * LEAP_BURST_UNIT + baseX;
                burst._42 = (float)(FUN_1013f880() % LEAP_BURST_SPREAD - LEAP_BURST_SPREAD / 2) * LEAP_BURST_UNIT + baseY;
                burst._43 = (float)(FUN_1013f880() % LEAP_BURST_SPREAD - LEAP_BURST_SPREAD / 2) * LEAP_BURST_UNIT + baseZ;
                FUN_1001d8b0(index, LEAP_BURST_EFFECT, burstPosition, -1, -1, 0, 0,
                             FUN_1013f880() % LEAP_BURST_FRAME_RANGE, &burst, &burstOptions);
            }
            burst._41 = baseX;
            burst._42 = baseY;
            burst._43 = baseZ;
            FUN_1001d8b0(index, 5, burstPosition, -1, -1, 0, 0, 0, &burst, &burstOptions);
            FUN_10065b10(index, LEAP_BURST_SOUND, 0, 0);
        }
        float held = DAT_104bfe24 - LEAP_PIT_DEPTH;
        if (held < DAT_10db6ee8[index].matrix._43)
        {
            DAT_10db6ee8[index].matrix._43 = held;
            DAT_10db6ee8[index].move.z = 0.0f;
            move.z = 0.0f;
            DAT_1020b430[index].hoverPattern = LEAP_HOVER_RISING;
        }
    }
    else if (state == LEAP_STATE_LEAP)
    {
        DAT_104483e8[index] = -1;
        DAT_10218fc0[index] = 1;
        float bearing = atan2f(diff.x, diff.y) + LEAP_PI;
        float wrapped;
        if (bearing <= DAT_10db6ee8[index].targetHeading)
            wrapped = bearing + LEAP_TWO_PI;
        else
            wrapped = bearing - LEAP_TWO_PI;
        float toBearing = floorf(DAT_10db6ee8[index].targetHeading - bearing);
        if (toBearing <= floorf(DAT_10db6ee8[index].targetHeading - wrapped))
        {
            if (bearing < DAT_10db6ee8[index].targetHeading)
                DAT_10db6ee8[index].targetHeading =
                    DAT_10db6ee8[index].targetHeading - DAT_10db6ee8[index].speedScale * LEAP_TURN_STEP;
            else if (DAT_10db6ee8[index].targetHeading < bearing)
                DAT_10db6ee8[index].targetHeading =
                    DAT_10db6ee8[index].speedScale * LEAP_TURN_STEP + DAT_10db6ee8[index].targetHeading;
        }
        else if (0.0f < wrapped)
            DAT_10db6ee8[index].targetHeading =
                DAT_10db6ee8[index].speedScale * LEAP_TURN_STEP + DAT_10db6ee8[index].targetHeading;
        else
            DAT_10db6ee8[index].targetHeading =
                DAT_10db6ee8[index].targetHeading - DAT_10db6ee8[index].speedScale * LEAP_TURN_STEP;
        float lowered = DAT_10db6ee8[index].matrix._43 + LEAP_MODEL_LIFT;
        if (DAT_10db91dc->worldMatrix._43 <= lowered)
        {
            if (lowered <= DAT_10db91dc->worldMatrix._43)
                move.z = LEAP_LEAP_LEVEL;
            else
            {
                move.z = (DAT_10db91dc->worldMatrix._43 - lowered) / LEAP_HEAD_DIVISOR -
                         DAT_10db6ee8[index].speedScale * LEAP_HEAD_SPEED;
                if (LEAP_LEAP_MAX_DROP < move.z)
                    move.z = LEAP_LEAP_MAX_DROP;
            }
        }
        else
        {
            move.z = DAT_10db6ee8[index].speedScale * LEAP_HEAD_SPEED +
                     (DAT_10db91dc->worldMatrix._43 - lowered) / LEAP_HEAD_DIVISOR;
            if (move.z < LEAP_LEAP_MAX_RISE)
                move.z = LEAP_LEAP_MAX_RISE;
        }
        int frames = DAT_1020b430[index].leapFrames;
        DAT_1020b430[index].leapFrames = frames + 1;
        float shrink = LEAP_LEAP_SPEED - (float)frames * LEAP_LEAP_SHRINK;
        move.x = shrink * move.x;
        move.y = shrink * move.y;
        if (DAT_104bfe24 < DAT_10db6ee8[PLAYER_SLOT].matrix._43 || range < flatRange ||
            (frames + 1 > LEAP_LEAP_FRAMES && DAT_1030b8f8[index] != 0))
        {
            DAT_1020b430[index].state = LEAP_STATE_FLOAT;
            DAT_1020b430[index].leapFrames = 0;
        }
        float held = DAT_104bfe24 - LEAP_PIT_DEPTH;
        if (held < DAT_10db6ee8[index].matrix._43 && DAT_1030b8f8[index] != 0)
        {
            DAT_10db6ee8[index].matrix._43 = held;
            DAT_10db6ee8[index].move.z = 0.0f;
            move.z = 0.0f;
            DAT_1020b430[index].hoverPattern = LEAP_HOVER_RISING;
        }
        float leapZero = 0.0f;
        origin.Fill(leapZero);
        matrix.SetIdentity();
        matrix = DAT_10db6ee8[index].matrix;
        matrix._43 = matrix._43 + LEAP_MODEL_LIFT;
        options.m_0004 = 0;
        options.m_0000 = 0;
        options.flags = 1;
        options.list = 1;
        if (DAT_1020b430[index].leapFrames % 2 != 0)
            FUN_1001d8b0(index, LEAP_BURST_EFFECT, origin, -1, -1, 0, 0, FUN_1013f880() % LEAP_BURST_FRAME_RANGE,
                         &matrix, &options);
        options.flags = 0;
        options.list = 0;
        options.m_0000 = 0;
        FUN_1001d8b0(index, 0x658, origin, -1, -1, &DAT_1021bdf8[index][1]->worldMatrix, 0, 0, 0, 0);
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_000e = 5;
        FUN_1001d8b0(index, 0x657, origin, -1, -1, &DAT_1021bdf8[index][1]->worldMatrix, 0, 0, 0, &options);
        DAT_1021f6c0[index] = 1;
        DAT_10db6ee8[index].PlayMotion(1, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
    }
    else if (state == LEAP_STATE_FLOAT)
    {
        int frames = DAT_1020b430[index].leapFrames;
        move.x = DAT_10802bf8[index].step.x * LEAP_CROUCH_DAMP;
        DAT_104483e8[index] = -1;
        DAT_1020b430[index].leapFrames = frames + 1;
        move.z = 0.0f;
        move.y = DAT_10802bf8[index].step.y * LEAP_CROUCH_DAMP;
        float held = DAT_104bfe24 - LEAP_PIT_DEPTH;
        if (held < DAT_10db6ee8[index].matrix._43)
        {
            DAT_10db6ee8[index].matrix._43 = held;
            DAT_10db6ee8[index].move.z = 0.0f;
            DAT_1020b430[index].hoverPattern = LEAP_HOVER_RISING;
        }
        if (frames + 1 == LEAP_FLOAT_FRAMES)
        {
            DAT_1020b430[index].leapFrames = 0;
            DAT_1020b430[index].state = LEAP_STATE_STAND;
        }
        if (DAT_1020b430[index].leapFrames < LEAP_FLOAT_DUST_FRAMES)
        {
            DAT_10218fc0[index] = 1;
            DAT_10db6ee8[index].PlayMotion(1, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
            float leapZero = 0.0f;
            origin.Fill(leapZero);
            matrix.SetIdentity();
            matrix = DAT_10db6ee8[index].matrix;
            matrix._43 = matrix._43 + LEAP_MODEL_LIFT;
            options.m_0004 = 0;
            options.m_0000 = 0;
            options.flags = 1;
            options.list = 1;
            if (DAT_1020b430[index].leapFrames % 2 != 0)
                FUN_1001d8b0(index, LEAP_BURST_EFFECT, origin, -1, -1, 0, 0, FUN_1013f880() % LEAP_BURST_FRAME_RANGE,
                             &matrix, &options);
            options.flags = 0;
            options.list = 0;
            options.m_0000 = 0;
            FUN_1001d8b0(index, 0x658, origin, -1, -1, &DAT_1021bdf8[index][1]->worldMatrix, 0, 0, 0, 0);
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_000e = 5;
            FUN_1001d8b0(index, 0x657, origin, -1, -1, &DAT_1021bdf8[index][1]->worldMatrix, 0, 0, 0, &options);
            DAT_1021f6c0[index] = 1;
        }
        else
            DAT_10db6ee8[index].PlayMotion(2, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
    }
    else if (state == LEAP_STATE_SCOUT)
    {
        move.z = DAT_10db6ee8[index].speedScale * LEAP_CLIMB_RISE_SPEED;
        DAT_104483e8[index] = -1;
        DAT_10218fc0[index] = 1;
        DAT_10db6ee8[index].targetHeading = atan2f(diff.x, diff.y) + LEAP_PI;
        if (DAT_104bfe24 - LEAP_CLIMB_HEIGHT < DAT_10db6ee8[index].matrix._43)
        {
            DAT_10868838[index][0] = DAT_10db6ee8[index].matrix;
            DAT_1020b430[index].state = LEAP_STATE_CLIMB_CROUCH;
            DAT_1020b430[index].leapFrames = 0;
            Vec4 ahead;
            Vec4 ahead2;
            Vec4 ahead3;
            Vec4 ahead4;
            for (int i = LEAP_SCOUT_FIRST; i < LEAP_SCOUT_LAST; i++)
            {
                float scoutZero = 0.0f;
                ahead.Fill(scoutZero);
                ahead.y = (float)i * LEAP_SCOUT_SPACING;
                ahead.RotateZ(-DAT_10db6ee8[index].heading);
                ahead.z = ahead.z + LEAP_SCOUT_LIFT;
                ahead2.Copy(ahead);
                ahead3.Copy(ahead);
                ahead4.Copy(ahead);
                ahead2.Copy(ahead2 * LEAP_SCOUT_REACH);
                ahead3.Copy(ahead3 * LEAP_SCOUT_REACH_MID);
                ahead4.Copy(ahead4 * LEAP_SCOUT_REACH_FAR);
                if (DAT_10db6ee8[index].IsClearOfMapLimited(&ahead) == 0 ||
                    DAT_10db6ee8[index].IsClearOfMapLimited(&ahead2) == 0 ||
                    DAT_10db6ee8[index].IsClearOfMapLimited(&ahead3) == 0 ||
                    DAT_10db6ee8[index].IsClearOfMapLimited(&ahead4) == 0 ||
                    DAT_10db6ee8[index].IsClearAny(&ahead) == 0 || DAT_10db6ee8[index].IsClearAny(&ahead2) == 0 ||
                    DAT_10db6ee8[index].IsClearAny(&ahead3) == 0 || DAT_10db6ee8[index].IsClearAny(&ahead4) == 0)
                {
                    range = 0.0f;
                    break;
                }
            }
        }
        if (DAT_10db6ee8[index].IsClearAny(&move) == 0 || range == 0.0f)
        {
            DAT_10255160[index][0] = LEAP_REVERSE_ANGLE;
            if (FUN_1013f880() % 2 == 0)
            {
                DAT_1020b430[index].veerFrames = -LEAP_VEER_FRAMES;
                DAT_10255160[index][0] = DAT_10255160[index][0] * -1.0f;
            }
            DAT_1020b430[index].state = LEAP_STATE_TURN;
            DAT_1020b430[index].leapFrames = 0;
            DAT_1020b430[index].turningAround = 1;
        }
    }
    else if (state == LEAP_STATE_CLIMB_CROUCH)
    {
        int frames = DAT_1020b430[index].leapFrames;
        DAT_104483e8[index] = -1;
        DAT_10218fc0[index] = 1;
        if (frames < LEAP_CROUCH_RISE_FROM)
            move.z = 0.0f;
        else
            move.z = (float)(frames - LEAP_CROUCH_RISE_FROM) * LEAP_SLIDE_SPEED;
        if (frames == LEAP_CLIMB_CROUCH_FRAMES)
        {
            DAT_1020b430[index].state = LEAP_STATE_CLIMB_JUMP;
            DAT_1020b430[index].leapFrames = 0;
        }
        DAT_1020b430[index].leapFrames = DAT_1020b430[index].leapFrames + 1;
        DAT_10db6ee8[index].PlayMotion(3, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
        if (DAT_10db6ee8[index].IsClearAny(&move) == 0 || range == 0.0f)
        {
            DAT_1020b430[index].state = LEAP_STATE_DOWN;
            DAT_1020b430[index].leapFrames = 0;
            DAT_10db6ee8[index].PlayMotion(0xc, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
            float stopZero = 0.0f;
            DAT_10802bf8[index].step.Fill(stopZero);
            stopZero = 0.0f;
            move.Fill(stopZero);
        }
        if (DAT_1020b430[index].dustDone == 0 && DAT_104bfe24 < DAT_10db6ee8[index].matrix._43 + LEAP_MODEL_LIFT &&
            DAT_1020b430[index].state == LEAP_STATE_CLIMB_CROUCH)
        {
            FUN_10065b10(index, variant == 0 ? LEAP_LAND_SOUND : LEAP_LAND_SOUND_VARIANT, 0, 0);
            Vec4 dustPosition;
            EffectOptions dustOptions;
            Matrix4 dust;
            float dustZero = 0.0f;
            dustPosition.Fill(dustZero);
            dust.SetIdentity();
            dust.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
            dust._43 = DAT_104bfe24 + LEAP_DUST_PIT_LIFT;
            dustOptions.m_0004 = 0;
            dustOptions.m_0000 = 0;
            FUN_1001d8b0(index, LEAP_DUST_EFFECT, dustPosition, -1, -1, 0, 0, 0, &dust, &dustOptions);
            DAT_1020b430[index].dustDone = 1;
        }
    }
    else if (state == LEAP_STATE_CLIMB_JUMP)
    {
        int dustDone = DAT_1020b430[index].dustDone;
        DAT_104483e8[index] = -1;
        DAT_10218fc0[index] = 1;
        if (dustDone == 0 && DAT_104bfe24 < DAT_10db6ee8[index].matrix._43 + LEAP_MODEL_LIFT)
        {
            FUN_10065b10(index, variant == 0 ? LEAP_LAND_SOUND : LEAP_LAND_SOUND_VARIANT, 0, 0);
            Vec4 dustPosition;
            EffectOptions dustOptions;
            Matrix4 dust;
            float dustZero = 0.0f;
            dustPosition.Fill(dustZero);
            dust.SetIdentity();
            dust.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
            dust._43 = DAT_104bfe24 + LEAP_RISE_STEP;
            dustOptions.m_0004 = 0;
            dustOptions.m_0000 = 0;
            FUN_1001d8b0(index, LEAP_DUST_EFFECT, dustPosition, -1, -1, 0, 0, 0, &dust, &dustOptions);
            DAT_1020b430[index].dustDone = 1;
        }
        int frames = DAT_1020b430[index].leapFrames;
        move.x = move.x * LEAP_JUMP_BOOST;
        move.y = move.y * LEAP_JUMP_BOOST;
        DAT_1020b430[index].leapFrames = frames + 1;
        move.z = (float)(LEAP_JUMP_LIFT_BASE - frames) * LEAP_JUMP_LIFT;
        if (frames + 1 < LEAP_JUMP_LAUNCH)
            DAT_10db6ee8[index].PlayMotion(4, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
        else if (frames + 1 == LEAP_JUMP_END_FRAME)
        {
            DAT_1020b430[index].state = LEAP_STATE_CLIMB_RISE;
            DAT_1020b430[index].leapFrames = 0;
            DAT_1020b430[index].dustDone = 0;
            DAT_10db6ee8[index].PlayMotion(7, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
        }
        else
        {
            if (frames + 1 == LEAP_JUMP_SWEEP_FRAME)
            {
                if (DAT_1020b430[index].sweepSkipped == 0)
                {
                    Vec4 spot;
                    Vec4 aim;
                    EffectOptions sweepOptions;
                    Matrix4 shot;
                    float savedHeading = DAT_10db6ee8[index].heading;
                    DAT_10db6ee8[index].heading = DAT_10db6ee8[index].targetHeading;
                    shot.SetIdentity();
                    spot.Copy(player);
                    spot.z = spot.z + LEAP_SWEEP_SPOT_LIFT;
                    float aimZero = 0.0f;
                    aim.Fill(aimZero);
                    aim.z = LEAP_SWEEP_AIM_HEIGHT;
                    if (FUN_100242c0(index, 0, flatRange, 0, 1, &aim) == 0)
                    {
                        DAT_10db6ee8[index].heading = savedHeading;
                        aim.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
                        aim.z = aim.z + LEAP_SWEEP_AIM_HEIGHT;
                        int first = 0;
                        if (DAT_10c635f8 == 2)
                            first = -1;
                        for (int i = first; i <= -first; i++)
                        {
                            if (player.z <= aim.z)
                            {
                                if ((aim - spot).LengthSquared3() == 0.0f)
                                    shot = RotationX(-LEAP_HALF_PI) *
                                           RotationZ((float)i * LEAP_SWEEP_ANGLE + DAT_10db6ee8[index].targetHeading);
                                else
                                    shot = RotationX(-acosf((aim - spot).Length2D() / (aim - spot).Length())) *
                                           RotationZ((float)i * LEAP_SWEEP_ANGLE + DAT_10db6ee8[index].targetHeading);
                            }
                            else
                            {
                                if ((aim - spot).LengthSquared3() == 0.0f)
                                    shot = RotationX(LEAP_HALF_PI) *
                                           RotationZ((float)i * LEAP_SWEEP_ANGLE + DAT_10db6ee8[index].targetHeading);
                                else
                                    shot = RotationX(acosf((aim - spot).Length2D() / (aim - spot).Length())) *
                                           RotationZ((float)i * LEAP_SWEEP_ANGLE + DAT_10db6ee8[index].targetHeading);
                            }
                            float spotZero = 0.0f;
                            spot.Fill(spotZero);
                            shot.SetTranslation(aim);
                            sweepOptions.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                            sweepOptions.m_0000 = 0;
                            if (variant == 0)
                                FUN_1001d8b0(index, LEAP_SPIT_EFFECT, spot, -1, -1, 0, 0, 0, &shot, &sweepOptions);
                            else
                            {
                                sweepOptions.m_0008 = LEAP_VARIANT_EFFECT_SIZE;
                                FUN_1001d8b0(index, LEAP_SPIT_EFFECT_VARIANT, spot, -1, -1, 0, 0, 0, &shot,
                                             &sweepOptions);
                            }
                        }
                        FUN_10065b10(index, variant == 0 ? LEAP_SPIT_SOUND : LEAP_SPIT_SOUND_VARIANT, 0, 0);
                    }
                    DAT_10db6ee8[index].heading = savedHeading;
                }
                else
                    DAT_1020b430[index].sweepSkipped = 0;
            }
            DAT_10db6ee8[index].PlayMotion(5, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
        }
        if (DAT_10db6ee8[index].IsClearAny(&move) == 0 || range == 0.0f)
        {
            DAT_1020b430[index].state = LEAP_STATE_DOWN;
            DAT_1020b430[index].leapFrames = 0;
            DAT_10db6ee8[index].PlayMotion(0xc, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
            float stopZero = 0.0f;
            DAT_10802bf8[index].step.Fill(stopZero);
            stopZero = 0.0f;
            move.Fill(stopZero);
        }
    }
    else if (state == LEAP_STATE_CLIMB_RISE)
    {
        float frames = (float)DAT_1020b430[index].leapFrames;
        DAT_104483e8[index] = -1;
        DAT_10218fc0[index] = 1;
        move.z = LEAP_RISE_LAUNCH - frames * LEAP_JUMP_LIFT;
        DAT_1020b430[index].leapFrames = 0;
        DAT_1020b430[index].state = LEAP_STATE_CLIMB_GLIDE;
        move.x = move.x * LEAP_RISE_FALL_BOOST;
        move.y = move.y * LEAP_RISE_FALL_BOOST;
        DAT_10db6ee8[index].PlayMotion(7, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
    }
    else if (state == LEAP_STATE_CLIMB_GLIDE)
    {
        DAT_104483e8[index] = -1;
        int frames = DAT_1020b430[index].leapFrames;
        float stepFrames = (float)frames;
        DAT_10218fc0[index] = 1;
        move.z = LEAP_GLIDE_RISE_BASE - stepFrames * LEAP_GLIDE_RISE_STEP;
        if (move.z < LEAP_GLIDE_MIN)
            move.z = LEAP_GLIDE_LOW_BASE - stepFrames * LEAP_GLIDE_LOW_STEP;
        float above = DAT_10db6ee8[index].matrix._43 + LEAP_MODEL_LIFT;
        DAT_1020b430[index].leapFrames = frames + 1;
        if (DAT_104bfe24 <= above)
        {
            float rise = above - DAT_104bfe24;
            move.x = ((DAT_10868838[index][0]._41 - DAT_10db6ee8[index].matrix._41) * rise) / LEAP_GLIDE_DISTANCE;
            move.y = ((DAT_10868838[index][0]._42 - DAT_10db6ee8[index].matrix._42) * rise) / LEAP_GLIDE_DISTANCE;
            if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) == 0 || DAT_10db6ee8[index].IsClearAny(&move) == 0)
            {
                DAT_1020b430[index].state = LEAP_STATE_DOWN;
                DAT_1020b430[index].leapFrames = 0;
                DAT_10db6ee8[index].PlayMotion(0xc, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
                float stopZero = 0.0f;
                DAT_10802bf8[index].step.Fill(stopZero);
                stopZero = 0.0f;
                move.Fill(stopZero);
            }
            else
            {
                DAT_10db6ee8[index].matrix._41 = move.x + DAT_10db6ee8[index].matrix._41;
                DAT_10db6ee8[index].matrix._42 = move.y + DAT_10db6ee8[index].matrix._42;
            }
        }
        else
        {
            DAT_10db6ee8[index].matrix._41 = DAT_10868838[index][0]._41;
            DAT_10db6ee8[index].matrix._42 = DAT_10868838[index][0]._42;
            DAT_1020b430[index].state = LEAP_STATE_LAND;
            DAT_1020b430[index].leapFrames = 0;
            FUN_10065b10(index, variant == 0 ? LEAP_LAND_SOUND : LEAP_LAND_SOUND_VARIANT, 0, 0);
            Vec4 dustPosition;
            EffectOptions dustOptions;
            Matrix4 dust;
            float dustZero = 0.0f;
            dustPosition.Fill(dustZero);
            dust.SetIdentity();
            dust.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
            dust._43 = DAT_104bfe24 + LEAP_RISE_STEP;
            dustOptions.m_0004 = 0;
            dustOptions.m_0000 = 0;
            FUN_1001d8b0(index, LEAP_DUST_EFFECT, dustPosition, -1, -1, 0, 0, 0, &dust, &dustOptions);
        }
        if (DAT_1020b430[index].state != LEAP_STATE_DOWN)
        {
            if (DAT_1020b430[index].leapFrames < LEAP_GLIDE_MOTION_SWITCH)
                DAT_10db6ee8[index].PlayMotion(8, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
            else
                DAT_10db6ee8[index].PlayMotion(9, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
        }
    }
    else if (state == LEAP_STATE_LAND)
    {
        move.z = DAT_10802bf8[index].step.z;
        DAT_104483e8[index] = -1;
        int frames = DAT_1020b430[index].leapFrames;
        move.z = move.z * LEAP_LAND_DAMP;
        DAT_10218fc0[index] = 1;
        DAT_1020b430[index].leapFrames = frames + 1;
        move.x = 0.0f;
        move.y = 0.0f;
        if (frames + 1 == LEAP_LAND_FRAMES)
        {
            DAT_1020b430[index].leapFrames = 0;
            DAT_1020b430[index].state = LEAP_STATE_STAND;
        }
        DAT_10db6ee8[index].PlayMotion(10, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
    }
    else if (state == LEAP_STATE_DOWN)
    {
        DAT_10db6ee8[index].mode = 1;
        if (DAT_10db6ee8[index].m_238c != 0)
        {
            float kickZero = 0.0f;
            DAT_10802bf8[index].step.Fill(kickZero);
            DAT_10802bf8[index].step.y = (float)(FUN_1013f880() % LEAP_DOWN_KICK_RANGE);
            DAT_10802bf8[index].step.RotateZ((float)(FUN_1013f880() % LEAP_DOWN_TURN_RANGE) * LEAP_PI /
                                             LEAP_DEGREES_PER_PI);
            DAT_10db6ee8[index].AddMotionSpeed((float)(FUN_1013f880() % LEAP_DOWN_SPEED_RANGE) + LEAP_DOWN_SPEED_BASE);
            if (FUN_1013f880() % LEAP_DOWN_FACE_ROLL == 0)
                DAT_10db6ee8[index].targetHeading =
                    (float)(FUN_1013f880() % LEAP_DOWN_TURN_RANGE) * LEAP_PI / LEAP_DEGREES_PER_PI;
        }
        move.Copy(DAT_10802bf8[index].step);
        DAT_10db6ee8[index].PlayMotion(0xc, 0, LEAP_BLEND, LEAP_MOTION_SPEED);
        DAT_1020b430[index].lyingDown = 1;
        if (DAT_10db6ee8[index].matrix._43 + LEAP_MODEL_LIFT < DAT_104bfe24 && overPit != 0)
        {
            FUN_10065b10(index, variant == 0 ? LEAP_LAND_SOUND : LEAP_LAND_SOUND_VARIANT, 0, 0);
            Vec4 dustPosition;
            EffectOptions dustOptions;
            Matrix4 dust;
            float dustZero = 0.0f;
            dustPosition.Fill(dustZero);
            dust.SetIdentity();
            dust.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
            dust._43 = DAT_104bfe24 + LEAP_DUST_PIT_LIFT;
            dustOptions.m_0004 = 0;
            dustOptions.m_0000 = 0;
            FUN_1001d8b0(index, LEAP_DUST_EFFECT, dustPosition, -1, -1, 0, 0, 0, &dust, &dustOptions);
            DAT_1020b430[index].lyingDown = 0;
            DAT_1020b430[index].leapFrames = 0;
            DAT_1020b430[index].state = LEAP_STATE_CROUCH;
            DAT_1020b430[index].dustDone = 1;
        }
    }
    if (DAT_1020b430[index].state != LEAP_STATE_LEAP && DAT_1020b430[index].state < LEAP_STATE_SCOUT)
    {
        int hover = DAT_1020b430[index].hoverPattern;
        if (hover == LEAP_HOVER_RISING)
        {
            float totalDrop = DAT_104bfe24 - (float)DAT_1020b430[index].floorHeight;
            int sinkChance = (int)((DAT_104bfe24 - (DAT_10db6ee8[index].matrix._43 + LEAP_MODEL_LIFT)) / totalDrop *
                                   LEAP_BOB_PERCENT);
            int hoverChance = (int)((amount + LEAP_MODEL_LIFT) / totalDrop * LEAP_BOB_PERCENT);
            move.z = LEAP_LEAP_LEVEL;
            if (FUN_1013f880() % LEAP_BOB_ROLL_LOW > sinkChance)
            {
                if (FUN_1013f880() % LEAP_BOB_ROLL_LOW <= hoverChance)
                    DAT_1020b430[index].hoverPattern = LEAP_HOVER_HOVERING;
            }
            else
                DAT_1020b430[index].hoverPattern = LEAP_HOVER_SINKING;
        }
        else if (hover == LEAP_HOVER_HOVERING)
        {
            move.z = LEAP_LEAP_LEVEL - (DAT_10db6ee8[index].speedScale + DAT_10db6ee8[index].speedScale);
            int roll = FUN_1013f880() % LEAP_BOB_ROLL_MID;
            int sinkChance = (int)((DAT_104bfe24 - (DAT_10db6ee8[index].matrix._43 + LEAP_MODEL_LIFT)) /
                                   (DAT_104bfe24 - (float)DAT_1020b430[index].floorHeight) * LEAP_BOB_PERCENT);
            if (roll > sinkChance)
            {
                if (FUN_1013f880() % LEAP_BOB_CHANCE_MID == 0)
                    DAT_1020b430[index].hoverPattern = LEAP_HOVER_HOVERING;
            }
            else
                DAT_1020b430[index].hoverPattern = LEAP_HOVER_SINKING;
        }
        else if (hover == LEAP_HOVER_SINKING)
        {
            move.z = (DAT_10db6ee8[index].speedScale + 1.0f) + (DAT_10db6ee8[index].speedScale + 1.0f);
            int roll = FUN_1013f880() % LEAP_BOB_ROLL_HIGH;
            int hoverChance = (int)((amount + LEAP_MODEL_LIFT) /
                                    (DAT_104bfe24 - (float)DAT_1020b430[index].floorHeight) * LEAP_BOB_PERCENT);
            if (roll > hoverChance)
            {
                if (FUN_1013f880() % LEAP_BOB_CHANCE_MID == 0)
                    DAT_1020b430[index].hoverPattern = LEAP_HOVER_HOVERING;
            }
            else
                DAT_1020b430[index].hoverPattern = LEAP_HOVER_HOVERING;
        }
        else if (hover == LEAP_HOVER_PIT)
        {
            move.z = LEAP_PIT_HOVER_SPEED;
            if (DAT_104bfe24 - LEAP_CLIMB_HEIGHT < DAT_10db6ee8[index].matrix._43)
                DAT_1020b430[index].hoverPattern = LEAP_HOVER_HELD;
        }
    }
    if (DAT_104bfd08 == 0)
        FUN_100241e0(index, move.x, move.y, move.z);
    DAT_10802bf8[index].position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    DAT_10802bf8[index].step.Copy(move);
    DAT_10db6ee8[index].move.x = DAT_10db6ee8[index].move.x * DAT_10db6ee8[index].speedScale;
    DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].move.y * DAT_10db6ee8[index].speedScale;
    return 0;
}
