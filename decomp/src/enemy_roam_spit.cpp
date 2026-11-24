// A roaming spitter (0x10084e80, called from 0x100f9670): a ghost-like enemy that follows the player's recorded path
// (the frame matrices of the "Bip01" node), steers clear of the other roamers and now and then spits an effect at the
// player.

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
#include "camera_vars.h"
#include "dodge_game.h"
#include "world.h"

#include "char_ground.h"
#include "enemy_roam_spit.h"

#define ROAM_STEP_FOLLOW 0
#define ROAM_STEP_AVOID 1
#define ROAM_STEP_SPIT 2
#define ROAM_PATH_BASE 0x67
#define ROAM_PATH_CHAPTER_FRAMES 100
#define ROAM_PATH_CHAPTER_LIMIT 50
#define ROAM_GROUND_PERIOD 30
#define ROAM_SCALE 0.15f
#define ROAM_PI 3.1415f
#define ROAM_HALF_TURN 1.5708f
#define ROAM_QUARTER_TURN 1.570796f
#define ROAM_SIDE_TURN 1.39626f
#define ROAM_TURN_RANGE 6.283f
#define ROAM_PATH_FAR_SQUARED 400.0f
#define ROAM_PATH_SPEED 10.0f
#define ROAM_PATH_LIFT_LIMIT 20
#define ROAM_NEAR_RANGE 400.0f
#define ROAM_STEER_SPEED 8.0f
#define ROAM_STEER_LIFT 3.0f
#define ROAM_STEER_LOW 100.0f
#define ROAM_STEER_HIGH 150.0f
#define ROAM_FIRST_FRIEND 60
#define ROAM_FRIEND_END 66
#define ROAM_CROWD_RANGE 200.0f
#define ROAM_CROWD_NEAR_RANGE 100.0f
#define ROAM_FIRST_FRIEND_SEARCH 0x5c
#define ROAM_SLOT_END 0x7c
#define ROAM_FRIEND_KIND 'N'
#define ROAM_CROWD_SLOTS 13
#define ROAM_PLAYER_SLOTS 2
#define ROAM_LOW_FLIGHT 150.0f
#define ROAM_HIGH_FLIGHT 300.0f
#define ROAM_RADIUS_FACTOR 2.5f
#define ROAM_SPIT_CHANCE_RANGE 1000
#define ROAM_HUNT_CHANCE 2
#define ROAM_SPIT_CHANCE 50
#define ROAM_SPIT_MIN_WAIT 0x78
#define ROAM_SPIT_RANGE 1500.0f
#define ROAM_SPIT_DOUBLE_MODE 2
#define ROAM_SPIT_FAN 0.2618f
#define ROAM_SPIT_DROP 50.0f
#define ROAM_SPIT_EFFECT 600
#define ROAM_SPIT_SOUND 0x2a1
#define ROAM_SPIT_FAR_SQUARED 6250000.0f
#define ROAM_SPIT_NEAR_SLOT 0x3a
#define ROAM_HUNT_FRAMES 150
#define ROAM_HUNT_ACTIVE 30
#define ROAM_SPIT_MOTION_FRAMES 30
#define ROAM_MOTION_WALK 0
#define ROAM_MOTION_FOLLOW 1
#define ROAM_MOTION_IDLE 2
#define ROAM_MOTION_SPIT 4
#define ROAM_BLEND_FRAMES 5
#define ROAM_UNSET (-1)

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);   // Sound_PlayEffect
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

// 0x10084e80 (Enemy_RoamAndSpit). `index` is the character slot. A living roamer (while DAT_10db03bc is above 0 and
// DAT_10210e18 is clear) is unlocked and put on the ground every 30 frames. Step 0 follows the recorded path: when
// the path point is 20 or more units away it walks to it (motion 1); otherwise it faces the player and, now and then,
// spits (a hunt of 150 frames). Step 1 walks away from the player, steering around the other roamers (slots 60 to
// 65 and the 13 slots after the first kind 'N' character from slot 0x5c), turns to the player inside 400 units and
// spits with a 5 % chance per frame once 120 frames passed (step 2: 30 frames of spit motion). A spit is effect 600
// from 50 units below the head, tilted towards the player. Returns 0.
int Enemy_RoamAndSpit(int index)
{
    if (DAT_10daaae8[index].stats[STAT_HEALTH] <= 0)
        return 0;
    if (DAT_10210d30[index] != 0)
        return 0;
    if (DAT_10db03bc <= 0)
        return 0;
    if (DAT_10210e18 != 0)
        return 0;
    DAT_10db6ee8[index].m_238c = 0;
    DAT_10db6ee8[index].mode = 0;
    DAT_10218fc0[index] = 0;
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    DAT_10db6ee8[index].uniformScale = ROAM_SCALE;
    float flatRange = toPlayer.Length2D();
    float range = toPlayer.Length();
    toPlayer.Copy(Vec3_Normalized(&toPlayer));
    float heading = DAT_10db6ee8[index].heading;
    floorf(heading - atan2f(toPlayer.x, toPlayer.y) - ROAM_PI);
    if ((int)DAT_104c0380 % ROAM_GROUND_PERIOD == index % ROAM_GROUND_PERIOD)
    {
        DAT_10db6ee8[index].m_0012 = 2;
        FUN_1001c090(index, 1);
    }
    int motion = ROAM_UNSET;
    if ((DAT_1020c2b0 == 0 || DAT_1020c2b0 == 2 || DAT_1020c2b0 == 3) && DAT_1020c2c4 < ROAM_PATH_CHAPTER_LIMIT &&
        DAT_1020b430[index].talk + ROAM_PATH_BASE + DAT_1020c2c4 * ROAM_PATH_CHAPTER_FRAMES <
            DAT_1021c620->frameMatrixCount)
    {
        if (DAT_1020b430[index].state != ROAM_STEP_SPIT)
        {
            DAT_1020b430[index].state = 0;
            DAT_1020b430[index].wait = 0;
        }
    }
    else if (DAT_1020b430[index].state == 0)
        DAT_1020b430[index].state = 1;
    if (DAT_1020b430[index].state == ROAM_STEP_FOLLOW)
    {
        Vec4 trail;
        if (DAT_1020b430[index].talk + ROAM_PATH_BASE + DAT_1020c2c4 * ROAM_PATH_CHAPTER_FRAMES <
            DAT_1021c620->frameMatrixCount)
            trail.Copy(DAT_1021c620
                           ->frameMatrices[DAT_1020b430[index].talk + ROAM_PATH_BASE +
                                           DAT_1020c2c4 * ROAM_PATH_CHAPTER_FRAMES]
                           .GetTranslation());
        if (!((self - trail).LengthSquared3() < ROAM_PATH_FAR_SQUARED))
        {
            Vec4 move;
            move.Copy(trail - self);
            move.Normalized3();
            DAT_10db6ee8[index].targetHeading = atan2f(move.x, move.y) + ROAM_PI;
            move.Copy(move * ROAM_PATH_SPEED);
            float next = move.z + self.z;
            if (trail.z > next)
            {
                int lift = (int)(trail.z - self.z - move.z);
                if (lift >= ROAM_PATH_LIFT_LIMIT)
                    lift = ROAM_PATH_LIFT_LIMIT;
                float rise = (float)lift;
                move.z = rise + move.z;
            }
            else if (trail.z < next)
            {
                int lift = (int)(next - trail.z);
                if (lift >= ROAM_PATH_LIFT_LIMIT)
                    lift = ROAM_PATH_LIFT_LIMIT;
                float drop = (float)lift;
                move.z = move.z - drop;
            }
            FUN_100241e0(index, move.x, move.y, move.z);
            motion = ROAM_MOTION_FOLLOW;
        }
        else
        {
            DAT_10db6ee8[index].targetHeading = atan2f(toPlayer.x, toPlayer.y) + ROAM_PI;
            motion = ROAM_MOTION_IDLE;
            if (DAT_1020b430[index].huntFrames == 0 && FUN_1013f880() % ROAM_SPIT_CHANCE_RANGE < ROAM_HUNT_CHANCE)
            {
                DAT_1020b430[index].huntFrames = 1;
                Vec4 mouth;
                Vec4 aim;
                EffectOptions options;
                Matrix4 matrix;
                matrix.SetIdentity();
                if (range < ROAM_SPIT_RANGE && DAT_1030b8f8[index] != 0)
                {
                    int last = 0;
                    if (DAT_10c635f8 == ROAM_SPIT_DOUBLE_MODE)
                    {
                        last = 1;
                        motion = 1;
                    }
                    do
                    {
                        mouth.Fill(0.0f);
                        aim.Copy(DAT_10db6ee8[index].headNode->worldMatrix.GetTranslation());
                        aim.z = aim.z - ROAM_SPIT_DROP;
                        if (aim.z < player.z)
                        {
                            if (range != 0.0f)
                                matrix = RotationX(acosf(flatRange / range)) *
                                         RotationZ((float)motion * ROAM_SPIT_FAN + DAT_10db6ee8[index].heading);
                            else
                                matrix = RotationZ((float)motion * ROAM_SPIT_FAN + DAT_10db6ee8[index].heading);
                        }
                        else if (range != 0.0f)
                            matrix = RotationX(-acosf(flatRange / range)) *
                                     RotationZ((float)motion * ROAM_SPIT_FAN + DAT_10db6ee8[index].heading);
                        else
                            matrix = RotationZ((float)motion * ROAM_SPIT_FAN + DAT_10db6ee8[index].heading);
                        matrix.SetTranslation(aim);
                        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                        options.m_0000 = 0;
                        FUN_1001d8b0(index, ROAM_SPIT_EFFECT, mouth, -1, -1, 0, 0, 0, &matrix, &options);
                        motion++;
                    } while (motion <= last);
                    FUN_10065b10(index, ROAM_SPIT_SOUND, 0, 0);
                }
                motion = ROAM_MOTION_SPIT;
            }
            if (DAT_1020b430[index].huntFrames != 0)
            {
                DAT_1020b430[index].huntFrames++;
                motion = ROAM_MOTION_SPIT;
                if (DAT_1020b430[index].huntFrames > ROAM_HUNT_ACTIVE)
                    motion = ROAM_MOTION_IDLE;
                if (DAT_1020b430[index].huntFrames == ROAM_HUNT_FRAMES)
                    DAT_1020b430[index].huntFrames = 0;
            }
        }
    }
    else if (DAT_1020b430[index].state == ROAM_STEP_AVOID)
    {
        int closeIn = 0;
        Vec4 dir;
        motion = ROAM_MOTION_WALK;
        if (flatRange < ROAM_NEAR_RANGE)
            closeIn = 1;
        dir.Copy(toPlayer * ROAM_STEER_SPEED);
        dir.z = 0.0f;
        if (player.z + ROAM_STEER_LOW > self.z)
            dir.z = ROAM_STEER_LIFT;
        else if (player.z + ROAM_STEER_HIGH < self.z)
            dir.z = -ROAM_STEER_LIFT;
        if (closeIn)
            dir.RotateZ(ROAM_SIDE_TURN);
        Vec4 pos;
        int crowd = 0;
        bool found = false;
        float facing = atan2f(dir.x, dir.y) + ROAM_PI;
        int slot;
        for (slot = ROAM_FIRST_FRIEND; slot < ROAM_FRIEND_END; slot++)
        {
            if (slot != index)
            {
                pos.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
                if (((pos - self).Length2D() < ROAM_CROWD_RANGE && !closeIn) ||
                    (pos - self).Length2D() < ROAM_CROWD_NEAR_RANGE)
                {
                    float turn = atan2f(pos.x - self.x, pos.y - self.y) + ROAM_PI;
                    turn = turn - facing;
                    for (; turn < -ROAM_PI; turn += ROAM_TURN_RANGE)
                        ;
                    for (; turn > ROAM_PI; turn -= ROAM_TURN_RANGE)
                        ;
                    if (floorf(turn) < ROAM_HALF_TURN)
                    {
                        crowd++;
                        if (crowd == 1)
                        {
                            dir.x = pos.x - self.x;
                            dir.y = pos.y - self.y;
                        }
                        else
                        {
                            dir.x = (pos.x - self.x) / (float)crowd + dir.x;
                            dir.y = (pos.y - self.y) / (float)crowd + dir.y;
                        }
                        pos.z = dir.z;
                        dir.z = 0.0f;
                        dir.Normalized3();
                        dir.Copy(dir * ROAM_PATH_SPEED);
                        dir.RotateZ(turn < 0.0f ? -ROAM_QUARTER_TURN : ROAM_QUARTER_TURN);
                        facing = atan2f(dir.x, dir.y) + ROAM_PI;
                        dir.z = pos.z;
                        found = true;
                    }
                }
            }
        }
        slot = ROAM_FIRST_FRIEND_SEARCH;
        do
        {
            if (DAT_1021f3c0[slot] == ROAM_FRIEND_KIND)
                break;
            slot++;
        } while (slot < ROAM_SLOT_END);
        int end = slot + ROAM_CROWD_SLOTS;
        for (; slot < end && slot < ROAM_SLOT_END; slot++)
        {
            pos.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
            if ((slot > ROAM_PLAYER_SLOTS && self.z < ROAM_HIGH_FLIGHT) || self.z < ROAM_LOW_FLIGHT)
            {
                if ((pos - self).Length2D() < (float)DAT_1020b430[slot].bodyRadius * ROAM_RADIUS_FACTOR)
                {
                    float turn = pos.x - self.x;
                    turn = (float)atan2((double)turn, (double)(pos.y - self.y));
                    float diff = turn + ROAM_PI;
                    diff = diff - facing;
                    for (; diff < -ROAM_PI; diff += ROAM_TURN_RANGE)
                        ;
                    for (; diff > ROAM_PI; diff -= ROAM_TURN_RANGE)
                        ;
                    turn = (float)fabs(diff);
                    if (turn < ROAM_HALF_TURN)
                    {
                        crowd++;
                        if (crowd == 1)
                        {
                            dir.x = pos.x - self.x;
                            dir.y = pos.y - self.y;
                        }
                        else
                        {
                            dir.x = (pos.x - self.x) / (float)crowd + dir.x;
                            dir.y = (pos.y - self.y) / (float)crowd + dir.y;
                        }
                        pos.z = dir.z;
                        dir.z = 0.0f;
                        dir.Normalized3();
                        dir.Copy(dir * ROAM_PATH_SPEED);
                        dir.RotateZ(diff < 0.0f ? -ROAM_QUARTER_TURN : ROAM_QUARTER_TURN);
                        turn = (float)atan2((double)dir.x, (double)dir.y);
                        facing = turn + ROAM_PI;
                        dir.z = pos.z;
                    }
                }
            }
        }
        DAT_1020b430[index].wait++;
        if (!closeIn)
        {
            if (!found)
            {
                float angle = (float)atan2((double)dir.x, (double)dir.y);
                DAT_10db6ee8[index].targetHeading = angle + ROAM_PI;
            }
        }
        else
        {
            float angle = (float)atan2((double)toPlayer.x, (double)toPlayer.y);
            DAT_10db6ee8[index].targetHeading = angle + ROAM_PI;
            if (FUN_1013f880() % ROAM_SPIT_CHANCE_RANGE < ROAM_SPIT_CHANCE &&
                DAT_1020b430[index].wait > ROAM_SPIT_MIN_WAIT)
            {
                DAT_1020b430[index].state = ROAM_STEP_SPIT;
                DAT_1020b430[index].wait = 0;
                Vec4 mouth;
                Vec4 aim;
                EffectOptions options;
                Matrix4 matrix;
                matrix.SetIdentity();
                if (range < ROAM_SPIT_RANGE && DAT_1030b8f8[index] != 0)
                {
                    int last = 0;
                    if (DAT_10c635f8 == ROAM_SPIT_DOUBLE_MODE)
                    {
                        last = 1;
                        slot = 1;
                    }
                    do
                    {
                        mouth.Fill(0.0f);
                        aim.Copy(DAT_10db6ee8[index].headNode->worldMatrix.GetTranslation());
                        aim.z = aim.z - ROAM_SPIT_DROP;
                        if (aim.z < player.z)
                        {
                            if (range != 0.0f)
                                matrix = RotationX((float)acos((double)(flatRange / range))) *
                                         RotationZ((float)slot * ROAM_SPIT_FAN + DAT_10db6ee8[index].heading);
                            else
                                matrix = RotationZ((float)slot * ROAM_SPIT_FAN + DAT_10db6ee8[index].heading);
                        }
                        else if (range != 0.0f)
                            matrix = RotationX(-(float)acos((double)(flatRange / range))) *
                                     RotationZ((float)slot * ROAM_SPIT_FAN + DAT_10db6ee8[index].heading);
                        else
                            matrix = RotationZ((float)slot * ROAM_SPIT_FAN + DAT_10db6ee8[index].heading);
                        matrix.SetTranslation(aim);
                        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                        options.m_0000 = 0;
                        DAT_10daaa80.Start(ROAM_SPIT_EFFECT, mouth, -1, -1, 0, 0, 0, &matrix, &options,
                                           &DAT_10219770[index]);
                        slot++;
                    } while (slot <= last);
                    if ((DAT_10db6ee8[index].matrix.GetTranslation() - DAT_118e6cb8).LengthSquared3() <
                            ROAM_SPIT_FAR_SQUARED ||
                        index < ROAM_SPIT_NEAR_SLOT)
                    {
                        Vec4 where;
                        where.Set(DAT_10db6ee8[index].matrix.translation.x, DAT_10db6ee8[index].matrix.translation.y,
                                  DAT_10db6ee8[index].matrix.translation.z, 0.0f);
                        FUN_1003ad20(ROAM_SPIT_SOUND, &where, 1.0f);
                    }
                }
                motion = ROAM_MOTION_SPIT;
            }
            if (found)
            {
                dir.x = 0.0f;
                dir.y = 0.0f;
            }
        }
        FUN_100241e0(index, dir.x, dir.y, dir.z);
    }
    else if (DAT_1020b430[index].state == ROAM_STEP_SPIT)
    {
        DAT_1020b430[index].wait++;
        motion = ROAM_MOTION_SPIT;
        if (DAT_1020b430[index].wait > ROAM_SPIT_MOTION_FRAMES)
        {
            motion = ROAM_MOTION_IDLE;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = ROAM_STEP_AVOID;
        }
    }
    else
        return 0;
    if (motion == ROAM_UNSET)
        return 0;
    DAT_10db6ee8[index].PlayMotion(motion, 0, ROAM_BLEND_FRAMES, 1.0f);
    return 0;
}
