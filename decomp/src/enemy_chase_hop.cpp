// An enemy chasing the player step by step (step 2 of the enemy update 0x100e0320): it steers round other enemies,
// probes the way ahead and the ground below, hops up ledges and gives up on the chase (step 3) when it cannot get
// closer.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "char_vars.h"
#include "game_vars.h"
#include "last_position.h"
#include "map_vars.h"

#include "enemy_chase_hop.h"

// The game's atan2 wrapper (float arguments, so each difference is rounded to a float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

#define CHASE_PI 3.1415f
#define CHASE_STEP_GIVE_UP 3
#define CHASE_EYE_LIFT 10.0f             // the sight line runs 10 units above both feet
#define CHASE_SIGHT_MASK 4
#define CHASE_SIGHT_ABOVE 600.0f         // target this much higher: give up
#define CHASE_LEDGE_DROP 300.0f          // target this much higher and nothing ahead: the way on is shut
#define CHASE_GROUND_DEPTH 25.0f
#define CHASE_GROUND_LIFT 12.5f
#define CHASE_LEDGE_STEP (-5.0f)
#define CHASE_LEDGE_HEIGHT 320.0f
#define CHASE_NEAR 50.0f
#define CHASE_FAR 200.0f
#define CHASE_FARTHEST 1000.0f
#define CHASE_SLACK 5.0f
#define CHASE_HOP_FRAMES 4
#define CHASE_HOP_SPEED_UP 10.0f
#define CHASE_RUN_AIR (-45.0f)
#define CHASE_RUN_AIR_IDLE (-70.0f)
#define CHASE_RUN_GROUND (-30.0f)
#define CHASE_AIR_INNER 2500.0f
#define CHASE_AIR_RANGE 80000.0f
#define CHASE_GROUND_INNER 40000.0f
#define CHASE_GROUND_RANGE 210000.0f
#define CHASE_LOOK_START 30.0f
#define CHASE_LOOK_STEP 40.0f
#define CHASE_LOOK_END 250.0f
#define CHASE_PROBE_EVERY 3
#define CHASE_PROBE_SCALE 2.0f
#define CHASE_PROBE_RAISE 40.0f
#define CHASE_PROBE_LOWER (-10.0f)
#define CHASE_GROUND_DEPTH_FAR 350.0f
#define CHASE_RISE_PER_FRAME 0.025f
#define CHASE_RISE_LIMIT 5.0f
#define CHASE_SPEED_LIMIT 40.0f
#define CHASE_LAND_SPEED 2.5f
#define CHASE_STILL_DISTANCE 5.0f
#define CHASE_STALL_LIMIT 15
#define CHASE_STALL_LIMIT_AIR 5
#define CHASE_WALK_SPEED 12.0f
#define CHASE_STAND_SPEED 1.0f
#define CHASE_MOTION_BLEND 5
#define CHASE_MOTION_WALK 0
#define CHASE_MOTION_RUN 1
#define CHASE_MOTION_STAND 2
#define CHASE_MOTION_SPECIAL 7

// 0x100bcce0 (Enemy_ChaseAndHop). One frame of the chase of enemy `index` towards `target`, standing at `self`
// (`model` is the character whose idle flag and collision filter it also reads). It first asks 0x10078060 whether
// another enemy is in the way (steer) and probes the ground 25 units below (airborne). A sight line 10 units above
// both feet (and, past a wall, probes ahead and down) says whether the way on is open; more than 600 units below the
// target it gives up (step 3). When it is far away (200 units, 50 while airborne) and not avoiding anyone it works out
// a run step from the distance, turns towards the target and looks for a free direction at 30..250 units height
// (hopTimer 4 when it finds one). Every third frame an idle model probes the ground ahead; otherwise one that is
// below the target hops: its motion speed grows by (5 - airFrames / 40) and, when the way up is blocked, it is lifted.
// It then counts the frames without getting closer (stalledFrames; past 15, or 5 airborne, it gives up), sets the
// step as the move and plays the walk, run or stand motion (or the one 0x10078060 asked for). Returns 0.
extern "C" int FUN_100bcce0(int index, int model, Vec4 *target, Vec4 *self)
{
    Vec4 steer;
    int obstacleFound = 0;
    float distance = (*target - *self).Length();
    int avoiding = 0;
    int requestedMotion = FUN_10078060(index, self, &obstacleFound, &steer);
    if (obstacleFound != 0)
        avoiding = 1;
    int pathOpen;
    if (DAT_10db6ee8[model].m_238c != 0)
    {
        Vec4 ground;
        Vec4 offset;
        offset.Fill(0.0f);
        DAT_11b002c0 = DAT_10db6ee8[model].collisionFilter;
        offset.z = CHASE_GROUND_LIFT;
        if (FUN_10024750(model, &ground, CHASE_GROUND_DEPTH, 0, &offset, 1, 0) == 0)
        {
            DAT_1020b430[index].airborne = 1;
            DAT_11b002c0 = 0;
        }
        else
        {
            DAT_1020b430[index].airborne = 0;
            DAT_11b002c0 = 0;
        }
    }
    Vec4 eye;
    Vec4 targetEye;
    Vec4 sightHit;
    eye.Copy(*self);
    targetEye.Copy(*target);
    eye.z += CHASE_EYE_LIFT;
    targetEye.z += CHASE_EYE_LIFT;
    if (DAT_10255f08 != 0 && DAT_10255f08->Collide(0, CHASE_SIGHT_MASK, &eye, &targetEye, &sightHit) != 0)
    {
        if (self->z + CHASE_SIGHT_ABOVE < target->z)
        {
            DAT_1020b430[index].state = CHASE_STEP_GIVE_UP;
            DAT_1020b430[index].phaseFrames = 0;
        }
        else
        {
            Vec4 ahead;
            Vec4 lift;
            Vec4 below;
            lift.z = CHASE_EYE_LIFT;
            float heading = Atan2(target->x - self->x, target->y - self->y);
            if (FUN_100244b0(index, CHASE_PI - heading, &ahead, (*target - *self).Length2D(), 0, 0, &lift, 0) != 0)
            {
                if (target->z < self->z)
                {
                    eye.Fill(0.0f);
                    eye.y = CHASE_LEDGE_STEP;
                    eye.RotateZ(-(atan2f(target->x - self->x, target->y - self->y) + CHASE_PI));
                    eye.Copy(eye + ahead);
                    pathOpen = FUN_100248e0(eye, &ahead, CHASE_LEDGE_HEIGHT) != 0;
                }
                else if (FUN_100244b0(index, -atan2f(target->x - self->x, target->y - self->y), &below,
                                      (*target - *self).Length2D(), 0, 0, &lift, 0) != 0)
                {
                    eye.Fill(0.0f);
                    eye.y = CHASE_LEDGE_STEP;
                    eye.RotateZ(CHASE_PI - atan2f(target->x - self->x, target->y - self->y));
                    eye.Copy(eye + ahead);
                    pathOpen = FUN_100248e0(eye, &ahead, CHASE_LEDGE_HEIGHT) != 0;
                }
                else if (self->z + CHASE_LEDGE_DROP < target->z)
                {
                    pathOpen = 0;
                }
                else
                {
                    pathOpen = 1;
                }
            }
            else
            {
                pathOpen = 1;
            }
        }
    }
    if (avoiding == 0 &&
        (distance > CHASE_FAR || (DAT_1020b430[index].airborne != 0 && distance > CHASE_NEAR)))
    {
        steer.Fill(0.0f);
        float runSpeed;
        if (DAT_1020b430[index].airborne != 0)
        {
            runSpeed = CHASE_RUN_AIR;
            if (DAT_10db6ee8[index].m_238c == 0)
                runSpeed = CHASE_RUN_AIR_IDLE;
            steer.y = ((((*target - *self).LengthSquared3() - CHASE_AIR_INNER) / CHASE_AIR_RANGE) < 1.0f
                           ? (((*target - *self).LengthSquared3() - CHASE_AIR_INNER) / CHASE_AIR_RANGE)
                           : 1.0f) *
                     runSpeed;
        }
        else
        {
            steer.y = ((((*target - *self).LengthSquared3() - CHASE_GROUND_INNER) / CHASE_GROUND_RANGE) < 1.0f
                           ? (((*target - *self).LengthSquared3() - CHASE_GROUND_INNER) / CHASE_GROUND_RANGE)
                           : 1.0f) *
                     CHASE_RUN_GROUND;
        }
        float yaw = Atan2(target->x - self->x, target->y - self->y);
        DAT_10db6ee8[index].targetHeading = yaw + CHASE_PI;
        steer.z = (float)fabs(steer.y);
        steer.RotateZ(-DAT_10db6ee8[index].targetHeading);
        int clearOfMap = DAT_10db6ee8[index].IsClearOfMapLimited(&steer);
        int found;
        if ((pathOpen != 0 && clearOfMap == 0) || DAT_10db6ee8[index].IsClearAny(&steer) == 0)
        {
            float lookHeight = CHASE_LOOK_START;
            do
            {
                steer.z = lookHeight;
                found = DAT_10db6ee8[index].IsClearOfMapLimited(&steer) && DAT_10db6ee8[index].IsClearAny(&steer);
                lookHeight += CHASE_LOOK_STEP;
            } while (lookHeight < CHASE_LOOK_END);
            if (found != 0 && DAT_1020b430[index].hopTimer <= 0 && DAT_10db6ee8[index].m_238c != 0)
                DAT_1020b430[index].hopTimer = CHASE_HOP_FRAMES;
            clearOfMap = found;
        }
        avoiding = clearOfMap != 0 && DAT_10db6ee8[index].IsClearOfMapLimited(0) && DAT_10db6ee8[index].IsClearAny(0);
        steer.z = 0;
    }
    if (DAT_104bf8a4 % CHASE_PROBE_EVERY == 0 && DAT_10db6ee8[index].m_238c != 0)
    {
        Vec4 probeOffset;
        Vec4 probeGround;
        probeOffset.Copy(steer * CHASE_PROBE_SCALE);
        probeOffset.z += CHASE_PROBE_RAISE;
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        if ((DAT_10db6ee8[model].m_238c != 0 && DAT_1020b430[index].airborne != 0 &&
             self->z + CHASE_NEAR < target->z) ||
            FUN_10024750(index, &probeGround, CHASE_GROUND_DEPTH_FAR, 0, &probeOffset, 1, 0) == 0)
        {
            Vec4 lowered;
            lowered.Copy(steer * CHASE_PROBE_SCALE);
            lowered.z = CHASE_PROBE_LOWER;
            int mapClear = DAT_10db6ee8[index].IsClearOfMapLimited(&lowered);
            int anyClear = DAT_10db6ee8[index].IsClearAny(&lowered);
            if ((DAT_1020b430[index].airborne == 0 || (mapClear & anyClear) != 0 ||
                 self->z + CHASE_NEAR < target->z) &&
                (DAT_10db6ee8[index].m_238c != 0 && DAT_1020b430[index].hopTimer == 0))
                DAT_1020b430[index].hopTimer = CHASE_HOP_FRAMES;
            DAT_1020b430[index].airFrames = 0;
        }
        DAT_11b002c0 = 0;
    }
    else if (DAT_10db6ee8[index].m_238c == 0 && self->z + CHASE_SLACK < target->z)
    {
        Vec4 landOffset;
        Vec4 landGround;
        landOffset.z += CHASE_PROBE_RAISE;
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        if ((DAT_10db6ee8[model].m_238c != 0 && DAT_1020b430[index].airborne != 0 &&
             self->z + CHASE_SLACK < target->z) ||
            FUN_10024750(index, &landGround, CHASE_GROUND_DEPTH_FAR, 0, &landOffset, 1, 0) == 0)
        {
            int frames = DAT_1020b430[index].airFrames;
            float rise = (float)frames * CHASE_RISE_PER_FRAME;
            float limitedRise = CHASE_RISE_LIMIT > rise ? rise : CHASE_RISE_LIMIT;
            float speed = (CHASE_SLACK - limitedRise) + DAT_10db6ee8[index].motionSpeed;
            DAT_10db6ee8[index].motionSpeed = speed;
            if (speed > CHASE_SPEED_LIMIT)
                DAT_10db6ee8[index].motionSpeed = CHASE_SPEED_LIMIT;
            if (DAT_10db6ee8[index].motionSpeed > 0.0f)
                DAT_10db6ee8[index].m_237c = 0;
            if (DAT_10db6ee8[model].m_238c != 0)
                DAT_1020b430[index].airFrames = frames + 1;
            Vec4 landing;
            landing.Fill(0.0f);
            landing.z = DAT_10db6ee8[index].motionSpeed - CHASE_LAND_SPEED;
            if (landing.z > 0.0f && DAT_10db6ee8[index].IsClearAny(&landing) == 0)
                DAT_10db6ee8[index].matrix._43 += landing.z;
        }
        DAT_11b002c0 = 0;
    }
    if (DAT_1020b430[index].hopTimer > 0)
    {
        DAT_1020b430[index].hopTimer--;
        DAT_10db6ee8[index].AddMotionSpeed(CHASE_HOP_SPEED_UP);
    }
    if ((distance > CHASE_FAR || (DAT_1020b430[index].airborne != 0 && distance > CHASE_NEAR)) &&
        (DAT_10802bf8[index].step - *self).Length2D() < CHASE_STILL_DISTANCE && avoiding == 0)
    {
        if (DAT_10db6ee8[index].motionSpeed < 0.0f || DAT_10db6ee8[index].m_238c != 0 || distance > CHASE_FARTHEST)
            DAT_1020b430[index].stalledFrames++;
        if (DAT_1020b430[index].stalledFrames > CHASE_STALL_LIMIT ||
            (DAT_1020b430[index].airborne != 0 && DAT_1020b430[index].stalledFrames > CHASE_STALL_LIMIT_AIR))
        {
            DAT_1020b430[index].phaseFrames = 0;
            DAT_1020b430[index].state = CHASE_STEP_GIVE_UP;
            DAT_1020b430[index].stalledFrames = 0;
        }
    }
    else if (DAT_10db6ee8[index].m_238c != 0)
    {
        DAT_10802bf8[index].step.Copy(*self);
        DAT_1020b430[index].airFrames = 0;
        DAT_1020b430[index].stalledFrames = 0;
    }
    FUN_100241e0(index, steer.x, steer.y, steer.z);
    int motion;
    if (steer.Length() > CHASE_WALK_SPEED)
        motion = CHASE_MOTION_RUN;
    else if (steer.Length() > CHASE_STAND_SPEED)
        motion = CHASE_MOTION_WALK;
    else
        motion = CHASE_MOTION_STAND;
    if (requestedMotion > -1)
        motion = requestedMotion;
    if (motion > -1)
    {
        if (motion == CHASE_MOTION_SPECIAL)
        {
            DAT_10db6ee8[index].PlayMotion(motion, 0, CHASE_MOTION_BLEND, 1.0f);
            return 0;
        }
        DAT_10db6ee8[index].PlayMotion(motion, 0, CHASE_MOTION_BLEND, 1.0f);
    }
    return 0;
}
