// The ground enemy that walks about, spits a fan of effects at the player and charges at him when he is close
// (0x1006be40, a per-kind behaviour of 0x100f9670). See the comment above Enemy_SpitAndCharge for the steps.

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

#include "field_item_menu.h"
#include "spit_charge_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40
Matrix4 RotationX(float angle);   // 0x101b1ad0

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define SPITCHARGE_SPINE_NODE "Bip01 Spine"
#define SPITCHARGE_STAT_RETALIATES 6        // CharSlot::stats[6]: non-zero = its kind spits back when hit
#define SPITCHARGE_PI 3.1415f
#define SPITCHARGE_TWO_PI 6.283f
#define SPITCHARGE_QUARTER_TURN 1.5708f
#define SPITCHARGE_SIGHT_HEIGHT 0x96        // a ledge probe that found a step lower than this still sees the player
#define SPITCHARGE_ALERT_RANGE_SQUARED 1210000.0f
#define SPITCHARGE_ALERT_HEIGHT 500.0f
#define SPITCHARGE_ALERT_FRAMES 600
#define SPITCHARGE_GROUND_PERIOD 30
#define SPITCHARGE_SHADOW_LIFT 10.0f
#define SPITCHARGE_SHADOW_DEPTH 1000.0f
#define SPITCHARGE_NO_GROUND (-3500.0f)
#define SPITCHARGE_WAKE_FIRST 58            // the slots whose same-kind neighbours it wakes: 58 to 91
#define SPITCHARGE_WAKE_END 92
#define SPITCHARGE_WAKE_PERIOD 10
#define SPITCHARGE_WAKE_RANGE_SQUARED 1000000.0f
#define SPITCHARGE_WAKE_HEIGHT 250.0f
#define SPITCHARGE_REACT_HEIGHT 250.0f
#define SPITCHARGE_NOTICE_ANGLE 0.17453f
#define SPITCHARGE_NOTICE_ANGLE_FAR 0.34907f
#define SPITCHARGE_NOTICE_NEAR 400.0f
#define SPITCHARGE_NOTICE_FAR 1000.0f
#define SPITCHARGE_STEP_PERIOD 5            // the ground ahead is looked at once in this many frames
#define SPITCHARGE_PROBE_PERIOD 10
#define SPITCHARGE_PROBE_MOVE (-10.0f)
#define SPITCHARGE_PROBE_LIFT 10.0f
#define SPITCHARGE_AHEAD_REACH (-200.0f)
#define SPITCHARGE_AHEAD_LIFT 50.0f
#define SPITCHARGE_AHEAD_DROP 900.0f
#define SPITCHARGE_AHEAD_DROP_LOW 425.0f    // the look-ahead is shorter while it is hit-reacting (m_238c)
#define SPITCHARGE_MOVE_SPEED (-3.0f)
#define SPITCHARGE_STEP_WANDER 0
#define SPITCHARGE_STEP_REACT 1
#define SPITCHARGE_STEP_APPROACH 2
#define SPITCHARGE_STEP_SPIT 3
#define SPITCHARGE_STEP_CHARGE 4
#define SPITCHARGE_STEP_IDLE 5
#define SPITCHARGE_STEP_TURN 6
#define SPITCHARGE_GAIT_WALKING 0
#define SPITCHARGE_GAIT_PAUSING 1
#define SPITCHARGE_GAIT_RECOVERING 2
#define SPITCHARGE_GAIT_STILL 3
#define SPITCHARGE_WANDER_ROLL 300
#define SPITCHARGE_TURN_ROLL 100
#define SPITCHARGE_IDLE_ROLL 200
#define SPITCHARGE_IDLE_CHECK 50
#define SPITCHARGE_WANDER_TURN_RANGE 10000
#define SPITCHARGE_WANDER_TURN 0.0001f
#define SPITCHARGE_ALERT_PROMPT 0xf
#define SPITCHARGE_PROMPT_FRAMES 0x3c
#define SPITCHARGE_REACT_SPEED 27.0f
#define SPITCHARGE_REACT_SOUND 0x2b3
#define SPITCHARGE_REACT_LEAD 15            // frames the reaction lasts before it starts approaching
#define SPITCHARGE_FAN 0.2618f
#define SPITCHARGE_FAN_RAISE 250.0f
#define SPITCHARGE_FAN_DROP 50.0f
#define SPITCHARGE_FAN_EFFECT 0x258
#define SPITCHARGE_SPIT_SOUND 0x2b4
#define SPITCHARGE_CHARGE_SOUND 0x2b5
#define SPITCHARGE_DUST_EFFECT 0x655
#define SPITCHARGE_APPROACH_SPEED (-10.0f)
#define SPITCHARGE_APPROACH_CLOSE 150.0f
#define SPITCHARGE_APPROACH_NEAR 300.0f
#define SPITCHARGE_APPROACH_FRAMES 0xb4
#define SPITCHARGE_APPROACH_GIVE_UP 2200.0f
#define SPITCHARGE_LEDGE_DROP 100.0f
#define SPITCHARGE_WALL_CLOSE 200.0f
#define SPITCHARGE_SPIT_WINDUP 10
#define SPITCHARGE_SPIT_TURN_END 0x14
#define SPITCHARGE_SPIT_END 0x1e
#define SPITCHARGE_SPIT_RECOVER 0x5a
#define SPITCHARGE_CHARGE_RANGE 400.0f
#define SPITCHARGE_CHARGE_SPEED 20.0f
#define SPITCHARGE_CHARGE_DAMP 0.985f
#define SPITCHARGE_CHARGE_HIT_FROM 3        // frames 4 to 34 of the charge hit (DAT_1021f6c0)
#define SPITCHARGE_CHARGE_HIT_TO 0x23
#define SPITCHARGE_CHARGE_MIN 0x1e
#define SPITCHARGE_CHARGE_REST 0x28
#define SPITCHARGE_CHARGE_END 0x3c
#define SPITCHARGE_TURN_DONE 0.02f
#define SPITCHARGE_MOTION_CHARGE 7
#define SPITCHARGE_MOTION_IDLE 2
#define SPITCHARGE_BLEND 5
#define SPITCHARGE_MOTION_SPEED 1.0f
#define SPITCHARGE_STRIDE_START 0x1e
#define SPITCHARGE_STRIDE_END 0x9b
#define SPITCHARGE_STRIDE_SKIP 0x69
#define SPITCHARGE_PAUSE_FRAMES 0x5a
#define SPITCHARGE_SHORT_PAUSE 10
#define SPITCHARGE_LIFT_STEP 1.25f
#define SPITCHARGE_LIFT_LIMIT (-2.5f)
#define SPITCHARGE_LIFT_FLOOR (-2.51f)
#define SPITCHARGE_LIFT_RELEASE 0.01f
#define SPITCHARGE_LIFT_SCALE_LIMIT 1.8f
#define SPITCHARGE_LIFT_SCALE_HALF 0.5f
#define SPITCHARGE_LIFT_NEAR_GROUND 50.0f
#define SPITCHARGE_LIFT_SKIP_GROUND 70.0f
#define SPITCHARGE_STRIDE_SPEED 42.0f

// The original expands none of math.h's inline float functions in this function.
#pragma inline_depth(0)

// 0x1006be40 (Enemy_SpitAndCharge). Remembers each frame whether the line to the player is free (a ledge probe
// result of 0, or of 2 with a low step). A walking gait (state 0 to 3: stride, pause, recovery, standing still)
// and the action steps run side by side: step 0 wanders (turning away from walls and ledges), step 1 reacts to the
// player or to a hit (turns, hurries, spits), step 2 approaches him, step 3 spits a fan of effect 600 (three with
// DAT_10c635f8 == 2) and, with him close, step 4 charges at 20 units a frame (damped by 0.985) with a dust effect
// and a hit window; steps 5 and 6 idle and turn. An alert one wakes its same-kind neighbours (slots 58 to 91).
// Every frame ends with the gait motion, the move and the motion speed. Returns 1 while the slot is flagged gone,
// 0 otherwise.
int Enemy_SpitAndCharge(int index)
{
    Vec4 mouth;
    Vec4 aim;
    EffectOptions options;
    Matrix4 matrix;
    int lineClear;
    if (DAT_1020b430[index].ledgeResult == 0 ||
        (DAT_1020b430[index].ledgeResult == 2 && DAT_1020b430[index].ledgeHeight < SPITCHARGE_SIGHT_HEIGHT))
        lineClear = 1;
    else
        lineClear = 0;
    int ledgeAhead = 0;
    int motion = -1;
    if (DAT_10210d30[index] != 0)
        return 1;
    if (DAT_104483e8[index] == 0)
        return 0;
    Vec4 player;
    Vec4 place;
    Vec4 diff;
    Vec4 move;
    Vec4 step;
    Vec4 wall;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    place.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    diff.Copy(player - place);
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].state = SPITCHARGE_GAIT_STILL;
        DAT_1020b430[index].gaitHold = 0;
        DAT_1020b430[index].resetSpare = 0;
        DAT_1020b430[index].gaitFrames = 0;
        DAT_1020b430[index].actionStep = 0;
        DAT_1020b430[index].actionFrames = 0;
        if (DAT_1020b430[index].attackActive != 0)
            DAT_1020b430[index].attackActive = 0;
        float bearing = (float)atan2(diff.x, diff.y);
        DAT_10db6ee8[index].targetHeading = bearing + SPITCHARGE_PI;
        return 0;
    }
    if (DAT_1020b430[index].healthSeen != DAT_10daaae8[index].stats[STAT_HEALTH])
    {
        if (DAT_1020b430[index].healthSeen == 0)
        {
            DAT_1020b430[index].state = SPITCHARGE_GAIT_STILL;
            DAT_1020b430[index].healthSeen = DAT_10daaae8[index].stats[STAT_HEALTH];
            DAT_1021bdf8[index][1] = DAT_10db6ee8[index].root->Find(SPITCHARGE_SPINE_NODE, 0);
            if (DAT_10daaae8[index].stats[SPITCHARGE_STAT_RETALIATES] != 0)
                DAT_1020b430[index].retaliates = 1;
        }
        else
        {
            DAT_10db6ee8[index].targetHeading = atan2f(diff.x, diff.y) + SPITCHARGE_PI;
            DAT_1020b430[index].healthSeen = DAT_10daaae8[index].stats[STAT_HEALTH];
            DAT_1020b430[index].state = SPITCHARGE_GAIT_STILL;
            DAT_1020b430[index].gaitHold = 0;
            DAT_1020b430[index].resetSpare = 0;
            DAT_1020b430[index].gaitFrames = 0;
            DAT_1020b430[index].actionFrames = 0;
            DAT_10db6ee8[index].m_238c = 1;
            DAT_1020b430[index].actionStep = (DAT_1020b430[index].retaliates != 0) * 2 + 1;
        }
    }
    if (DAT_1030b8f8[index] != 0 && diff.LengthSquared3() < SPITCHARGE_ALERT_RANGE_SQUARED &&
        floorf(diff.z) < SPITCHARGE_ALERT_HEIGHT)
        DAT_1020b430[index].alertFrames = SPITCHARGE_ALERT_FRAMES;
    if (DAT_1020b430[index].alertFrames > 0)
        DAT_1020b430[index].alertFrames--;
    DAT_10218fc0[index] = 0;
    if (DAT_104c0380 % SPITCHARGE_GROUND_PERIOD == index % SPITCHARGE_GROUND_PERIOD)
    {
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        DAT_10db6ee8[index].m_0012 = 2;
        if (DAT_102022b8 != 0)
        {
            Vec4 from;
            Vec4 ground;
            Vec4 top = DAT_10db6ee8[index].matrix.GetTranslation();
            from.Copy(top);
            top.z += SPITCHARGE_SHADOW_LIFT;
            Vec4 direction = *DAT_10804bf8.LightDirection(&DAT_10db6ee8[index], &DAT_118e69e8, &DAT_118e6cb8);
            direction.Normalized3();
            float distance = SPITCHARGE_SHADOW_DEPTH;
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
                ground.z = SPITCHARGE_NO_GROUND;
            }
            DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
        }
        DAT_11b002c0 = 0;
    }
    if (DAT_1020b430[index].actionStep >= 1 && DAT_1020b430[index].actionStep <= 4 &&
        DAT_10db6ee8[index].motionFrame % SPITCHARGE_WAKE_PERIOD == 0 && DAT_10c66710 == 0)
    {
        Vec4 otherPosition;
        for (int other = SPITCHARGE_WAKE_FIRST; other < SPITCHARGE_WAKE_END; other++)
        {
            if (DAT_10daaae8[index].kind == DAT_10daaae8[other].kind && index != other)
            {
                otherPosition.Copy(DAT_10db6ee8[other].matrix.GetTranslation());
                int otherClear;
                if (DAT_1020b430[other].ledgeResult == 0 ||
                    (DAT_1020b430[other].ledgeResult == 2 &&
                     DAT_1020b430[other].ledgeHeight < SPITCHARGE_SIGHT_HEIGHT))
                    otherClear = 1;
                else
                    otherClear = 0;
                if (DAT_1030b8f8[other] == 0 &&
                    !((player - otherPosition).LengthSquared3() < SPITCHARGE_WAKE_RANGE_SQUARED))
                    continue;
                if ((DAT_1020b430[other].actionStep < 1 || DAT_1020b430[other].actionStep > 4) &&
                    floorf(diff.z) < SPITCHARGE_WAKE_HEIGHT && otherClear != 0)
                {
                    if (DAT_1020b430[other].healthSeen != 0)
                        DAT_1020b430[other].healthSeen = -1;
                    DAT_1020b430[other].alertFrames = SPITCHARGE_ALERT_FRAMES;
                }
            }
        }
    }
    float flatRange = (place - player).Length2D();
    float range = (place - player).Length();
    float turnError = DAT_10db6ee8[index].heading - (float)atan2(diff.x, diff.y) - SPITCHARGE_PI;
    turnError = (float)fabs(turnError);
    float savedHeading = DAT_10db6ee8[index].heading;
    step.Fill(0.0f);
    step.y = SPITCHARGE_PROBE_MOVE;
    step.RotateZ(-DAT_10db6ee8[index].heading);
    DAT_10db6ee8[index].matrix._41 = step.x + DAT_10db6ee8[index].matrix._41;
    DAT_10db6ee8[index].matrix._42 = step.y + DAT_10db6ee8[index].matrix._42;
    DAT_10db6ee8[index].matrix._43 = DAT_10db6ee8[index].matrix._43 + SPITCHARGE_PROBE_LIFT;
    int blocked;
    if (DAT_10db6ee8[index].IsClearAny(0) == 0 || DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0)
        blocked = 1;
    else
        blocked = 0;
    DAT_10db6ee8[index].matrix._41 = place.x;
    DAT_10db6ee8[index].matrix._43 = place.z;
    DAT_10db6ee8[index].matrix._42 = place.y;
    int ledgeState;
    float wallDistance;
    float drop;
    if (DAT_10db6ee8[index].motionFrame % SPITCHARGE_PROBE_PERIOD == (index + 2) % SPITCHARGE_PROBE_PERIOD)
    {
        DAT_10db6ee8[index].heading = atan2f(diff.x, diff.y) + SPITCHARGE_PI;
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        float ledgeHeight;
        ledgeState = FUN_10050df0(index, &wall, &step, &wallDistance, &ledgeHeight, &drop, diff.Length2D());
        DAT_11b002c0 = 0;
        DAT_10db6ee8[index].heading = savedHeading;
        if (drop < 0.0f)
            drop = 0.0f;
        DAT_1020b430[index].ledgeResult = ledgeState;
        DAT_1020b430[index].ledgeHeight = (int)ledgeHeight;
        DAT_10255160[index][2] = wallDistance;
        DAT_10255160[index][3] = drop;
    }
    else
    {
        ledgeState = DAT_1020b430[index].ledgeResult;
        drop = DAT_10255160[index][3];
        wallDistance = DAT_10255160[index][2];
    }
    move.Fill(0.0f);
    DAT_10db6ee8[index].mode = 1;
    if (DAT_10db6ee8[index].motionFrame % SPITCHARGE_STEP_PERIOD == index % SPITCHARGE_STEP_PERIOD)
    {
        Vec4 to;
        Vec4 from;
        Vec4 hit;
        from.Fill(0.0f);
        from.y = DAT_10db6ee8[index].speedScale * SPITCHARGE_AHEAD_REACH;
        from.RotateZ(-DAT_10db6ee8[index].heading);
        from.Copy(from + place);
        from.z = from.z + SPITCHARGE_AHEAD_LIFT;
        to.Copy(from);
        if (DAT_10db6ee8[index].m_238c == 0)
            to.z = to.z - SPITCHARGE_AHEAD_DROP;
        else
            to.z = to.z - SPITCHARGE_AHEAD_DROP_LOW;
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        if (DAT_10255f08 == 0 || DAT_10255f08->Collide(0, 0x4000000, &from, &to, &hit) == 0)
            ledgeAhead = 1;
        DAT_11b002c0 = 0;
    }
    int actionStep = DAT_1020b430[index].actionStep;
    if (actionStep == SPITCHARGE_STEP_WANDER)
    {
        move.y = SPITCHARGE_MOVE_SPEED;
        move.RotateZ(-DAT_10db6ee8[index].heading);
        if (DAT_1020b430[index].state == SPITCHARGE_GAIT_RECOVERING && DAT_1020b430[index].gaitHold == 1)
        {
            DAT_1020b430[index].state = SPITCHARGE_GAIT_STILL;
            DAT_1020b430[index].gaitHold = 0;
            DAT_1020b430[index].gaitMotion = 0;
        }
        else if (DAT_1020b430[index].state == SPITCHARGE_GAIT_STILL &&
                 (FUN_1013f880() % SPITCHARGE_WANDER_ROLL == 0 || blocked != 0))
            DAT_1020b430[index].state = SPITCHARGE_GAIT_WALKING;
        int gait = DAT_1020b430[index].state;
        if ((gait == SPITCHARGE_GAIT_RECOVERING && blocked != 0) ||
            (gait == SPITCHARGE_GAIT_WALKING && DAT_10db6ee8[index].m_238c == 0 && blocked != 0) ||
            (gait == SPITCHARGE_GAIT_STILL && blocked != 0) || ledgeAhead != 0)
        {
            if (DAT_1020b430[index].turnsRight != 0)
                DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading + SPITCHARGE_QUARTER_TURN;
            else
                DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading - SPITCHARGE_QUARTER_TURN;
            DAT_1020b430[index].actionStep = SPITCHARGE_STEP_TURN;
        }
        if (FUN_1013f880() % SPITCHARGE_TURN_ROLL == 0)
        {
            DAT_1020b430[index].actionStep = SPITCHARGE_STEP_TURN;
            if (DAT_1020b430[index].turnsRight != 0)
                DAT_10db6ee8[index].targetHeading =
                    (float)(FUN_1013f880() % SPITCHARGE_WANDER_TURN_RANGE) * SPITCHARGE_WANDER_TURN +
                    DAT_10db6ee8[index].targetHeading;
            else
                DAT_10db6ee8[index].targetHeading =
                    DAT_10db6ee8[index].targetHeading -
                    (float)(FUN_1013f880() % SPITCHARGE_WANDER_TURN_RANGE) * SPITCHARGE_WANDER_TURN;
        }
        else if (FUN_1013f880() % SPITCHARGE_IDLE_ROLL == 0 && DAT_10db6ee8[index].m_238c != 0)
            DAT_1020b430[index].actionStep = SPITCHARGE_STEP_IDLE;
        float heightGap = (float)fabs(diff.z);
        if (heightGap < SPITCHARGE_REACT_HEIGHT &&
            (turnError < SPITCHARGE_NOTICE_ANGLE || (DAT_10c66710 == 0 && turnError < SPITCHARGE_NOTICE_ANGLE_FAR)) &&
            (range < SPITCHARGE_NOTICE_NEAR || (DAT_1030b8f8[index] != 0 && range < SPITCHARGE_NOTICE_FAR)) &&
            DAT_1030b8f8[index] != 0 && DAT_10db6ee8[index].m_238c != 0 && lineClear != 0)
        {
            DAT_1020b430[index].actionStep = SPITCHARGE_STEP_REACT;
            FUN_1000d620(index, SPITCHARGE_ALERT_PROMPT, SPITCHARGE_PROMPT_FRAMES);
            DAT_104483e8[index] = -1;
        }
    }
    else if (actionStep == SPITCHARGE_STEP_REACT)
    {
        DAT_104483e8[index] = -1;
        if (DAT_10db6ee8[index].m_238c != 0 && DAT_10db6ee8[index].mode == 1 &&
            DAT_1020b430[index].state == SPITCHARGE_GAIT_STILL && DAT_10db6ee8[index].motion->layer < 4 &&
            DAT_10db6ee8[index].motion->nextLayer < 4)
            DAT_10db6ee8[index].AddMotionSpeed(SPITCHARGE_REACT_SPEED);
        if (DAT_1020b430[index].actionFrames == 0)
        {
            if (DAT_10db6ee8[index].m_238c != 0 && DAT_10db6ee8[index].mode == 1 &&
                DAT_1020b430[index].state == SPITCHARGE_GAIT_STILL && DAT_10db6ee8[index].motion->layer < 4 &&
                DAT_10db6ee8[index].motion->nextLayer < 4)
                FUN_10065b10(index, SPITCHARGE_REACT_SOUND, 0, 0);
            float angle = DAT_10db6ee8[index].heading - atan2f(place.x - player.x, place.y - player.y);
            for (; angle > SPITCHARGE_PI; angle = angle - SPITCHARGE_TWO_PI)
            {
            }
            for (; angle < -SPITCHARGE_PI; angle = angle + SPITCHARGE_TWO_PI)
            {
            }
            if (floorf(angle) < SPITCHARGE_QUARTER_TURN || DAT_1020b430[index].retaliates != 0)
            {
                matrix.SetIdentity();
                if (DAT_10c635f8 < 2)
                {
                    mouth.Fill(0.0f);
                    aim.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
                    aim.z = aim.z + SPITCHARGE_FAN_RAISE;
                    if (place.z < player.z)
                        matrix = RotationX(acosf(flatRange / (aim - player).Length())) *
                                 RotationZ(DAT_10db6ee8[index].heading);
                    else
                        matrix = RotationX(-acosf(flatRange / (aim - player).Length())) *
                                 RotationZ(DAT_10db6ee8[index].heading);
                    matrix.SetTranslation(aim);
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0000 = 0;
                    FUN_1001d8b0(index, SPITCHARGE_FAN_EFFECT, mouth, -1, -1, 0, 0, 0, &matrix, &options);
                }
                else
                {
                    for (int i = -1; i < 2; i++)
                    {
                        mouth.Fill(0.0f);
                        aim.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
                        aim.z = aim.z + SPITCHARGE_FAN_RAISE;
                        if (place.z < player.z)
                            matrix = RotationX(acosf(flatRange / (aim - player).Length())) *
                                     RotationZ((float)i * SPITCHARGE_FAN + DAT_10db6ee8[index].heading);
                        else
                            matrix = RotationX(-acosf(flatRange / (aim - player).Length())) *
                                     RotationZ((float)i * SPITCHARGE_FAN + DAT_10db6ee8[index].heading);
                        matrix.SetTranslation(aim);
                        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                        options.m_0000 = 0;
                        FUN_1001d8b0(index, SPITCHARGE_FAN_EFFECT, mouth, -1, -1, 0, 0, 0, &matrix, &options);
                    }
                }
                FUN_10065b10(index, SPITCHARGE_SPIT_SOUND, 0, 0);
            }
        }
        DAT_1020b430[index].state = SPITCHARGE_GAIT_STILL;
        DAT_1020b430[index].gaitMotion = 0;
        DAT_1020b430[index].actionFrames = DAT_1020b430[index].actionFrames + 1;
        if (DAT_1020b430[index].actionFrames > SPITCHARGE_REACT_LEAD)
        {
            DAT_1020b430[index].actionFrames = 0;
            DAT_1020b430[index].actionStep = SPITCHARGE_STEP_APPROACH;
            DAT_1020b430[index].turnsRight = FUN_1013f880() % 2;
            DAT_1020b430[index].approachFrames = 0;
        }
        DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading;
    }
    else if (actionStep == SPITCHARGE_STEP_APPROACH)
    {
        DAT_1020b430[index].approachFrames++;
        DAT_104483e8[index] = -1;
        float angle = DAT_10db6ee8[index].heading - atan2f(place.x - player.x, place.y - player.y);
        for (; angle > SPITCHARGE_PI; angle = angle - SPITCHARGE_TWO_PI)
        {
        }
        for (; angle < -SPITCHARGE_PI; angle = angle + SPITCHARGE_TWO_PI)
        {
        }
        DAT_10db6ee8[index].targetHeading = atan2f(diff.x, diff.y) + SPITCHARGE_PI;
        if (diff.Length2D() < SPITCHARGE_APPROACH_CLOSE)
            DAT_1020b430[index].actionStep = SPITCHARGE_STEP_SPIT;
        move.y = SPITCHARGE_APPROACH_SPEED;
        move.RotateZ(-DAT_10db6ee8[index].heading);
        if ((DAT_1020b430[index].approachFrames > SPITCHARGE_APPROACH_FRAMES && FUN_1013f880() % 200 == 0) ||
            (diff.Length2D() < SPITCHARGE_APPROACH_NEAR && FUN_1013f880() % 50 < 2) || blocked != 0)
        {
            if (DAT_10db6ee8[index].m_238c == 0)
            {
                if (drop <= SPITCHARGE_LEDGE_DROP)
                {
                    DAT_10db6ee8[index].m_238c = 1;
                    DAT_1020b430[index].modeLocked = 1;
                }
                else
                {
                    DAT_1020b430[index].actionStep = SPITCHARGE_STEP_SPIT;
                    DAT_1020b430[index].state = SPITCHARGE_GAIT_WALKING;
                    DAT_1020b430[index].gaitHold = 0;
                    DAT_1020b430[index].gaitFrames = SPITCHARGE_SHORT_PAUSE;
                    DAT_1020b430[index].actionFrames = SPITCHARGE_SHORT_PAUSE;
                }
            }
            else
            {
                DAT_1020b430[index].actionStep = SPITCHARGE_STEP_SPIT;
                DAT_1020b430[index].state = SPITCHARGE_GAIT_WALKING;
                DAT_1020b430[index].gaitHold = 0;
                DAT_1020b430[index].gaitFrames = 0;
                DAT_1020b430[index].actionFrames = 0;
            }
        }
        if (DAT_1020b430[index].state == SPITCHARGE_GAIT_RECOVERING && DAT_1020b430[index].gaitHold == 1)
        {
            DAT_1020b430[index].state = SPITCHARGE_GAIT_STILL;
            DAT_1020b430[index].gaitHold = 0;
            DAT_1020b430[index].gaitMotion = 0;
        }
        else if (DAT_1020b430[index].state == SPITCHARGE_GAIT_STILL &&
                 ((FUN_1013f880() % SPITCHARGE_WANDER_ROLL == 0 ||
                   (wallDistance < SPITCHARGE_WALL_CLOSE && ledgeState != 0)) &&
                  DAT_1020b430[index].actionStep != SPITCHARGE_STEP_SPIT))
        {
            DAT_1020b430[index].state = SPITCHARGE_GAIT_WALKING;
            DAT_1020b430[index].gaitFrames = 0;
        }
        if (DAT_1020b430[index].alertFrames == 0 || flatRange > SPITCHARGE_APPROACH_GIVE_UP || lineClear == 0 ||
            ledgeAhead != 0)
            DAT_1020b430[index].actionStep = SPITCHARGE_STEP_WANDER;
    }
    else if (actionStep == SPITCHARGE_STEP_SPIT)
    {
        DAT_1020b430[index].actionFrames++;
        DAT_104483e8[index] = -1;
        DAT_10218fc0[index] = 1;
        int frames = DAT_1020b430[index].actionFrames;
        if (frames <= SPITCHARGE_SPIT_WINDUP || frames >= SPITCHARGE_SPIT_TURN_END)
        {
            if (frames == SPITCHARGE_SPIT_TURN_END)
            {
                DAT_1020b430[index].attackActive = 1;
                matrix.SetIdentity();
                if (DAT_1030b8f8[index] != 0)
                {
                    int first = 0;
                    if (DAT_10c635f8 == 2)
                        first = -1;
                    int last = -first;
                    for (int i = first; i <= last; i++)
                    {
                        mouth.Fill(0.0f);
                        aim.Copy(DAT_10db6ee8[index].headNode->worldMatrix.GetTranslation());
                        aim.z = aim.z - SPITCHARGE_FAN_DROP;
                        if (place.z < player.z)
                            matrix = RotationX(acosf(flatRange / range)) *
                                     RotationZ((float)i * SPITCHARGE_FAN + DAT_10db6ee8[index].heading);
                        else
                            matrix = RotationX(-acosf(flatRange / range)) *
                                     RotationZ((float)i * SPITCHARGE_FAN + DAT_10db6ee8[index].heading);
                        matrix.SetTranslation(aim);
                        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                        options.m_0000 = 0;
                        FUN_1001d8b0(index, SPITCHARGE_FAN_EFFECT, mouth, -1, -1, 0, 0, 0, &matrix, &options);
                    }
                    FUN_10065b10(index, SPITCHARGE_SPIT_SOUND, 0, 0);
                }
            }
            else if (frames == SPITCHARGE_SPIT_END)
            {
                DAT_1020b430[index].attackActive = 0;
                DAT_1020b430[index].gaitFrames = SPITCHARGE_SPIT_RECOVER;
            }
        }
        else
            DAT_10db6ee8[index].targetHeading = atan2f(diff.x, diff.y) + SPITCHARGE_PI;
        if (DAT_10db6ee8[index].m_238c != 0)
        {
            if (DAT_1020b430[index].attackActive != 0)
            {
                if (flatRange >= SPITCHARGE_CHARGE_RANGE)
                    motion = SPITCHARGE_MOTION_IDLE;
                DAT_1020b430[index].attackActive = 0;
            }
            DAT_1020b430[index].actionFrames = 0;
            DAT_1020b430[index].state = SPITCHARGE_GAIT_STILL;
            if (flatRange >= SPITCHARGE_CHARGE_RANGE)
            {
                DAT_1020b430[index].actionStep = SPITCHARGE_STEP_APPROACH;
                DAT_1020b430[index].gaitFrames = 0;
                DAT_1020b430[index].gaitMotion = 0;
            }
            else
            {
                DAT_1020b430[index].actionStep = SPITCHARGE_STEP_CHARGE;
                DAT_1020b430[index].gaitMotion = 0;
                DAT_1020b430[index].gaitHold = 0;
                DAT_1020b430[index].gaitFrames = 0;
                DAT_10255160[index][1] = SPITCHARGE_CHARGE_SPEED;
                DAT_10db6ee8[index].targetHeading = atan2f(diff.x, diff.y) + SPITCHARGE_PI;
                motion = SPITCHARGE_MOTION_CHARGE;
                DAT_1020b430[index].attackActive = 1;
                FUN_10065b10(index, SPITCHARGE_CHARGE_SOUND, 0, 0);
                Matrix4 rotation;
                Vec4 origin;
                rotation = RotationZ(DAT_10db6ee8[index].heading);
                EffectOptions dustOptions;
                dustOptions.owner = index;
                FUN_1001d8b0(index, SPITCHARGE_DUST_EFFECT, origin, -1, -1, &DAT_1021bdf8[index][1]->worldMatrix, 0,
                             0, &rotation, &dustOptions);
            }
        }
    }
    else if (actionStep == SPITCHARGE_STEP_CHARGE)
    {
        DAT_104483e8[index] = -1;
        if (ledgeAhead != 0)
        {
            if (DAT_1020b430[index].turnsRight != 0)
                DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading + SPITCHARGE_QUARTER_TURN;
            else
                DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading - SPITCHARGE_QUARTER_TURN;
        }
        move.y = -DAT_10255160[index][1];
        move.RotateZ(-DAT_10db6ee8[index].heading);
        DAT_10218fc0[index] = 1;
        DAT_10255160[index][1] = DAT_10255160[index][1] * SPITCHARGE_CHARGE_DAMP;
        if ((DAT_1020b430[index].state == SPITCHARGE_GAIT_RECOVERING && DAT_1020b430[index].gaitHold == 1) ||
            DAT_1020b430[index].actionFrames < SPITCHARGE_CHARGE_MIN)
        {
            DAT_1020b430[index].state = SPITCHARGE_GAIT_STILL;
            DAT_1020b430[index].gaitHold = 0;
            DAT_1020b430[index].gaitMotion = 0;
        }
        else if (DAT_1020b430[index].state == SPITCHARGE_GAIT_STILL && blocked != 0 && ledgeState == 2 &&
                 DAT_1020b430[index].attackActive == 0)
            DAT_1020b430[index].state = SPITCHARGE_GAIT_WALKING;
        if (DAT_1020b430[index].actionFrames > SPITCHARGE_CHARGE_HIT_FROM &&
            DAT_1020b430[index].actionFrames < SPITCHARGE_CHARGE_HIT_TO)
            DAT_1021f6c0[index] = 1;
        DAT_1020b430[index].actionFrames++;
        if (DAT_1020b430[index].actionFrames <= SPITCHARGE_CHARGE_END)
        {
            if (DAT_1020b430[index].actionFrames == SPITCHARGE_CHARGE_REST)
            {
                motion = SPITCHARGE_MOTION_IDLE;
                DAT_1020b430[index].attackActive = 0;
            }
            else if (DAT_1020b430[index].actionFrames < SPITCHARGE_CHARGE_REST)
                motion = SPITCHARGE_MOTION_CHARGE;
        }
        else
        {
            DAT_1020b430[index].actionFrames = 0;
            DAT_1020b430[index].actionStep = SPITCHARGE_STEP_APPROACH;
            DAT_1020b430[index].turnsRight = 0;
        }
    }
    else if (actionStep == SPITCHARGE_STEP_IDLE || actionStep == SPITCHARGE_STEP_TURN)
    {
        if (actionStep == SPITCHARGE_STEP_IDLE)
        {
            if (DAT_1020b430[index].state == SPITCHARGE_GAIT_RECOVERING && DAT_1020b430[index].gaitHold == 1)
            {
                DAT_1020b430[index].state = SPITCHARGE_GAIT_STILL;
                DAT_1020b430[index].gaitHold = 0;
                DAT_1020b430[index].gaitMotion = 0;
            }
            else if (DAT_1020b430[index].state == SPITCHARGE_GAIT_STILL &&
                     FUN_1013f880() % SPITCHARGE_WANDER_ROLL == 0)
                DAT_1020b430[index].state = SPITCHARGE_GAIT_WALKING;
            if (FUN_1013f880() % SPITCHARGE_TURN_ROLL == 0)
            {
                if (FUN_1013f880() % 2 != 0)
                    DAT_10db6ee8[index].targetHeading =
                        (float)(FUN_1013f880() % SPITCHARGE_WANDER_TURN_RANGE) * SPITCHARGE_WANDER_TURN +
                        DAT_10db6ee8[index].targetHeading;
                else
                    DAT_10db6ee8[index].targetHeading =
                        DAT_10db6ee8[index].targetHeading -
                        (float)(FUN_1013f880() % SPITCHARGE_WANDER_TURN_RANGE) * SPITCHARGE_WANDER_TURN;
            }
            else if (FUN_1013f880() % SPITCHARGE_IDLE_CHECK == 0 && DAT_10db6ee8[index].m_238c != 0)
                DAT_1020b430[index].actionStep = SPITCHARGE_STEP_WANDER;
        }
        else
        {
            move.y = SPITCHARGE_MOVE_SPEED;
            move.RotateZ(-DAT_10db6ee8[index].heading);
            if (floorf(DAT_10db6ee8[index].targetHeading - DAT_10db6ee8[index].heading) < SPITCHARGE_TURN_DONE)
                DAT_1020b430[index].actionStep = SPITCHARGE_STEP_WANDER;
        }
        if (floorf(diff.z) < SPITCHARGE_REACT_HEIGHT &&
            (turnError < SPITCHARGE_NOTICE_ANGLE || (DAT_10c66710 == 0 && turnError < SPITCHARGE_NOTICE_ANGLE_FAR)) &&
            (range < SPITCHARGE_NOTICE_NEAR || (DAT_1030b8f8[index] != 0 && range < SPITCHARGE_NOTICE_FAR)) &&
            DAT_1030b8f8[index] != 0 && DAT_10db6ee8[index].m_238c != 0 && lineClear != 0)
        {
            DAT_1020b430[index].actionStep = SPITCHARGE_STEP_REACT;
            FUN_1000d620(index, SPITCHARGE_ALERT_PROMPT, SPITCHARGE_PROMPT_FRAMES);
        }
    }
    int gait = DAT_1020b430[index].state;
    if (gait == SPITCHARGE_GAIT_WALKING)
    {
        DAT_104483e8[index] = -1;
        if (DAT_1020b430[index].gaitFrames < SPITCHARGE_STRIDE_START)
            DAT_1020b430[index].gaitMotion = 1;
        else if (DAT_1020b430[index].gaitFrames < SPITCHARGE_STRIDE_END)
            DAT_1020b430[index].gaitMotion = 2;
        if (drop < SPITCHARGE_LIFT_NEAR_GROUND && DAT_1020b430[index].gaitFrames > SPITCHARGE_STRIDE_START)
            DAT_1020b430[index].gaitFrames = SPITCHARGE_STRIDE_END;
        DAT_1020b430[index].gaitFrames++;
        if (DAT_1020b430[index].gaitFrames <= SPITCHARGE_STRIDE_END)
        {
            if (DAT_10db6ee8[index].m_238c == 1)
            {
                DAT_10db6ee8[index].AddMotionSpeed(SPITCHARGE_STRIDE_SPEED);
                FUN_10065b10(index, SPITCHARGE_REACT_SOUND, 0, 0);
                DAT_1020b430[index].state = SPITCHARGE_GAIT_WALKING;
                DAT_1020b430[index].gaitHold = SPITCHARGE_SHORT_PAUSE;
                DAT_1020b430[index].gaitFrames = 0;
                DAT_1020b430[index].gaitMotion = 1;
            }
            else if (DAT_1020b430[index].gaitHold == 0)
            {
                if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
                    DAT_10db6ee8[index].mode = 0;
                if (DAT_10255160[index][0] >= SPITCHARGE_LIFT_LIMIT)
                    DAT_10255160[index][0] = DAT_10255160[index][0] - SPITCHARGE_LIFT_STEP;
                else
                {
                    DAT_10255160[index][0] = SPITCHARGE_LIFT_FLOOR;
                    if (DAT_10db6ee8[index].m_237c > SPITCHARGE_LIFT_SCALE_LIMIT)
                        DAT_10db6ee8[index].m_237c = DAT_10db6ee8[index].m_237c * SPITCHARGE_LIFT_SCALE_HALF;
                    if (DAT_10db6ee8[index].m_237c < SPITCHARGE_LIFT_SCALE_LIMIT)
                        DAT_10db6ee8[index].m_237c = SPITCHARGE_LIFT_SCALE_LIMIT;
                }
                if (drop < SPITCHARGE_LIFT_SKIP_GROUND)
                    DAT_1020b430[index].gaitFrames = SPITCHARGE_STRIDE_SKIP;
            }
            else
            {
                DAT_1020b430[index].gaitHold--;
                if (DAT_1020b430[index].gaitHold == 0)
                    DAT_10255160[index][0] = DAT_10db6ee8[index].verticalSpeed - SPITCHARGE_LIFT_RELEASE;
            }
        }
        else
        {
            DAT_1020b430[index].gaitFrames = 0;
            DAT_1020b430[index].state = SPITCHARGE_GAIT_RECOVERING;
            DAT_1020b430[index].gaitHold = SPITCHARGE_PAUSE_FRAMES;
            DAT_10db6ee8[index].verticalSpeed = DAT_10255160[index][0];
            DAT_1020b430[index].gaitMotion = 3;
        }
        if (DAT_10db6ee8[index].m_238c == 1)
            DAT_1020b430[index].gaitMotion = 0;
    }
    else if (gait == SPITCHARGE_GAIT_PAUSING)
    {
        DAT_104483e8[index] = -1;
        DAT_1020b430[index].gaitFrames++;
        if (DAT_1020b430[index].gaitFrames < SPITCHARGE_STRIDE_START)
            DAT_1020b430[index].gaitMotion = 3;
        else
            DAT_1020b430[index].gaitMotion = 0;
        DAT_1020b430[index].gaitHold--;
        if (DAT_1020b430[index].gaitHold < 1)
        {
            DAT_1020b430[index].gaitFrames = 0;
            DAT_10255160[index][0] = DAT_10db6ee8[index].verticalSpeed - SPITCHARGE_LIFT_RELEASE;
            DAT_1020b430[index].gaitHold = 0;
            DAT_1020b430[index].state = SPITCHARGE_GAIT_WALKING;
            DAT_1020b430[index].gaitMotion = 0;
        }
    }
    else if (gait == SPITCHARGE_GAIT_RECOVERING)
    {
        DAT_104483e8[index] = -1;
        DAT_1020b430[index].gaitMotion = 0;
        DAT_1020b430[index].gaitHold--;
        if (DAT_1020b430[index].gaitHold < 1)
        {
            DAT_1020b430[index].gaitHold = 0;
            DAT_1020b430[index].state = SPITCHARGE_GAIT_WALKING;
            DAT_1020b430[index].gaitMotion = 0;
        }
    }
    int motionIndex;
    if (DAT_1020b430[index].attackActive == 1)
        motionIndex = motion;
    else if (DAT_1020b430[index].gaitMotion == 1)
        motionIndex = 4;
    else if (DAT_1020b430[index].gaitMotion == 2)
        motionIndex = 5;
    else if (DAT_1020b430[index].gaitMotion == 3)
        motionIndex = 6;
    else
    {
        motionIndex = 1;
        int actionNow = DAT_1020b430[index].actionStep;
        if (actionNow < 1 || actionNow > 4)
        {
            if (DAT_1020b430[index].state == SPITCHARGE_GAIT_WALKING && DAT_10db6ee8[index].m_238c == 0)
                motionIndex = 1;
            else if (DAT_1020b430[index].state == SPITCHARGE_GAIT_RECOVERING && DAT_10db6ee8[index].m_238c == 0)
                motionIndex = 2;
            else if (actionNow == 0 || actionNow == 6)
                motionIndex = 0;
            else
                motionIndex = 2;
        }
    }
    if (DAT_1020b430[index].modeLocked != 0)
        DAT_10db6ee8[index].mode = 1;
    if (DAT_104bfd08 == 0)
        FUN_100241e0(index, move.x, move.y, move.z);
    DAT_10db6ee8[index].move.x = DAT_10db6ee8[index].move.x * DAT_10db6ee8[index].speedScale;
    DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].move.y * DAT_10db6ee8[index].speedScale;
    if (motionIndex >= 0)
        DAT_10db6ee8[index].PlayMotion(motionIndex, 0, SPITCHARGE_BLEND, SPITCHARGE_MOTION_SPEED);
    return 0;
}
