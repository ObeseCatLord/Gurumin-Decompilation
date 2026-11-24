// The hopping spitter of kind 4 (0x10065bf0, called from 0x100f9670): a creature that flits about the level, turning
// left and right on its way, hops at the player once it noticed him and spits a fan of effects when it is close.

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
#include "game.h"
#include "world.h"

#include "char_ground.h"
#include "enemy_chase_hop.h"
#include "field_item_menu.h"
#include "hop_spit_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40
Matrix4 RotationX(float angle);   // 0x101b1ad0

#define HOPSPIT_STEP_TURN_LEFT 0
#define HOPSPIT_STEP_TURN_RIGHT 1
#define HOPSPIT_STEP_HOP 2
#define HOPSPIT_STEP_REST 3
#define HOPSPIT_STEP_SPIT 4
#define HOPSPIT_PROBE_DEPTH 100000.0f       // how far down the first ground lookups reach
#define HOPSPIT_NO_GROUND (-5000)
#define HOPSPIT_RETRY_LIMIT 20
#define HOPSPIT_RETRY_LIFT 10.0f            // it is lifted this much while no ground is found below it
#define HOPSPIT_PIT_MARGIN 50.0f            // the ground lookup snaps to the pit floor this near above it
#define HOPSPIT_PIT_OFFSET 50
#define HOPSPIT_GROUND_PERIOD 30
#define HOPSPIT_FADE_LIMIT 16               // DAT_102199b8 above this restarts the fade-in
#define HOPSPIT_SIGHT_PERIOD 10
#define HOPSPIT_SIGHT_REACH 1000.0f
#define HOPSPIT_GROUND_FRAMES 0x1e
#define HOPSPIT_HOME_RANGE 600.0f           // farther than this from where it came in it turns back
#define HOPSPIT_HOME_STEP (-10.0f)
#define HOPSPIT_HOVER_LIMIT 155.0f          // above this height over the ground the vertical drift is damped
#define HOPSPIT_DRIFT_DAMP 0.1f
#define HOPSPIT_DRIFT_MIN (-34.0f)
#define HOPSPIT_PI 3.1415f
#define HOPSPIT_WAKE_FIRST 58               // the slots whose same-kind neighbours it wakes: 58 to 91
#define HOPSPIT_WAKE_END 92
#define HOPSPIT_KIND 4
#define HOPSPIT_WAKE_PERIOD 10
#define HOPSPIT_WAKE_RANGE_SQUARED 1000000.0f
#define HOPSPIT_WAKE_HEIGHT 250.0f
#define HOPSPIT_FORGET_RANGE 1000.0f
#define HOPSPIT_SPIT_RANGE 200.0f
#define HOPSPIT_SPIT_ANGLE 0.2618f
#define HOPSPIT_FAN 0.2618f
#define HOPSPIT_FAN_DROP 50.0f
#define HOPSPIT_FAN_EFFECT 0x258
#define HOPSPIT_SPIT_SOUND 0x2a1
#define HOPSPIT_ALERT_SOUND 0x2a2
#define HOPSPIT_ALERT_PROMPT 0xf
#define HOPSPIT_QUESTION_PROMPT 0xd
#define HOPSPIT_PROMPT_FRAMES 0x3c
#define HOPSPIT_WINDUP_FRAMES 0x28
#define HOPSPIT_SPIT_PERIOD 0x1e
#define HOPSPIT_SPIT_END 0x82
#define HOPSPIT_SPIT_COOLDOWN 0x5a
#define HOPSPIT_HEADING_LENGTH 4.0f
#define HOPSPIT_QUARTER_TURN 1.5708f
#define HOPSPIT_TURN 0.0872f
#define HOPSPIT_TURN_EXTRA 0.08727f
#define HOPSPIT_WALL_NEAR 500.0f
#define HOPSPIT_HOP_DAMP 0.95f
#define HOPSPIT_HOP_FRAMES 0x3c
#define HOPSPIT_HOP_LIFT 0.65f
#define HOPSPIT_HOP_FALL 0.061f
#define HOPSPIT_HOP_SPEED_LIMIT 10.0f
#define HOPSPIT_HOP_CEILING 300.0f
#define HOPSPIT_REST_FRAMES 0x9f
#define HOPSPIT_REST_LOW 55.0f
#define HOPSPIT_REST_PULL (-0.1f)
#define HOPSPIT_REST_CLIMB_LIMIT 300.0f
#define HOPSPIT_REST_CLIMB (-30.0f)
#define HOPSPIT_REST_WALL_RANGE 300.0f
#define HOPSPIT_REST_WALL_FRAME 0x81
#define HOPSPIT_SMOOTH_NEW 1.3f
#define HOPSPIT_SMOOTH_OLD 0.55f
#define HOPSPIT_AIM_NEW 0.7f
#define HOPSPIT_AIM_OLD (-0.55f)
#define HOPSPIT_NEAR_RANGE 300.0f
#define HOPSPIT_CHANGE_PERIOD 300
#define HOPSPIT_SPIT_CHANCE 20
#define HOPSPIT_NOTICE_RANGE 1000.0f
#define HOPSPIT_SHIFT_DAMP 0.1f
#define HOPSPIT_FALL_PULL 5.0f
#define HOPSPIT_BLEND 5
#define HOPSPIT_MOTION_SPEED 1.0f
#define HOPSPIT_WALK 0
#define HOPSPIT_HOP_MOTION 1
#define HOPSPIT_SPIT_MOTION 4

// Snaps the ground height of `index` to the pit floor (50 units above it) when `ground` is less than that above the
// pit floor and the slot stands inside the pit rectangle.
static __forceinline void SnapGroundToPit(int index, float ground)
{
    if (DAT_104bfe24 + HOPSPIT_PIT_MARGIN > ground)
    {
        if (DAT_104b7200.PointInRect((int)DAT_10db6ee8[index].matrix._41, (int)DAT_10db6ee8[index].matrix._42,
                                     (int)DAT_104d29e8.x, (int)DAT_104d29e8.y, (int)DAT_104d29f8.x,
                                     (int)DAT_104d29f8.y) != 0)
            DAT_1020b430[index].groundHeight = (int)DAT_104bfe24 + HOPSPIT_PIT_OFFSET;
    }
}

// 0x10065bf0 (Enemy_WanderHopAndSpit). The slot is put on the ground once (it is lifted in 10 unit steps while it
// finds none) and every 30 frames; a ground lookup under the pit area snaps to the pit floor. Every 10th frame it
// probes ahead (1000 units) for an obstacle; far from where it came in (DAT_104c4438) an outward step counts as one.
// Without a sighting of the player it flits about: step 0 turns left, step 1 right (a little more with an obstacle
// ahead), at random it goes to step 2 or 4; step 2 hops at the player (a parabolic lift, 60 frames), step 3 rests
// 160 frames and, at 130, spits a fan of effect 600 (three with DAT_10c635f8 == 2) and waits. Once the player is
// near, in front of it and in sight, step 4 spits: a wind-up of 40 frames, then a fan every 30 frames and a pause
// of 130 frames. An awake one wakes its same-kind neighbours (slots 58 to 91). Returns 1 while the slot is flagged
// gone, 0 otherwise.
int Enemy_WanderHopAndSpit(int index)
{
    int state = DAT_1020b430[index].state;
    int step = DAT_1020b430[index].swoopStep;
    DAT_1020b430[index].updating = 1;
    Vec4 unusedA;
    Vec4 unusedB;
    Vec4 groundSpot;
    Vec4 unusedC;
    if (DAT_10210d30[index] != 0)
    {
        DAT_103086b0[index] = 1;
        return 1;
    }
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].state = 0;
        state = 0;
        step = 0;
        DAT_10254390[index] = 0;
    }
    if (DAT_1020b430[index].groundSetUp == 0)
    {
        DAT_1020b430[index].groundSetUp = 1;
        DAT_10255160[index][0] = 0.0f;
        if (FUN_10015860(DAT_10db6ee8[index].matrix.GetTranslation()) == 0)
        {
            DAT_1020b430[index].groundSetUp = 0;
            return 0;
        }
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        if (FUN_10024750(index, &groundSpot, HOPSPIT_PROBE_DEPTH, 0, 0, 0, 0) != 0)
        {
            int height = (int)groundSpot.z;
            DAT_1020b430[index].groundHeight = height;
            SnapGroundToPit(index, (float)height);
        }
        else
        {
            DAT_1020b430[index].groundHeight = HOPSPIT_NO_GROUND;
            if (DAT_1020b430[index].groundRetries > HOPSPIT_RETRY_LIMIT)
                SnapGroundToPit(index, (float)HOPSPIT_NO_GROUND);
            else
            {
                float lifted = DAT_10db6ee8[index].matrix._43 + HOPSPIT_RETRY_LIFT;
                DAT_1020b430[index].groundRetries++;
                DAT_11b002c0 = 0;
                DAT_10db6ee8[index].matrix._43 = lifted;
                DAT_1020b430[index].groundSetUp = 0;
                DAT_10db6ee8[index].SetPosition(DAT_10db6ee8[index].matrix._41, DAT_10db6ee8[index].matrix._42,
                                                lifted);
                return 0;
            }
        }
        DAT_11b002c0 = 0;
        if (DAT_102199b8[index] > HOPSPIT_FADE_LIMIT)
        {
            DAT_102199b8[index] = 1;
            DAT_10db6ee8[index].alpha = 1.0f;
        }
    }
    if (DAT_104c0380 % HOPSPIT_GROUND_PERIOD == index % HOPSPIT_GROUND_PERIOD)
    {
        Vec4 lookup;
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        if (FUN_10024750(index, &lookup, HOPSPIT_PROBE_DEPTH, 0, 0, 0, 0) != 0)
        {
            int height = (int)lookup.z;
            DAT_1020b430[index].groundHeight = height;
            SnapGroundToPit(index, (float)height);
        }
        DAT_11b002c0 = 0;
    }
    float groundHeight = (float)DAT_1020b430[index].groundHeight;
    DAT_10db6ee8[index].m_238c = 0;
    if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
        DAT_10db6ee8[index].mode = 0;
    if (DAT_104483e8[index] == 0)
        return 0;
    DAT_10218fc0[index] = 0;
    Vec4 player;
    Vec4 place;
    Vec4 diff;
    Vec4 sightSpot;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    place.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    diff.Copy(player - place);
    float flatRange = diff.Length2D();
    float range = diff.Length();
    float obstacleDistance;
    int obstacleAhead;
    if (++DAT_1020b430[index].probeFrames % HOPSPIT_SIGHT_PERIOD == 0)
    {
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        int hit = FUN_100242c0(index, &sightSpot, HOPSPIT_SIGHT_REACH, 0, 0, 0);
        if (hit != 0)
        {
            obstacleDistance = (sightSpot - place).Length2D();
            obstacleAhead = 1;
        }
        else
        {
            obstacleDistance = -1.0f;
            obstacleAhead = 0;
        }
        if (DAT_1020b430[index].probeFrames == HOPSPIT_GROUND_FRAMES)
        {
            DAT_1020b430[index].probeFrames = 0;
            DAT_10db6ee8[index].m_0012 = 2;
            FUN_1001c090(index, 1);
        }
        DAT_11b002c0 = 0;
        DAT_1020b430[index].obstacleDistance = (int)obstacleDistance;
        DAT_1020b430[index].obstacleAhead = obstacleAhead;
        DAT_1020b430[index].shadowHeight = (int)DAT_10db6ee8[index].shadowMatrix._43;
    }
    else
    {
        obstacleDistance = (float)DAT_1020b430[index].obstacleDistance;
        obstacleAhead = DAT_1020b430[index].obstacleAhead;
        DAT_10db6ee8[index].shadowMatrix._43 = (float)DAT_1020b430[index].shadowHeight;
    }
    if (DAT_1020b430[index].obstacleAhead == 0)
    {
        if ((DAT_104c4438[index] - place).Length2D() > HOPSPIT_HOME_RANGE)
        {
            Vec4 away;
            away.y = HOPSPIT_HOME_STEP;
            away.RotateZ(-DAT_10db6ee8[index].heading);
            float homeDistance = (place - DAT_104c4438[index]).Length2D();
            if (((place - DAT_104c4438[index]) + away).Length2D() > homeDistance)
            {
                obstacleAhead = 1;
                DAT_1020b430[index].obstacleAhead = 1;
            }
        }
    }
    float aboveGround = place.z - groundHeight;
    diff.Copy(Vec3_Normalized(&diff));
    float turnError = DAT_10db6ee8[index].heading - atan2f(diff.x, diff.y) - HOPSPIT_PI;
    turnError = (float)fabs(turnError);
    if (aboveGround > HOPSPIT_HOVER_LIMIT)
    {
        float lowered = DAT_10802bf8[index].step.z - HOPSPIT_DRIFT_DAMP;
        DAT_10802bf8[index].step.z = lowered;
        if (lowered < HOPSPIT_DRIFT_MIN)
            DAT_10802bf8[index].step.z = HOPSPIT_DRIFT_MIN;
    }
    else
        DAT_10802bf8[index].step.z = 0.0f;
    if (DAT_1020b430[index].alerted != 0 && DAT_10db6ee8[index].motionFrame % HOPSPIT_WAKE_PERIOD == 0 &&
        DAT_10c66710 == 0)
    {
        Vec4 otherPosition;
        for (int other = HOPSPIT_WAKE_FIRST; other < HOPSPIT_WAKE_END; other++)
        {
            if (DAT_10daaae8[other].kind == HOPSPIT_KIND)
            {
                otherPosition.Copy(DAT_10db6ee8[other].matrix.GetTranslation());
                if (DAT_1030b8f8[other] == 0 &&
                    !((player - otherPosition).LengthSquared3() < HOPSPIT_WAKE_RANGE_SQUARED))
                    continue;
                if (DAT_1020b430[other].updating == 0 && floorf(player.z - otherPosition.z) < HOPSPIT_WAKE_HEIGHT)
                    DAT_1020b430[other].alerted = 1;
            }
        }
    }
    if (DAT_1020b430[index].alerted != 0 && range > HOPSPIT_FORGET_RANGE && DAT_1030b8f8[index] == 0)
        DAT_1020b430[index].alerted = 0;
    if (state < 0 || state > 1)
        DAT_1020b430[index].state = 0;
    else if (state == 0)
    {
        if (step != HOPSPIT_STEP_SPIT &&
            (range > HOPSPIT_SPIT_RANGE || turnError > HOPSPIT_SPIT_ANGLE || step == HOPSPIT_STEP_HOP ||
             step == HOPSPIT_STEP_REST))
        {
            place.Fill(0.0f);
            place.x = 1.0f;
            place.RotateZ(-DAT_10db6ee8[index].heading - HOPSPIT_QUARTER_TURN);
            place.Copy(place * HOPSPIT_HEADING_LENGTH);
            if (step == HOPSPIT_STEP_TURN_LEFT)
            {
                float turned = DAT_10db6ee8[index].heading - HOPSPIT_TURN;
                DAT_10db6ee8[index].targetHeading = turned;
                if (obstacleAhead == 1 && obstacleDistance < HOPSPIT_WALL_NEAR)
                {
                    DAT_10db6ee8[index].targetHeading = turned - HOPSPIT_TURN_EXTRA;
                    DAT_10db6ee8[index].heading = DAT_10db6ee8[index].heading - HOPSPIT_TURN_EXTRA;
                }
                if (DAT_104bfd08 == 0)
                    FUN_100241e0(index, place.x, place.y, 0.0f);
                DAT_10db6ee8[index].PlayMotion(HOPSPIT_WALK, 0, HOPSPIT_BLEND, HOPSPIT_MOTION_SPEED);
            }
            else if (step == HOPSPIT_STEP_TURN_RIGHT)
            {
                float turned = DAT_10db6ee8[index].heading + HOPSPIT_TURN;
                DAT_10db6ee8[index].targetHeading = turned;
                if (obstacleAhead == 1 && obstacleDistance < HOPSPIT_WALL_NEAR)
                {
                    DAT_10db6ee8[index].targetHeading = turned + HOPSPIT_TURN_EXTRA;
                    DAT_10db6ee8[index].heading = DAT_10db6ee8[index].heading + HOPSPIT_TURN_EXTRA;
                }
                if (DAT_104bfd08 == 0)
                    FUN_100241e0(index, place.x, place.y, 0.0f);
                DAT_10db6ee8[index].PlayMotion(HOPSPIT_WALK, 0, HOPSPIT_BLEND, HOPSPIT_MOTION_SPEED);
            }
            else if (step == HOPSPIT_STEP_HOP)
            {
                DAT_104483e8[index] = -1;
                DAT_1020b430[index].alerted = 1;
                place.Copy(DAT_10802bf8[index].position);
                place.Copy(place * HOPSPIT_HOP_DAMP);
                DAT_10802bf8[index].step.z = DAT_10802bf8[index].step.z + 1.0f;
                DAT_1020b430[index].stepFrames++;
                if (DAT_1020b430[index].stepFrames > HOPSPIT_HOP_FRAMES)
                {
                    DAT_1020b430[index].stepFrames = 0;
                    DAT_1020b430[index].swoopStep = HOPSPIT_STEP_REST;
                    DAT_10802bf8[index].step.z = 0.0f;
                }
                if (DAT_104bfd08 == 0)
                {
                    FUN_100241e0(index, place.x, place.y, 0.0f);
                    float lift = (HOPSPIT_HOP_LIFT - DAT_10802bf8[index].step.z * HOPSPIT_HOP_FALL) *
                                 DAT_10802bf8[index].step.z;
                    DAT_10db6ee8[index].move.z = lift;
                    if (lift < 0.0f)
                    {
                        DAT_10db6ee8[index].move.z = 0.0f;
                        DAT_10802bf8[index].step.z = 0.0f;
                    }
                    if (DAT_10db6ee8[index].move.z > HOPSPIT_HOP_SPEED_LIMIT)
                        DAT_10db6ee8[index].move.z = HOPSPIT_HOP_SPEED_LIMIT;
                    if (DAT_10db6ee8[index].matrix._43 - (float)DAT_1020b430[index].groundHeight >
                        HOPSPIT_HOP_CEILING)
                        DAT_10db6ee8[index].move.z = 0.0f;
                }
                DAT_10db6ee8[index].PlayMotion(HOPSPIT_WALK, 0, HOPSPIT_BLEND, HOPSPIT_MOTION_SPEED);
            }
            else if (step == HOPSPIT_STEP_REST)
            {
                DAT_104483e8[index] = -1;
                DAT_1020b430[index].stepFrames++;
                if (DAT_1020b430[index].stepFrames > HOPSPIT_REST_FRAMES)
                {
                    DAT_1020b430[index].stepFrames = 0;
                    if (DAT_1020b430[index].alerted == 0)
                        DAT_1020b430[index].swoopStep = FUN_1013f880() % 3;
                    else if (range >= HOPSPIT_NEAR_RANGE || DAT_1030b8f8[index] == 0)
                        DAT_1020b430[index].swoopStep = HOPSPIT_STEP_HOP;
                    else
                        DAT_1020b430[index].swoopStep = HOPSPIT_STEP_SPIT;
                    DAT_10802bf8[index].step.z = 0.0f;
                    DAT_10db6ee8[index].PlayMotion(HOPSPIT_WALK, 0, HOPSPIT_BLEND, HOPSPIT_MOTION_SPEED);
                }
                if (DAT_1020b430[index].stepFrames == HOPSPIT_SPIT_END)
                {
                    Vec4 mouth;
                    Vec4 aim;
                    EffectOptions options;
                    Matrix4 matrix;
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
                            aim.z = aim.z - HOPSPIT_FAN_DROP;
                            if (aim.z < player.z)
                            {
                                if (range != 0.0f)
                                    matrix = RotationX(acosf(flatRange / range)) *
                                             RotationZ((float)i * HOPSPIT_FAN + DAT_10db6ee8[index].heading);
                                else
                                    matrix = RotationZ((float)i * HOPSPIT_FAN + DAT_10db6ee8[index].heading);
                            }
                            else if (range != 0.0f)
                                matrix = RotationX(-acosf(flatRange / range)) *
                                         RotationZ((float)i * HOPSPIT_FAN + DAT_10db6ee8[index].heading);
                            else
                                matrix = RotationZ((float)i * HOPSPIT_FAN + DAT_10db6ee8[index].heading);
                            matrix.SetTranslation(aim);
                            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                            options.m_0000 = 0;
                            FUN_1001d8b0(index, HOPSPIT_FAN_EFFECT, mouth, -1, -1, 0, 0, 0, &matrix, &options);
                        }
                        FUN_10065b10(index, HOPSPIT_SPIT_SOUND, 0, 0);
                    }
                    DAT_10db6ee8[index].PlayMotion(HOPSPIT_SPIT_MOTION, 1, 0, HOPSPIT_MOTION_SPEED);
                    DAT_1020b430[index].attackCooldown = HOPSPIT_SPIT_COOLDOWN;
                }
                else if (DAT_1020b430[index].stepFrames < HOPSPIT_SPIT_END)
                {
                    float height = aboveGround - HOPSPIT_REST_LOW;
                    if (height > HOPSPIT_REST_CLIMB_LIMIT)
                        DAT_10802bf8[index].step.z = HOPSPIT_REST_CLIMB;
                    else
                        DAT_10802bf8[index].step.z = height * HOPSPIT_REST_PULL;
                    DAT_10db6ee8[index].targetHeading = atan2f(diff.x, diff.y) + HOPSPIT_PI;
                    if (obstacleAhead == 1 && obstacleDistance < HOPSPIT_REST_WALL_RANGE &&
                        obstacleDistance < flatRange)
                        DAT_1020b430[index].stepFrames = HOPSPIT_REST_WALL_FRAME;
                    place.Copy(place * HOPSPIT_SMOOTH_NEW + DAT_10802bf8[index].position * HOPSPIT_SMOOTH_OLD);
                    DAT_10218fc0[index] = 1;
                    if (DAT_104bfd08 == 0)
                        FUN_100241e0(index, place.x, place.y, 0.0f);
                    DAT_10db6ee8[index].PlayMotion(HOPSPIT_HOP_MOTION, 0, HOPSPIT_BLEND, HOPSPIT_MOTION_SPEED);
                }
            }
            if (FUN_1013f880() % HOPSPIT_CHANGE_PERIOD == 0 && DAT_1020b430[index].swoopStep < 2)
            {
                DAT_1020b430[index].stepFrames = 0;
                if (DAT_1030b8f8[index] == 0 && DAT_1020b430[index].alerted == 0)
                    DAT_1020b430[index].swoopStep = FUN_1013f880() % 2;
                else if (DAT_1020b430[index].alerted == 0 || FUN_1013f880() % HOPSPIT_SPIT_CHANCE == 0)
                {
                    int pick = FUN_1013f880() % 3;
                    DAT_1020b430[index].swoopStep = pick;
                    if (pick == 2)
                        DAT_1020b430[index].alerted = 1;
                }
                else if (range >= HOPSPIT_NEAR_RANGE || DAT_1030b8f8[index] == 0)
                    DAT_1020b430[index].swoopStep = HOPSPIT_STEP_HOP;
                else
                    DAT_1020b430[index].swoopStep = HOPSPIT_STEP_SPIT;
                if (DAT_1020b430[index].swoopStep == HOPSPIT_STEP_HOP)
                {
                    FUN_10065b10(index, HOPSPIT_ALERT_SOUND, 0, 0);
                    FUN_1000d620(index, HOPSPIT_ALERT_PROMPT, HOPSPIT_PROMPT_FRAMES);
                    DAT_104483e8[index] = -1;
                }
            }
            if (DAT_1020b430[index].swoopStep < 2 && obstacleAhead != 1 && range < HOPSPIT_NOTICE_RANGE &&
                (turnError < HOPSPIT_SPIT_ANGLE || DAT_1020b430[index].alerted != 0) &&
                (DAT_1030b8f8[index] != 0 || (DAT_1020b430[index].alerted != 0 && range < HOPSPIT_NEAR_RANGE)))
            {
                DAT_1020b430[index].alerted = 1;
                DAT_1020b430[index].stepFrames = 0;
                DAT_1020b430[index].swoopStep = HOPSPIT_STEP_HOP;
                FUN_10065b10(index, HOPSPIT_ALERT_SOUND, 0, 0);
                FUN_1000d620(index, HOPSPIT_ALERT_PROMPT, HOPSPIT_PROMPT_FRAMES);
                DAT_104483e8[index] = -1;
                DAT_1020b430[index].alerted = 1;
            }
        }
        else
        {
            DAT_104483e8[index] = -1;
            DAT_10218fc0[index] = 1;
            if (++DAT_1020b430[index].stepFrames < HOPSPIT_WINDUP_FRAMES)
            {
                DAT_1020b430[index].alerted = 1;
                DAT_1020b430[index].swoopStep = HOPSPIT_STEP_SPIT;
                DAT_10db6ee8[index].PlayMotion(HOPSPIT_HOP_MOTION, 0, HOPSPIT_BLEND, HOPSPIT_MOTION_SPEED);
                FUN_1000d620(index, HOPSPIT_ALERT_PROMPT, HOPSPIT_PROMPT_FRAMES);
            }
            else if (DAT_1020b430[index].stepFrames == HOPSPIT_SPIT_END || DAT_1020b430[index].attackCooldown != 0)
            {
                if (DAT_1020b430[index].attackCooldown == 0)
                    DAT_1020b430[index].attackCooldown = HOPSPIT_SPIT_COOLDOWN;
                DAT_1020b430[index].stepFrames = 0;
                if (DAT_1020b430[index].alerted == 0)
                    DAT_1020b430[index].swoopStep = FUN_1013f880() % 2;
                else if (range >= HOPSPIT_NEAR_RANGE || DAT_1030b8f8[index] == 0)
                    DAT_1020b430[index].swoopStep = HOPSPIT_STEP_HOP;
                else
                    DAT_1020b430[index].swoopStep = HOPSPIT_STEP_SPIT;
                DAT_10db6ee8[index].PlayMotion(HOPSPIT_WALK, 0, HOPSPIT_BLEND, HOPSPIT_MOTION_SPEED);
                if (DAT_1020b430[index].alerted == 0)
                    FUN_1000d620(index, HOPSPIT_QUESTION_PROMPT, HOPSPIT_PROMPT_FRAMES);
            }
            else if ((DAT_1020b430[index].stepFrames - HOPSPIT_WINDUP_FRAMES) % HOPSPIT_SPIT_PERIOD == 0 &&
                     DAT_1030b8f8[index] != 0)
            {
                Vec4 mouth;
                Vec4 aim;
                EffectOptions options;
                Matrix4 matrix;
                matrix.SetIdentity();
                int first = 0;
                if (DAT_10c635f8 == 2)
                    first = -1;
                int last = -first;
                for (int i = first; i <= last; i++)
                {
                    mouth.Fill(0.0f);
                    aim.Copy(DAT_10db6ee8[index].headNode->worldMatrix.GetTranslation());
                    aim.z = aim.z - HOPSPIT_FAN_DROP;
                    if (aim.z < player.z)
                    {
                        if (range != 0.0f)
                            matrix = RotationX(acosf(flatRange / range)) *
                                     RotationZ((float)i * HOPSPIT_FAN + DAT_10db6ee8[index].heading);
                        else
                            matrix = RotationZ((float)i * HOPSPIT_FAN + DAT_10db6ee8[index].heading);
                    }
                    else if (range != 0.0f)
                        matrix = RotationX(-acosf(flatRange / range)) *
                                 RotationZ((float)i * HOPSPIT_FAN + DAT_10db6ee8[index].heading);
                    else
                        matrix = RotationZ((float)i * HOPSPIT_FAN + DAT_10db6ee8[index].heading);
                    matrix.SetTranslation(aim);
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0000 = 0;
                    FUN_1001d8b0(index, HOPSPIT_FAN_EFFECT, mouth, -1, -1, 0, 0, 0, &matrix, &options);
                }
                DAT_10db6ee8[index].PlayMotion(HOPSPIT_SPIT_MOTION, 1, 0, HOPSPIT_MOTION_SPEED);
                FUN_10065b10(index, HOPSPIT_SPIT_SOUND, 0, 0);
            }
            place.Copy(diff);
            place.Copy(place * HOPSPIT_AIM_NEW + DAT_10802bf8[index].position * HOPSPIT_AIM_OLD);
            DAT_10db6ee8[index].targetHeading = atan2f(place.x, place.y) + HOPSPIT_PI;
            if (DAT_104bfd08 == 0)
                FUN_100241e0(index, -place.x, -place.y, 0.0f);
            place.Copy(place * -1.0f);
        }
    }
    DAT_10802bf8[index].position.Copy(place);
    if (DAT_10255160[index][0] > 0.0f)
    {
        float shift = DAT_10255160[index][0] - HOPSPIT_SHIFT_DAMP;
        DAT_10255160[index][0] = shift;
        if (shift <= DAT_10802bf8[index].step.z)
        {
            DAT_10802bf8[index].step.z = shift + DAT_10802bf8[index].step.z;
            DAT_10255160[index][0] = 0.0f;
        }
        else
        {
            DAT_10255160[index][0] = shift + DAT_10802bf8[index].step.z;
            DAT_10802bf8[index].step.z = 0.0f;
        }
    }
    if (DAT_10255160[index][0] < 0.0f)
        DAT_10255160[index][0] = 0.0f;
    float limit = step == HOPSPIT_STEP_REST ? HOPSPIT_REST_LOW : HOPSPIT_HOVER_LIMIT;
    if (limit - aboveGround <= 0.0f)
        DAT_10db6ee8[index].move.z =
            DAT_10db6ee8[index].move.z + DAT_10255160[index][0] + DAT_10802bf8[index].step.z;
    else
        DAT_10db6ee8[index].move.z = DAT_10db6ee8[index].move.z + HOPSPIT_FALL_PULL;
    DAT_1020b430[index].updating = 0;
    DAT_10db6ee8[index].move.x = DAT_10db6ee8[index].move.x * DAT_10db6ee8[index].speedScale;
    DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].move.y * DAT_10db6ee8[index].speedScale;
    if (DAT_1020b430[index].attackCooldown > 0)
        DAT_1020b430[index].attackCooldown--;
    return 0;
}
