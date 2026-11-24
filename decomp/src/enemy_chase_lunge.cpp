// The common enemy behaviour (0x10076790): chases the player, circles round obstacles and ledges, lunges at him from
// close range, spits now and then and wanders when it cannot get on. See enemy_chase_lunge.h for the steps.

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

#include "world.h"

#include "enemy_chase_lunge.h"

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" float FUN_10001f80(float y, float x);                              // atan2f
extern "C" int FUN_100244b0(int index, float angle, Vec4 *ground, float depth, int fromHead, int testOnly,
                            const Vec4 *offset, const char *name);            // Char_ProbeDirection
extern "C" int FUN_10024c40(const Vec4 &position, Vec4 *direction, float depth, int distance);  // ledge test
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

// 1 when the model of `index` is more than `margin` below the pit floor and inside the pit rectangle (a pit on the map).
static __forceinline int IsInPitArea(int index, float margin)
{
    if (DAT_104bfe24 - margin > DAT_10db6ee8[index].matrix.m[14])
    {
        int y = (int)DAT_10db6ee8[index].matrix.m[13];
        int x = (int)DAT_10db6ee8[index].matrix.m[12];
        if (x >= (int)DAT_104d29e8.x && y >= (int)DAT_104d29e8.y && x <= (int)DAT_104d29f8.x &&
            y <= (int)DAT_104d29f8.y && DAT_104bfe28 != 0)
            return 1;
    }
    return 0;
}

// 0x10076790 (Enemy_ChaseLungeAndSpit). `variant` 1 gives the model a motion speed scale 1.5 times its slot's and
// shortens the pause after a lunge. A character held out of play (hiddenFrames) only counts it down, with its
// colours blanked and no shadow. Otherwise, while the slot is in play (DAT_10210d30 clear, DAT_104483e8 set), the
// first frame looks up its head and spine nodes (an idle NPC turned into an enemy leaves its spot); the
// identification colours come back after a hide; a reaction request (DAT_10210b30) resets the step; the pit area
// (a pit on the map and the model below the floor inside the pit rectangle) sets the model's mode to 2, else 1. A
// dazed enemy (dazeFrames) turns to the player and sparkles. Step 0 (chase): far from the player (or circling) it
// turns to him, probes the ledge ahead, steps along its heading and circles round blocked ways for a random time; a
// blocked way halts it (and with 7 in 10 it wanders, step 2); close to him it lunges (15 in 100 a frame, always
// over the pit; or when `state` counts down from above 200 to 200: motion 4, sound 0x2cc) or stands and, with a
// shotRate, spits effect 0x258 from its head. Step 1 (lunge) steps along the saved direction for 40 frames, hitting
// on frames 4 to 34, and circles round a ledge. Step 2 turns about and walks while the ground ahead is free and
// goes back to step 0 when it is. Over the pit it pushes itself up; after the loop it plays the chosen motion.
// Returns 0.
int Enemy_ChaseLungeAndSpit(int index, int variant)
{
    if (variant != 0)
        DAT_10db6ee8[index].speedScale = DAT_10daaae8[index].speedScale * CHASER_VARIANT_SPEED;
    Matrix4 spitMatrix;
    Vec4 spitPosition;
    Vec4 spitOffset;
    EffectOptions spitOptions;
    int restart = 0;
    int motion = CHASER_NO_MOTION;
    for (int pass = 0; pass < 1; pass++)
    {
        if (DAT_1020b430[index].hiddenFrames > 0)
        {
            DAT_10daaae8[index].m_0141 = 1;
            DAT_1020b430[index].hiddenFrames--;
            DAT_10db6ee8[index].shadowHidden = 1;
            DAT_10db6ee8[index].m_17e8 = 1;
            if (DAT_1020b430[index].colourStep == 0)
            {
                DAT_10db6ee8[index].root->M_101946b0(0, -1);
                DAT_1020b430[index].colourStep = 1;
            }
            break;
        }
        if (DAT_10210d30[index] != 0 || DAT_104483e8[index] == 0)
            break;
        if (DAT_1020b430[index].walkerSetUp == 0)
        {
            char idle = DAT_10db6ee8[index].m_0267;
            DAT_1020b430[index].walkerSetUp = -1;
            if (idle != 0)
            {
                DAT_10db6ee8[index].m_0267 = 0;
                DAT_1020b430[index].behaviour = 1;
                DAT_104c4438[index].x = CHASER_OUT_OF_PLAY;
            }
            DAT_1020b430[index].shotRate = 0;
            DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(CHASER_HEAD_NODE, 0);
            DAT_1021bdf8[index][1] = DAT_10db6ee8[index].root->Find(CHASER_SPINE_NODE, 0);
            DAT_1020b430[index].frameTick = index;
                DAT_1020b430[index].halted = 0;
            }
            DAT_10daaae8[index].m_0141 = 0;
            DAT_10db6ee8[index].m_17e8 = 0;
            if (DAT_1020b430[index].colourStep == 1)
            {
                DAT_10db6ee8[index].root->M_10194630(index, CHASER_BODY_MASK);
                DAT_10db6ee8[index].root->M_101946b0(CHASER_FLASH_COLOUR, CHASER_BODY_MASK);
                DAT_10db6ee8[index].root->M_10194730(CHASER_COLOUR_BLUE, CHASER_BODY_MASK);
                DAT_10db6ee8[index].root->M_10194630(0, CHASER_SHADOW_MASK);
                DAT_10db6ee8[index].root->M_101946b0(CHASER_SHADOW_COLOUR, CHASER_SHADOW_MASK);
                DAT_10db6ee8[index].root->M_10194730(CHASER_COLOUR_BLUE, CHASER_SHADOW_MASK);
                DAT_1020b430[index].colourStep = 2;
                DAT_10db6ee8[index].shadowHidden = 0;
            }
            if (DAT_10210b30[index] != 0)
            {
                DAT_10210b30[index] = 0;
                DAT_1020b430[index].state = CHASER_STEP_CHASE;
                if (DAT_10db6ee8[index].mode != 2)
                    DAT_10db6ee8[index].mode = 1;
            }
            DAT_10db6ee8[index].locked = 0;
            if (IsInPitArea(index, CHASER_PIT_MARGIN))
                DAT_10db6ee8[index].mode = 2;
            else
                DAT_10db6ee8[index].mode = 1;
            if (DAT_1020b430[index].dazeFrames > 0)
            {
                DAT_1020b430[index].dazeFrames--;
                motion = CHASER_MOTION_RUN;
                float dy = DAT_10db6ee8[0].matrix.GetTranslation().y - DAT_10db6ee8[index].matrix.GetTranslation().y;
                float dx = DAT_10db6ee8[0].matrix.GetTranslation().x - DAT_10db6ee8[index].matrix.GetTranslation().x;
                DAT_10db6ee8[index].targetHeading = (float)atan2((double)dx, (double)dy) + CHASER_PI;
                if (DAT_1020b430[index].dazeFrames % CHASER_DAZE_PERIOD == 0)
                {
                    Vec4 position;
                    Matrix4 spark;
                    spark.SetIdentity();
                    position.Fill(0.0f);
                    spark = DAT_10db6ee8[index].matrix;
                    FUN_1001d8b0(index, CHASER_DAZE_EFFECT, position, -1, -1, 0, 0, 0, &spark, 0);
                }
                break;
            }
            DAT_10218fc0[index] = 0;
            Vec4 aim;
            Vec4 self;
            aim.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            (self - aim).Length2D();
            float range = (self - aim).Length();   // first the distance to the player, later the bearing of a lunge or spit
            if (DAT_1020b430[index].state == CHASER_STEP_LUNGE)
            {
                DAT_104483e8[index] = -1;
                aim.Copy(DAT_10802bf8[index].position);
                DAT_10218fc0[index] = 1;
                if (DAT_1020b430[index].wait < CHASER_LUNGE_SLOW_BEFORE ||
                    DAT_1020b430[index].wait > CHASER_LUNGE_SLOW_AFTER)
                {
                    aim.Copy(aim * CHASER_LUNGE_CREEP);
                    if (DAT_1020b430[index].wait == CHASER_LUNGE_START_FRAME)
                    {
                        DAT_10868838[index][0].m[12] = self.x;
                        DAT_10868838[index][0].m[13] = self.y;
                        DAT_10868838[index][0].m[14] = self.z;
                    }
                }
                if (DAT_1020b430[index].wait == 1)
                {
                    Matrix4 rotation;
                    rotation = RotationZ(DAT_10db6ee8[index].heading);
                    EffectOptions options;
                    options.owner = index;
                    FUN_1001d8b0(index, CHASER_LUNGE_EFFECT, spitOffset, -1, -1, &DAT_1021bdf8[index][1]->worldMatrix, 0,
                                 0, &rotation, &options);
                }
                if (DAT_1020b430[index].wait > CHASER_LUNGE_HIT_FROM && DAT_1020b430[index].wait < CHASER_LUNGE_HIT_TO)
                    DAT_1021f6c0[index] = 1;
                DAT_10db6ee8[index].targetHeading = (float)atan2((double)aim.x, (double)aim.y) + CHASER_PI;
                if (DAT_104bfd08 == 0)
                {
                    int clear = 0;
                    if (DAT_1020b430[index].frameTick % CHASER_PROBE_THIRD == index % CHASER_PROBE_THIRD)
                    {
                        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
                        if (FUN_10024c40(DAT_10db6ee8[index].matrix.GetTranslation(), &aim, CHASER_SIGHT_DEPTH,
                                         CHASER_SIGHT_REACH) == 0)
                        {
                            clear = 1;
                            DAT_1020b430[index].edgeAhead = 1;
                            DAT_11b002c0 = 0;
                        }
                        else
                        {
                            DAT_1020b430[index].edgeAhead = 0;
                            DAT_11b002c0 = 0;
                        }
                    }
                    else
                        clear = DAT_1020b430[index].edgeAhead;
                    if (clear != 0)
                    {
                        if (DAT_1020b430[index].circleDirection == 0)
                            DAT_1020b430[index].circleDirection =
                                FUN_1013f880() % 2 != 0 ? CHASER_CIRCLE_SIDE_FRAMES : -CHASER_CIRCLE_SIDE_FRAMES;
                        if (DAT_1020b430[index].circleDirection > 0)
                        {
                            DAT_1020b430[index].circleDirection--;
                            DAT_10802bf8[index].position.RotateZ(CHASER_CIRCLE_TURN);
                            aim.RotateZ(CHASER_CIRCLE_TURN);
                        }
                        else
                        {
                            DAT_1020b430[index].circleDirection++;
                            DAT_10802bf8[index].position.RotateZ(-CHASER_CIRCLE_TURN);
                            aim.RotateZ(-CHASER_CIRCLE_TURN);
                        }
                    }
                    if (DAT_10db6ee8[index].mode == 2)
                        FUN_100241e0(index, aim.x, aim.y, CHASER_PIT_STEP);
                    else
                        FUN_100241e0(index, aim.x, aim.y, 0.0f);
                }
                DAT_10db6ee8[index].move.x = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.x;
                DAT_1020b430[index].wait++;
                DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.y;
                DAT_10db6ee8[index].move.z = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.z;
                if (DAT_1020b430[index].wait > CHASER_LUNGE_FRAMES)
                {
                    DAT_10254390[index] = variant != 0 ? CHASER_LUNGE_DELAY_VARIANT : CHASER_LUNGE_DELAY;
                    motion = CHASER_MOTION_IDLE;
                    DAT_1020b430[index].state = CHASER_STEP_CHASE;
                    if ((DAT_10868838[index][0].GetTranslation() - self).Length2D() <
                        DAT_10db6ee8[index].speedScale * CHASER_LUNGE_CIRCLE_RANGE)
                        DAT_1020b430[index].circleFrames = CHASER_CIRCLE_FRAMES;
                }
            }
            else if (DAT_1020b430[index].state == CHASER_STEP_WANDER)
            {
                Vec4 step;
                if (DAT_1020b430[index].wait == 0)
                {
                    DAT_10db6ee8[index].targetHeading =
                        ((float)(FUN_1013f880() % CHASER_WANDER_START_BEARING) - 90.0f) / 180.0f * CHASER_PI_LONG +
                        DAT_10db6ee8[index].targetHeading;
                }
                else if (DAT_1020b430[index].wait < CHASER_WANDER_STEPS)
                {
                    step.y = CHASER_SIGHT_STEP;
                    step.RotateZ(-DAT_10db6ee8[index].heading);
                    int blocked = 0;
                    if (DAT_1020b430[index].frameTick % CHASER_PROBE_TENTH == index % CHASER_PROBE_TENTH)
                    {
                        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
                        if (FUN_10024c40(DAT_10db6ee8[index].matrix.GetTranslation(), &step, CHASER_SIGHT_DEPTH,
                                         CHASER_SIGHT_REACH) == 0)
                        {
                            blocked = 1;
                            DAT_1020b430[index].edgeAhead = 1;
                            DAT_11b002c0 = 0;
                        }
                        else
                        {
                            DAT_1020b430[index].edgeAhead = 0;
                            DAT_11b002c0 = 0;
                        }
                    }
                    else if (DAT_1020b430[index].edgeAhead != 0)
                        blocked = 1;
                    step.z = CHASER_STEP_HEIGHT;
                    if (blocked == 0 && DAT_10db6ee8[index].IsClearOfMapLimited(&step) != 0 &&
                        DAT_10db6ee8[index].IsClearAny(&step) != 0)
                    {
                        step.z = 0.0f;
                        if (DAT_104bfd08 == 0)
                            FUN_100241e0(index, step.x, step.y, 0.0f);
                        step.z = 0.0f;
                        motion = CHASER_MOTION_WALK;
                        DAT_10db6ee8[index].move.x = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.x;
                        DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.y;
                        DAT_10db6ee8[index].move.z = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.z;
                    }
                    else
                    {
                        DAT_1020b430[index].wait = CHASER_WANDER_RESUME;
                        DAT_10db6ee8[index].targetHeading =
                            ((float)(FUN_1013f880() % CHASER_WANDER_ROLL - 10) / 180.0f + 1.0f) * CHASER_PI +
                            DAT_10db6ee8[index].targetHeading;
                        step.z = 0.0f;
                        motion = CHASER_MOTION_WALK;
                    }
                }
                else if (DAT_1020b430[index].wait < CHASER_WANDER_END)
                    motion = CHASER_MOTION_IDLE;
                else
                    DAT_1020b430[index].wait = -1;
                DAT_1020b430[index].wait++;
                aim.Copy(aim - self);
                DAT_1020b430[index].frameTick++;
                float turn = CHASER_PI - FUN_10001f80(aim.x, aim.y);
                DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
                if (DAT_1020b430[index].wait < CHASER_WANDER_WALK_AFTER ||
                    (DAT_1020b430[index].halted != 0 && DAT_1020b430[index].frameTick % CHASER_RETEST_PERIOD != 0))
                {
                    DAT_11b002c0 = 0;
                    if (DAT_1020b430[index].frameTick % CHASER_RETEST_PERIOD == 0)
                        DAT_1020b430[index].frameTick = 0;
                    break;
                }
                if (DAT_1020b430[index].frameTick % CHASER_RETEST_PERIOD == 0)
                {
                    if (FUN_100244b0(index, turn, 0, aim.Length(), 1, 1, 0, 0) != 0)
                    {
                        DAT_11b002c0 = 0;
                        if (DAT_1020b430[index].frameTick % CHASER_RETEST_PERIOD == 0)
                            DAT_1020b430[index].frameTick = 0;
                        break;
                    }
                }
                Vec4 toward;
                toward.Copy(DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[index].matrix.GetTranslation());
                float bearing = FUN_10001f80(toward.x, toward.y) + CHASER_PI;
                toward.Fill(0.0f);
                toward.y = CHASER_SIGHT_STEP;
                toward.RotateZ(-bearing);
                DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
                int shut = 0;
                if (FUN_10024c40(DAT_10db6ee8[index].matrix.GetTranslation(), &toward, CHASER_SIGHT_DEPTH,
                                 CHASER_SIGHT_REACH) == 0)
                    shut = 1;
                DAT_1020b430[index].edgeAhead = 0;
                if (shut == 0)
                {
                    DAT_1020b430[index].halted = 0;
                    DAT_1020b430[index].state = CHASER_STEP_CHASE;
                    DAT_1020b430[index].wait = 0;
                }
                DAT_11b002c0 = 0;
            }
        else if (DAT_1020b430[index].circleFrames != 0 ||
                 (range > CHASER_NEAR_RANGE && DAT_1020b430[index].state < CHASER_LUNGE_COOLDOWN))
        {
            Vec4 toPlayer;
            toPlayer.Copy(aim - self);
            DAT_1020b430[index].frameTick++;
            float turn = CHASER_PI - FUN_10001f80(toPlayer.x, toPlayer.y);
            DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
            if (DAT_1020b430[index].frameTick % CHASER_PROBE_TENTH == index % CHASER_PROBE_TENTH)
            {
                Vec4 sight;
                sight.y = CHASER_SIGHT_STEP;
                if (DAT_1020b430[index].circleDirection > 0)
                    sight.RotateZ(-DAT_10db6ee8[index].targetHeading - CHASER_QUARTER_TURN);
                else if (DAT_1020b430[index].circleDirection < 0)
                    sight.RotateZ(CHASER_QUARTER_TURN - DAT_10db6ee8[index].targetHeading);
                else
                    sight.RotateZ(-DAT_10db6ee8[index].targetHeading);
                if (FUN_10024c40(DAT_10db6ee8[index].matrix.GetTranslation(), &sight, CHASER_SIGHT_DEPTH,
                                 CHASER_SIGHT_REACH) == 0)
                {
                    DAT_1020b430[index].edgeAhead = 1;
                    goto halt;
                }
                DAT_1020b430[index].edgeAhead = 0;
            }
            else if (DAT_1020b430[index].edgeAhead != 0)
                goto halt;
            if (DAT_1020b430[index].halted != 0 && DAT_1020b430[index].frameTick % CHASER_RETEST_PERIOD != 0)
                goto halt;
            if (DAT_1020b430[index].frameTick % CHASER_RETEST_PERIOD == 0 &&
                FUN_100244b0(index, turn, 0, toPlayer.Length(), 1, 1, 0, 0) != 0)
                goto halt;
            DAT_1020b430[index].halted = 0;
            DAT_11b002c0 = 0;
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + CHASER_PI;
            aim.Fill(0.0f);
            aim.y = CHASER_SIGHT_STEP;
            aim.RotateZ(-DAT_10db6ee8[index].targetHeading);
            aim.z = CHASER_STEP_HEIGHT;
            if (DAT_1020b430[index].circleFrames != 0 || DAT_10db6ee8[index].IsClearOfMapLimited(&aim) == 0 ||
                DAT_10db6ee8[index].IsClearAny(&aim) == 0)
            {
                aim.Fill(0.0f);
                DAT_1020b430[index].circleFrames--;
                if (DAT_1020b430[index].circleFrames < 0)
                    DAT_1020b430[index].circleFrames = CHASER_CIRCLE_FRAMES;
                if (DAT_1020b430[index].circleDirection != 0)
                {
                    if (DAT_1020b430[index].circleDirection > 0)
                        DAT_1020b430[index].circleDirection--;
                    else
                        DAT_1020b430[index].circleDirection++;
                }
                if (DAT_1020b430[index].circleDirection == 0)
                {
                    DAT_1020b430[index].circleDirection = FUN_1013f880() % CHASER_SIDE_ROLL + CHASER_SIDE_MIN;
                    if (FUN_1013f880() % 2 != 0)
                        DAT_1020b430[index].circleDirection = -DAT_1020b430[index].circleDirection;
                }
                if (DAT_1020b430[index].circleDirection > 0)
                {
                    DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading + CHASER_QUARTER_TURN;
                    aim.Fill(0.0f);
                }
                else
                {
                    DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading - CHASER_QUARTER_TURN;
                    aim.Fill(0.0f);
                }
                aim.y = CHASER_SIGHT_STEP;
                aim.RotateZ(-DAT_10db6ee8[index].targetHeading);
                aim.z = CHASER_STEP_HEIGHT;
                if (FUN_1013f880() % CHASER_FLIP_ROLL < CHASER_FLIP_LIMIT &&
                    (DAT_10db6ee8[index].IsClearOfMapLimited(&aim) == 0 ||
                     DAT_10db6ee8[index].IsClearAny(&aim) == 0))
                    DAT_1020b430[index].circleDirection = -DAT_1020b430[index].circleDirection;
                aim.z = 0.0f;
            }
            if (DAT_104bfd08 == 0)
            {
                if (DAT_10db6ee8[index].mode == 2)
                    FUN_100241e0(index, aim.x, aim.y, CHASER_PIT_STEP);
                else
                    FUN_100241e0(index, aim.x, aim.y, 0.0f);
            }
            motion = CHASER_MOTION_WALK;
            DAT_10db6ee8[index].move.x = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.x;
            DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.y;
            DAT_10db6ee8[index].move.z = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.z;
            continue;
        halt:
            DAT_11b002c0 = 0;
            if (DAT_1020b430[index].frameTick % CHASER_RETEST_PERIOD == 0)
                DAT_1020b430[index].frameTick = 0;
            DAT_1020b430[index].halted = 1;
            motion = CHASER_MOTION_IDLE;
            if (FUN_1013f880() % CHASER_GIVE_UP_ROLL < CHASER_GIVE_UP_LIMIT)
            {
                DAT_1020b430[index].state = CHASER_STEP_WANDER;
                DAT_1020b430[index].wait = 0;
            }
            break;
        }
        else
        {
            if ((FUN_1013f880() % 100 < CHASER_LUNGE_ROLL || DAT_10db6ee8[index].mode == 2) &&
                DAT_1020b430[index].state == CHASER_STEP_CHASE)
            {
                DAT_104483e8[index] = -1;
                DAT_1020b430[index].state = CHASER_STEP_LUNGE;
                DAT_1020b430[index].wait = 0;
                aim.Copy(aim - self);
                aim.Copy(Vec3_Normalized(&aim));
                aim.Copy(aim * CHASER_LUNGE_STEP * CHASER_LUNGE_STEP_LONG);
                range = (float)atan2((double)aim.x, (double)aim.y);
                DAT_10db6ee8[index].targetHeading = range + CHASER_PI;
                DAT_10802bf8[index].position.Copy(aim);
                motion = CHASER_MOTION_LUNGE;
                restart = 1;
                FUN_10065b10(index, CHASER_LUNGE_SOUND, 0, 0);
                if (DAT_10db6ee8[index].mode == 2)
                {
                    DAT_10db6ee8[index].verticalSpeed = CHASER_LUNGE_PIT_JUMP;
                    DAT_10db6ee8[index].m_237c = 0;
                    DAT_10db6ee8[index].mode = 1;
                    DAT_10db6ee8[index].m_238c = 0;
                }
            }
            else if (DAT_1020b430[index].state > CHASER_LUNGE_COOLDOWN)
            {
                DAT_1020b430[index].state--;
                if (DAT_1020b430[index].state == CHASER_LUNGE_COOLDOWN)
                {
                    DAT_104483e8[index] = -1;
                    DAT_1020b430[index].state = CHASER_STEP_LUNGE;
                    DAT_1020b430[index].wait = 0;
                    aim.Copy(aim - self);
                    aim.Copy(Vec3_Normalized(&aim));
                    aim.Copy(aim * CHASER_LUNGE_STEP * CHASER_LUNGE_STEP_LONG);
                    range = (float)atan2((double)aim.x, (double)aim.y);
                    DAT_10db6ee8[index].targetHeading = range + CHASER_PI;
                    DAT_10802bf8[index].position.Copy(aim);
                    motion = CHASER_MOTION_LUNGE;
                    restart = 1;
                    FUN_10065b10(index, CHASER_LUNGE_SOUND, 0, 0);
                    if (DAT_10db6ee8[index].mode == 2)
                    {
                        DAT_10db6ee8[index].verticalSpeed = CHASER_LUNGE_PIT_JUMP;
                        DAT_10db6ee8[index].m_237c = 0;
                        DAT_10db6ee8[index].mode = 1;
                        DAT_10db6ee8[index].m_238c = 0;
                    }
                }
            }
            else
            {
                aim.Copy(aim - self);
                aim.Copy(Vec3_Normalized(&aim));
                aim.Copy(aim * CHASER_SPIT_AIM_STEP);
                range = (float)atan2((double)aim.x, (double)aim.y);
                motion = CHASER_MOTION_IDLE;
                DAT_10db6ee8[index].targetHeading = range + CHASER_PI;
                if (DAT_1020b430[index].shotRate > 0 &&
                    (CHASER_SPIT_LEVELS - DAT_1020b430[index].shotRate) * CHASER_SPIT_PERIOD <
                        DAT_1020b430[index].volleyTimer &&
                    DAT_10db6ee8[index].mode != 2)
                {
                    spitMatrix.SetIdentity();
                    spitOffset.y = CHASER_SPIT_FORWARD;
                    spitOffset.RotateZ(CHASER_PI - DAT_10db6ee8[index].heading);
                    spitPosition.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() + spitOffset);
                    spitOffset.Fill(0.0f);
                    spitMatrix = RotationZ(DAT_10db6ee8[index].heading);
                    spitMatrix.SetTranslation(spitPosition);
                    spitOptions.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    spitOptions.m_0000 = 0;
                    FUN_1001d8b0(index, CHASER_SPIT_EFFECT, spitOffset, -1, -1, 0, 0, 0, &spitMatrix, &spitOptions);
                    FUN_10065b10(index, CHASER_SPIT_SOUND, 0, 0);
                    DAT_1020b430[index].volleyTimer = 0;
                }
                DAT_1020b430[index].volleyTimer++;
            }
        }
    }
    if (DAT_10db6ee8[index].mode == 2)
    {
        Vec4 probe;
        probe.z = CHASER_PIT_PROBE;
        if (DAT_10db6ee8[index].IsClearOfMapLimited(&probe) != 0)
        {
            DAT_10db6ee8[index].move.z =
                DAT_10db6ee8[index].move.z < CHASER_PIT_RISE ? CHASER_PIT_RISE : DAT_10db6ee8[index].move.z;
            DAT_1020b430[index].overPit = 1;
        }
        else
        {
            DAT_10db6ee8[index].move.z = CHASER_PIT_LAND;
            DAT_1020b430[index].overPit = 1;
        }
    }
    else if (DAT_1020b430[index].overPit != 0)
    {
        if (IsInPitArea(index, CHASER_PIT_LEAVE_MARGIN))
            DAT_10db6ee8[index].mode = 0;
        else
            DAT_1020b430[index].overPit = 0;
    }
    if (motion > CHASER_NO_MOTION)
        DAT_10db6ee8[index].PlayMotion(motion, restart, CHASER_MOTION_BLEND, 1.0f);
    return 0;
}
