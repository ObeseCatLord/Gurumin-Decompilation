// The map 0x18 football game's CPU player. C++ rather than extern "C": the function keeps an exception frame for its
// Matrix4 local.

#include <math.h>
#include "vec4.h"
#include "matrix4.h"
#include "anim_model.h"
#include "node.h"
#include "char_state.h"
#include "last_position.h"
#include "char_vars.h"
#include "char_hikari.h"
#include "map_vars.h"
#include "ui_vars.h"
#include "soccer_ai.h"

#define SOCCER_PI 3.1415f
#define SOCCER_TWO_PI 6.283f
#define SOCCER_GOAL_Y 1200.0f              // the goals lie at y = +-1200
#define SOCCER_AIM_SHORT 100.0f            // the chaser aims this far short of the goal line
#define SOCCER_CHASE_BALL_Y 300.0f         // the chaser only starts once the ball is this far into the other half
#define SOCCER_CHASE_RANGE 30.0f           // the chaser walks to its post until the ball is within this distance
#define SOCCER_FAR_STEP 16.0f              // longest step while walking to the post
#define SOCCER_NEAR_BALL 400.0f            // the player turns to face the ball within this distance
#define SOCCER_PURSUE_NEAR 750.0f          // the chase starts within this distance of the ball ...
#define SOCCER_PURSUE_FAR 900.0f           // ... or within this when the ball is on the far side
#define SOCCER_PURSUE_STEP 12.0f           // longest step of the chase
#define SOCCER_BACK_STEP 13.0f             // longest step towards the ball for the back and forward players
#define SOCCER_POST_STEP 12.0f             // longest step towards the post
#define SOCCER_JUMP_REACH 300.0f           // jumps start for balls closer than this ...
#define SOCCER_JUMP_HEIGHT 150.0f          // ... and this far above the player
#define SOCCER_JUMP_SPEED 0x1e
#define SOCCER_JUMP_COOLDOWN 0x78
#define SOCCER_GRAVITY 2
#define SOCCER_IDLE_DELAY 15
#define SOCCER_NET_SLIDE_FRAMES 0x5a
#define SOCCER_SIDESTEP_FRAMES 15
#define SOCCER_SIDESTEP_SWITCH 30
#define SOCCER_ROLE_HOLD_Y 250.0f          // the posts are only held while the ball is this far into the other half
#define SOCCER_SIDESTEP_ANGLE 1.0f
#define SOCCER_TURN_DONE 0.05236f          // 3 degrees
#define SOCCER_ROLE_PICK_BALL_Y -900.0f
#define SOCCER_PROBE_LIFT 5.0f

// True when `p` lies in the zone the team's players keep to (rows 2*team and 2*team + 1 of the pitch, doubled).
static inline bool InTeamZone(const Vec4 &p, int team)
{
    return (float)(DAT_10211000[team * 2].x << 1) < p.x && p.x < (float)(DAT_10211000[team * 2 + 1].x << 1) &&
           (float)(DAT_10211000[team * 2].code << 1) < p.y && p.y < (float)(DAT_10211000[team * 2 + 1].code << 1);
}

// Links the player `index` and its team mate `mate` in each other's role tables (who holds which role).
static __forceinline void LinkRoleMate(int index, int mate)
{
    SoccerState &me = DAT_1020b430[index].soccer;
    SoccerState &other = DAT_1020b430[mate].soccer;
    me.roleOwner[0] = 0;
    me.roleOwner[1] = 0;
    me.roleOwner[2] = 0;
    other.roleOwner[0] = 0;
    other.roleOwner[1] = 0;
    other.roleOwner[2] = 0;
    me.roleOwner[me.role - 1] = index;
    if (other.role != 0)
        me.roleOwner[other.role - 1] = mate;
}

// 0x10032240 (SoccerGame_UpdateCpuPlayer). One frame of the CPU football player in character slot `index` of team
// `team`. It picks a role the first frame (1 = chases the ball, 2/3 = hold the back and forward posts until the ball
// comes close) and swaps roles with its team mates by their distance to the goal. The chaser walks to the ball and
// jumps for high ones; the other roles go to their post or follow the ball. The step is then bent round the goal nets
// and round other players (`clearance` extra distance), scaled by the difficulty and applied to the player's
// position (pushed out of walls by the map collision). Returns 0.
int SoccerGame_UpdateCpuPlayer(int index, int team, int mateA, int mateB, int clearance)
{
    Vec4 pos;
    Vec4 ball;
    Vec4 step;
    Vec4 dir;
    Vec4 goal;
    int arrived = 0;

    goal.x = 0.0f;
    goal.y = SOCCER_GOAL_Y;
    goal.z = 0.0f;
    if (team == 0)
        goal.y = -SOCCER_GOAL_Y;
    DAT_10db6ee8[index].mode = 0;
    pos.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    ball.Copy(DAT_10e40204.GetTranslation());
    SoccerState &me = DAT_1020b430[index].soccer;

    if (me.role == 0)
    {
        int roleA = 0;
        int roleB = 0;
        if (mateA != 0 && DAT_1020b430[mateA].soccer.role != 0)
            roleA = DAT_1020b430[mateA].soccer.role;
        if (mateB != 0 && DAT_1020b430[mateB].soccer.role != 0)
            roleB = DAT_1020b430[mateB].soccer.role;
        do
        {
            me.role = FUN_1013f880() % 3 + 1;
        } while (roleA == me.role || roleB == me.role);
        step.Fill(0.0f);
        if (team == 0)
        {
            if (mateA != 0)
                LinkRoleMate(index, mateA);
            if (mateB != 0)
                LinkRoleMate(index, mateB);
        }
    }
    if (team == 0)
    {
        if (me.role == 0)
            return 0;
        if (mateA != 0 && DAT_1020b430[mateA].soccer.role == 0)
            return 0;
        if (mateB != 0 && DAT_1020b430[mateB].soccer.role == 0)
            return 0;
    }

    if (me.role == SOCCER_ROLE_CHASER)
    {
        step.Copy(goal);
        step.z = 0.0f;
        if (step.y < 0.0f)
        {
            step.y += SOCCER_AIM_SHORT;
            if (ball.y < -SOCCER_CHASE_BALL_Y && InTeamZone(pos, team))
                me.pursuing = 1;
        }
        else
        {
            step.y -= SOCCER_AIM_SHORT;
            if (SOCCER_CHASE_BALL_Y < ball.y && InTeamZone(pos, team))
                me.pursuing = 1;
        }
        step.Copy(step - pos);
        step.z = 0.0f;
        if (DAT_1020c2d0 < 0)
            me.pursuing = 0;
        if (me.pursuing == 0 && step.Length2D() > SOCCER_CHASE_RANGE)
        {
            if (step.Length2D() > SOCCER_FAR_STEP)
            {
                step.Normalized3();
                step.Copy(step * SOCCER_FAR_STEP);
            }
            DAT_10db6ee8[index].targetHeading = atan2f(step.x, step.y) + SOCCER_PI;
            DAT_10db6ee8[index].PlayMotion(SOCCER_MOTION_RUN, 0, 5, 1.0f);
            if ((pos + step - ball).Length2D() < SOCCER_NEAR_BALL)
            {
                dir.Copy(ball - pos);
                dir.z = 0.0f;
                DAT_10db6ee8[index].targetHeading = atan2f(dir.x, dir.y) + SOCCER_PI;
            }
        }
        else if (DAT_1020c2d0 < 0)
        {
            DAT_10db6ee8[index].PlayMotion(SOCCER_MOTION_IDLE, 0, 5, 1.0f);
            dir.Copy(ball - pos);
            dir.z = 0.0f;
            float angle = (float)atan2(dir.x, dir.y);
            angle = angle + SOCCER_PI;
            DAT_10db6ee8[index].targetHeading = angle;
            float turn = angle - DAT_10db6ee8[index].heading;
            while (turn > SOCCER_PI)
                turn = turn - SOCCER_TWO_PI;
            while (turn < -SOCCER_PI)
                turn = turn + SOCCER_TWO_PI;
            float whole = (float)floor(turn);
            if (whole > 0.01f)
                DAT_10db6ee8[index].PlayMotion(SOCCER_MOTION_WALK, 0, 5, 1.0f);
        }
        else if (me.pursuing == 0)
        {
            if ((pos - ball).Length2D() < SOCCER_PURSUE_FAR && step.y > 0.0f || (pos - ball).Length2D() < SOCCER_PURSUE_NEAR)
            {
                me.pursuing = 1;
                step.Fill(0.0f);
            }
        }
        else if (me.pursuing == 1)
        {
            dir.Copy(ball - pos);
            step.Copy(dir);
            step.Normalized3();
            float stepLength;
            if ((ball - pos).Length2D() < SOCCER_PURSUE_STEP)
                stepLength = (ball - pos).Length2D();
            else
                stepLength = SOCCER_PURSUE_STEP;
            step.Copy(step * stepLength);
            if (step.x > dir.x && dir.x > 0.0f || step.x < dir.x && dir.x <= 0.0f)
                step.x = dir.x;
            if (step.y > dir.y && dir.y > 0.0f || step.y < dir.y && dir.y <= 0.0f)
                step.y = dir.y;
            float nextX = step.x + pos.x;
            float lowX = (float)(DAT_10211000[team * 2].x << 1);
            if (nextX < lowX)
                step.x = lowX - pos.x;
            else if ((float)(DAT_10211000[team * 2 + 1].x << 1) < nextX)
                step.x = (float)(DAT_10211000[team * 2 + 1].x << 1) - pos.x;
            float nextY = pos.y + step.y;
            float lowY = (float)DAT_10211000[team * 2].code;
            if (nextY < lowY)
                step.y = lowY - pos.y;
            else if ((float)DAT_10211000[team * 2 + 1].code < nextY)
                step.y = (float)DAT_10211000[team * 2 + 1].code - pos.y;
            float angle = atan2f(dir.x, dir.y) + SOCCER_PI;
            DAT_10db6ee8[index].targetHeading = angle;
            float turn = DAT_10db6ee8[index].heading - angle;
            while (turn < -SOCCER_PI)
                turn = turn + SOCCER_TWO_PI;
            while (turn > SOCCER_PI)
                turn = turn - SOCCER_TWO_PI;
            if (step.Length2D() < 1.0f && floorf(turn) < SOCCER_TURN_DONE)
            {
                if (me.idleDelay == 0)
                    DAT_10db6ee8[index].PlayMotion(SOCCER_MOTION_IDLE, 0, 5, 1.0f);
                if (me.idleDelay > 0)
                    me.idleDelay--;
                step.Fill(0.0f);
            }
            else
            {
                DAT_10db6ee8[index].PlayMotion(step.Length2D() < 5.0f ? SOCCER_MOTION_WALK : SOCCER_MOTION_RUN, 0, 5, 1.0f);
                me.idleDelay = SOCCER_IDLE_DELAY;
            }
            if ((pos - ball).Length2D() < SOCCER_JUMP_REACH && pos.z < ball.z - SOCCER_JUMP_HEIGHT &&
                DAT_10db6ee8[index].m_238c != 0 && me.jumpCooldown == 0)
            {
                if (DAT_10803ac0 >= 4.0f || (pos - ball).Length2D() <= 50.0f)
                {
                    DAT_10db6ee8[index].m_238c = 0;
                    me.jumpSpeed = SOCCER_JUMP_SPEED;
                    me.jumpCooldown = SOCCER_JUMP_COOLDOWN;
                }
            }
            if ((pos - ball).Length2D() > SOCCER_PURSUE_NEAR ||
                (pos - ball).Length2D() > SOCCER_PURSUE_FAR && ball.y > 0.0f)
                me.pursuing = 0;
        }
        else
        {
            dir.Copy(ball - pos);
            float angle = atan2f(dir.x, dir.y) + SOCCER_PI;
            DAT_10db6ee8[index].targetHeading = angle;
            float turn = DAT_10db6ee8[index].heading - angle;
            while (turn < -SOCCER_PI)
                turn = turn + SOCCER_TWO_PI;
            while (turn > SOCCER_PI)
                turn = turn - SOCCER_TWO_PI;
            if (floorf(turn) >= SOCCER_TURN_DONE)
            {
                me.idleDelay = SOCCER_IDLE_DELAY;
                DAT_10db6ee8[index].PlayMotion(SOCCER_MOTION_WALK, 0, 5, 1.0f);
            }
            else
            {
                if (me.idleDelay == 0)
                    DAT_10db6ee8[index].PlayMotion(SOCCER_MOTION_IDLE, 0, 5, 1.0f);
                if (me.idleDelay > 0)
                    me.idleDelay--;
            }
            step.Fill(0.0f);
        }
    }
    else if (me.role == SOCCER_ROLE_BACK || me.role == SOCCER_ROLE_FORWARD)
    {
        int mode = 1;
        if (me.role == SOCCER_ROLE_FORWARD)
        {
            if (team == 0)
            {
                if (-SOCCER_ROLE_HOLD_Y < ball.y)
                    mode = 1;
                else
                {
                    dir.y = (float)DAT_10211000[8].y;
                    dir.x = 0.0f;
                    mode = 2;
                }
            }
            else if (ball.y < SOCCER_ROLE_HOLD_Y)
                mode = 1;
            else
            {
                dir.y = (float)-DAT_10211000[8].y;
                dir.x = 0.0f;
                mode = 2;
            }
        }
        else if (team == 0)
        {
            if (ball.y < SOCCER_ROLE_HOLD_Y)
                mode = 1;
            else
            {
                dir.y = (float)DAT_10211000[8].x;
                dir.x = 0.0f;
                mode = 2;
            }
        }
        else if (-SOCCER_ROLE_HOLD_Y < ball.y)
            mode = 1;
        else
        {
            dir.y = (float)-DAT_10211000[8].x;
            dir.x = 0.0f;
            mode = 2;
        }

        if (DAT_1020c2d0 >= 0 && mode == 1)
        {
            step.Copy(ball - pos);
            step.z = 0.0f;
            step.Normalized3();
            float stepLength;
            if ((ball - pos).Length2D() < SOCCER_BACK_STEP)
                stepLength = (ball - pos).Length2D();
            else
                stepLength = SOCCER_BACK_STEP;
            step.Copy(step * stepLength);
            if ((dir - pos).Length2D() < 36.0f)
                arrived = 1;
            if (step.x > 0.0f && ball.x < step.x + pos.x || step.x < 0.0f && step.x + pos.x < ball.x)
                step.x = ball.x - pos.x;
            if (step.y > 0.0f && ball.y < pos.y + step.y || step.y < 0.0f && pos.y + step.y < ball.y)
                step.y = ball.y - pos.y;
            DAT_10db6ee8[index].targetHeading = (float)atan2(step.x, step.y) + SOCCER_PI;
        }
        else
        {
            step.Copy(dir - pos);
            step.z = 0.0f;
            step.Normalized3();
            float stepLength;
            if ((dir - pos).Length2D() < SOCCER_POST_STEP)
                stepLength = (dir - pos).Length2D();
            else
                stepLength = SOCCER_POST_STEP;
            step.Copy(step * stepLength);
            if ((dir - pos).Length2D() < 24.0f)
                arrived = 1;
            if (step.Length2D() < 2.0f)
            {
                if (team == 0 && me.role == SOCCER_ROLE_BACK || team != 0 && me.role == SOCCER_ROLE_FORWARD)
                    DAT_10db6ee8[index].targetHeading = SOCCER_PI;
                else
                    DAT_10db6ee8[index].targetHeading = 0.0f;
                step.Fill(0.0f);
            }
            else
                DAT_10db6ee8[index].targetHeading = atan2f(step.x, step.y) + SOCCER_PI;
        }

        float margin = (float)clearance;
        float nextX = step.x + pos.x;
        float nextY = pos.y + step.y - margin;
        if (nextX - margin < (float)DAT_10211000[1].x && margin + step.x + pos.x > (float)DAT_10211000[0].x &&
            (nextY < (float)DAT_10211000[1].y && margin + pos.y + step.y > (float)DAT_10211000[0].y ||
             nextY < (float)DAT_10211000[3].y && margin + pos.y + step.y > (float)DAT_10211000[2].y))
        {
            dir.Fill(0.0f);
            if (margin + pos.x < (float)DAT_10211000[0].x || (float)DAT_10211000[1].x < pos.x - margin)
            {
                dir.y = step.Length2D();
                if (step.y < 0.0f)
                    dir.y *= -1.0f;
                int side = me.netSide;
                if (side == 0 || side > 2)
                {
                    if (dir.y <= 0.0f)
                        me.netSide = 2;
                    else
                        me.netSide = 1;
                }
                else
                {
                    if (dir.y < 0.0f && side == 1 || dir.y > 0.0f && side == 2)
                        dir.y *= -1.0f;
                    if (++me.netFrames > SOCCER_NET_SLIDE_FRAMES)
                    {
                        me.netFrames = 0;
                        me.netSide = (side == 1) + 1;
                    }
                }
            }
            else
            {
                dir.x = step.Length2D();
                if (step.x < 0.0f)
                    dir.x *= -1.0f;
                int side = me.netSide;
                if (side < 3)
                {
                    if (dir.x <= 0.0f)
                        me.netSide = 4;
                    else
                        me.netSide = 3;
                }
                else
                {
                    if (dir.x < 0.0f && side == 3 || dir.x > 0.0f && side == 4)
                        dir.x *= -1.0f;
                    if (++me.netFrames > SOCCER_NET_SLIDE_FRAMES)
                    {
                        me.netFrames = 0;
                        me.netSide = (side == 3) + 3;
                    }
                }
            }
            step.Copy(dir);
            step.z = 0.0f;
            DAT_10db6ee8[index].targetHeading = atan2f(step.x, step.y) + SOCCER_PI;
        }
        else
        {
            me.netSide = 0;
            me.netFrames = 0;
        }
        DAT_10db6ee8[index].PlayMotion(step.Length2D() < 2.0f ? SOCCER_MOTION_IDLE
                                                               : (step.Length2D() < 8.0f ? SOCCER_MOTION_WALK : SOCCER_MOTION_RUN),
                                       0, 5, 1.0f);
    }

    if (goal.y < 0.0f && -SOCCER_CHASE_BALL_Y < ball.y)
    {
        Vec4 toGoal;
        Vec4 mateAToGoal;
        Vec4 mateBToGoal;
        toGoal.Copy(goal - DAT_10db6ee8[index].matrix.GetTranslation());
        mateAToGoal.Copy(goal - DAT_10db6ee8[mateA].matrix.GetTranslation());
        mateBToGoal.Copy(goal - DAT_10db6ee8[mateB].matrix.GetTranslation());
        int other = mateA;
        if (other == 0)
            other = mateB;
        if (me.role == SOCCER_ROLE_CHASER)
        {
            int closest = -1;
            if (toGoal.Length2D() > mateAToGoal.Length2D())
            {
                closest = mateA;
                if (mateAToGoal.Length2D() > mateBToGoal.Length2D())
                    closest = mateB;
            }
            else if (toGoal.Length2D() > mateBToGoal.Length2D())
                closest = mateB;
            if (closest > -1)
            {
                if (closest == 0)
                {
                    me.pursuing = 0;
                    me.role = (DAT_1020b430[other].soccer.role != SOCCER_ROLE_FORWARD) + 2;
                }
                else
                {
                    me.role = DAT_1020b430[other].soccer.role;
                    me.pursuing = 0;
                    DAT_1020b430[closest].soccer.role = SOCCER_ROLE_CHASER;
                    DAT_1020b430[closest].soccer.pursuing = 0;
                }
            }
        }
        else if (DAT_1020b430[other].soccer.role != SOCCER_ROLE_CHASER)
        {
            if (toGoal.Length2D() < mateAToGoal.Length2D() && toGoal.Length2D() < mateBToGoal.Length2D())
            {
                me.pursuing = 0;
                me.role = 0;
            }
        }
    }

    if (team == 0 && SOCCER_ROLE_PICK_BALL_Y < ball.y && me.role != SOCCER_ROLE_CHASER)
    {
        Vec4 selfPos;
        Vec4 mateAPos;
        Vec4 mateBPos;
        selfPos.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        mateAPos.Copy(DAT_10db6ee8[mateA].matrix.GetTranslation());
        mateBPos.Copy(DAT_10db6ee8[mateB].matrix.GetTranslation());
        int other = mateA;
        if (other == 0)
            other = mateB;
        int otherRole = DAT_1020b430[other].soccer.role;
        if (me.role != SOCCER_ROLE_FORWARD && selfPos.y > mateAPos.y && selfPos.y > mateBPos.y)
        {
            if (otherRole == SOCCER_ROLE_FORWARD)
            {
                DAT_1020b430[other].soccer.role = me.role;
                DAT_1020b430[other].soccer.pursuing = 0;
            }
            me.role = SOCCER_ROLE_FORWARD;
            me.pursuing = 0;
        }
        if (me.role != SOCCER_ROLE_BACK && (selfPos.y < mateAPos.y && selfPos.y > mateBPos.y ||
                                            selfPos.y < mateBPos.y && selfPos.y > mateAPos.y))
        {
            if (otherRole == SOCCER_ROLE_BACK)
            {
                DAT_1020b430[other].soccer.role = me.role;
                DAT_1020b430[other].soccer.pursuing = 0;
            }
            me.role = SOCCER_ROLE_BACK;
            me.pursuing = 0;
        }
    }

    if (DAT_10c635f8 == 0)
        step.Copy(step * 1.15f);
    else if (DAT_10c635f8 == 2 && arrived == 0)
        step.Copy(step * 1.3f);
    else
        step.Copy(step * 1.0f);
    if (index == 0x40)
        step.Copy(step * 1.3f);

    bool avoiding = false;
    for (int member = SOCCER_ROSTER_BALL; member < SOCCER_ROSTER_SIZE; member++)
    {
        if (member != SOCCER_ROSTER_BALL && DAT_1020c2b0[member] != index)
        {
            dir.Copy(DAT_10db6ee8[DAT_1020c2b0[member]].matrix.GetTranslation());
            float currentDistance = (dir - pos).Length2D();
            float nextDistance = (pos + step - dir).Length2D();
            if (currentDistance > nextDistance && currentDistance > 0.5f)
            {
                float radius = (float)(DAT_10211040[(member - 2) / 4][(member - 2) % 4] + clearance);
                if (nextDistance < radius)
                {
                    dir.Copy(dir - pos);
                    float angleToOther = (float)atan2(dir.x, dir.y) + SOCCER_PI;
                    float angleOfStep = (float)atan2(step.x, step.y) + SOCCER_PI;
                    float relative = angleOfStep;
                    while (relative > SOCCER_PI)
                        relative = relative - SOCCER_TWO_PI;
                    while (relative < -SOCCER_PI)
                        relative = relative + SOCCER_TWO_PI;
                    if (me.sidestep == 1)
                        relative = relative + SOCCER_SIDESTEP_ANGLE;
                    else if (me.sidestep == 2)
                        relative = relative - SOCCER_SIDESTEP_ANGLE;
                    if (angleOfStep - angleToOther <= 0.0f)
                        me.sidestep = 2;
                    else
                        me.sidestep = 1;
                    step.y = -step.Length2D();
                    step.x = 0.0f;
                    step.RotateZ(-relative);
                    me.sidestepFrames = SOCCER_SIDESTEP_FRAMES;
                    if (DAT_10802bf8[index].step.Length2D() < 5.0f)
                    {
                        if (++me.sidestepSwitch > SOCCER_SIDESTEP_SWITCH)
                        {
                            if (++me.sidestep == 3)
                                me.sidestep = 1;
                            me.sidestepSwitch = 0;
                        }
                    }
                    else if (me.sidestepSwitch > 0)
                        me.sidestepSwitch--;
                    avoiding = true;
                }
            }
        }
    }
    if (!avoiding)
    {
        if (me.sidestepFrames > 0)
            me.sidestepFrames--;
        else
        {
            me.sidestep = 0;
            me.sidestepSwitch = 0;
        }
    }

    if (step.Length2D() >= 1.0f)
    {
        Matrix4 probe;
        Vec4 probePoint;
        Vec4 normal;
        probe.SetIdentity();
        probePoint.Copy(DAT_10db6ee8[index].matrix.GetTranslation() + step);
        int slot = 3;
        do
        {
            if (DAT_1020c2b0[slot] == index)
                break;
            slot++;
        } while (slot < SOCCER_ROSTER_SIZE);
        if (slot == SOCCER_ROSTER_SIZE)
            slot = 3;
        float radius = (float)DAT_10211040[(slot - 2) / 4][(slot - 2) % 4];
        probePoint.z = probePoint.z + radius + SOCCER_PROBE_LIFT;
        float nextX = step.x + pos.x;
        float nextY = pos.y + step.y;
        if (nextX < (float)DAT_10211000[1].x && nextX > (float)DAT_10211000[0].x &&
            (nextY < (float)DAT_10211000[1].y && nextY > (float)DAT_10211000[0].y ||
             nextY < (float)DAT_10211000[3].y && nextY > (float)DAT_10211000[2].y))
        {
            if (pos.x > (float)DAT_10211000[1].x && step.x < 0.0f)
            {
                step.y = step.Length2D();
                step.x = 0.0f;
                if (me.sidestep == 1)
                    step.y *= -1.0f;
            }
            else if (pos.x < (float)DAT_10211000[0].x)
            {
                if (step.y > 0.0f)
                {
                    step.y = step.Length2D();
                    step.x = 0.0f;
                    if (me.sidestep == 2)
                        step.y *= -1.0f;
                }
                else
                {
                    step.x = step.Length2D();
                    step.y = 0.0f;
                    if (me.sidestep == 2)
                        step.x *= -1.0f;
                }
            }
            else if (step.y > 0.0f)
            {
                step.x = step.Length2D();
                step.y = 0.0f;
                if (me.sidestep == 1)
                    step.x *= -1.0f;
            }
            else
            {
                step.x = step.Length2D();
                step.y = 0.0f;
                if (me.sidestep == 2)
                    step.x *= -1.0f;
            }
            me.sidestepFrames = SOCCER_SIDESTEP_FRAMES;
            DAT_10db6ee8[index].matrix.translation.x += step.x;
            DAT_10db6ee8[index].matrix.translation.y += step.y;
            if (DAT_10802bf8[index].step.Length2D() < 10.0f)
            {
                if (++me.sidestepSwitch > SOCCER_SIDESTEP_FRAMES)
                {
                    if (++me.sidestep == 3)
                        me.sidestep = 1;
                    me.sidestepSwitch = 0;
                }
            }
            else if (me.sidestepSwitch > 0)
                me.sidestepSwitch--;
        }
        else
        {
            float depth;
            if (DAT_10255f08 != 0 && DAT_10255f08->Query(&probe, &probePoint, radius, &normal, &depth, 0, 0))
            {
                normal.Normalized3();
                float shove = (float)DAT_10211040[(slot - 2) / 4][(slot - 2) % 4] - sqrtf(depth) + 1.0f;
                step.Copy(step + normal * shove);
            }
            DAT_10db6ee8[index].matrix.translation.x += step.x;
            DAT_10db6ee8[index].matrix.translation.y += step.y;
        }
    }

    if (me.delayA != 0)
        me.delayA--;
    if (me.jumpSpeed != 0)
    {
        float height = (float)me.jumpSpeed + DAT_10db6ee8[index].matrix.translation.z;
        DAT_10db6ee8[index].matrix.translation.z = height;
        if (height < 0.0f)
        {
            DAT_10db6ee8[index].matrix.translation.z = 0.0f;
            DAT_10db6ee8[index].m_238c = 1;
            me.jumpSpeed = 0;
        }
    }
    if (DAT_10db6ee8[index].m_238c == 0)
        me.jumpSpeed -= SOCCER_GRAVITY;
    if (me.jumpCooldown > 0)
        me.jumpCooldown--;
    if (me.delayB > 0)
        me.delayB--;
    return 0;
}
