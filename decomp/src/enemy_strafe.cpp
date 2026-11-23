// Character kinds 0x4c-0x4e: enemies that run at the player, fire an effect from their own matrix now and then, and
// when they get stuck (barely moving for 16 frames) strafe left, strafe right or back off for a while. Dispatched by
// FUN_1009e2d0 (char_dispatch.cpp). Their CharState words: `wait` = cooldown before the next attack (or the frames
// left of a strafe), `attacking` = the step below, `attackTimer` = frames left of an attack, `action` = frames
// counted without moving.

#include <math.h>
#include <string.h>
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "effect_manager.h"

#include "char_slot.h"

#include "char_state.h"

#include "char_tables.h"

#include "last_position.h"

#include "strafe_enemy.h"

#include "game_vars.h"

#include "char_vars.h"

#include "map_vars.h"

#include "model_bank.h"

extern "C" int FUN_1013f880(void);                                          // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect
extern "C" int FUN_1003a7d0(const char *path, float x, float y, float z, int start, float ox, float oy,
                            float oz);  // EffectModel_Place (effect_model.cpp)

// 0x1009a690 (Enemy_StrafeAndShoot, kind 0x4c). One frame of enemy `index`. Chasing, it runs at the player (motion 1,
// 10 units a frame) when farther than 300 units, walks (motion 0, 4 units) when farther than 120, else idles; with a
// 7 % chance per frame once its cooldown is down to 60 it attacks (motion 3 for 30 frames, facing away from the
// player, 100 frames of cooldown), firing effect 0x6d with sound 0x156 every frame of the attack while m_238c is set.
// When it moves less than 3 units a frame for 16 frames it strafes (left, right or back, picked at random) for 120
// frames, attacking with a 3 % chance per frame once the strafe is over. Being told to react cancels everything.
// Returns 0. (The heading is atan2 of the move direction plus pi; the arguments are converted to double by hand,
// which gives the original's 8-byte aligned frame.)
extern "C" int FUN_1009a690(int index)
{
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].attacking = STRAFE_CHASE;
        DAT_1020b430[index].wait = 0;
    }
    if (DAT_1020b430[index].attacking == STRAFE_ATTACK)
    {
        if (DAT_1020b430[index].attackTimer != 0)
            DAT_1020b430[index].attackTimer--;
        if (DAT_1020b430[index].attackTimer == 0)
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
        if (DAT_10db6ee8[index].m_238c == 0)
            return 0;
        DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
        Vec4 offset;
        offset.Fill(0.0f);
        EffectOptions options;
        options.m_0000 = 0;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0008 = 1.5f;
        options.owner = index;
        options.m_000e = 1;
        FUN_10065b10(index, 0x156, 0, 0);
        FUN_1001d8b0(index, 0x6d, offset, -1, -1, &DAT_10db6ee8[index].matrix, 0, 0, &DAT_10db6ee8[index].matrix,
                     &options);
        if (DAT_10db6ee8[index].m_238c != 0)
            DAT_10db6ee8[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
        return 0;
    }
    else if (DAT_1020b430[index].attacking == STRAFE_CHASE)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (FUN_1013f880() % 100 <= 6 && DAT_1020b430[index].wait <= 60)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_LEFT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_RIGHT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_BACK)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            // backing off, it attacks without turning away first
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * -2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * -4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else
        return 0;
    if (DAT_10db6ee8[index].m_238c != 0)
        DAT_10db6ee8[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
    return 0;
}

// 0x10099040 (Enemy_StrafeAndLunge, kind 0x4e). Like Enemy_StrafeAndShoot, but its attack (sped-up motion 3 for
// 30 frames) is a lunge: for the first 16 frames it rushes at the player (25 units a frame), and one frame into the
// attack it fires effect 0x17 with sound 0x155. It measures being stuck in 3D, and attacks while backing off too.
// Returns 0.
extern "C" int FUN_10099040(int index)
{
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].attacking = STRAFE_CHASE;
        DAT_1020b430[index].wait = 0;
    }
    if (DAT_1020b430[index].attacking == STRAFE_ATTACK)
    {
        if (DAT_1020b430[index].attackTimer != 0)
            DAT_1020b430[index].attackTimer--;
        if (DAT_1020b430[index].attackTimer == 0)
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        if (DAT_1020b430[index].attackTimer >= LUNGE_MIN_FRAMES)
        {
            // the first frames of the attack: a lunge at the player, 25 units a frame
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * 5.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            float heading = (float)atan2((double)toPlayer.x, (double)toPlayer.y);
            DAT_10db6ee8[index].targetHeading = heading + PI_APPROX;
        }
        if (DAT_1020b430[index].attackTimer == LUNGE_EFFECT_FRAME)
        {
            Vec4 offset;
            offset.Fill(0.0f);
            EffectOptions options;
            options.m_0000 = 0;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0008 = 1.5f;
            options.owner = index;
            options.m_000e = 1;
            FUN_10065b10(index, 0x155, 0, 0);
            FUN_1001d8b0(index, 0x17, offset, -1, -1, &DAT_10db6ee8[index].matrix, 0, 0, &DAT_10db6ee8[index].matrix,
                         &options);
        }
        return 0;
    }
    else if (DAT_1020b430[index].attacking == STRAFE_CHASE)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (FUN_1013f880() % 100 <= 6 && DAT_1020b430[index].wait <= 60)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_10db6ee8[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_LEFT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_10db6ee8[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_RIGHT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_10db6ee8[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_BACK)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_10db6ee8[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * -2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * -4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    return 0;
}

// 0x10099b30 (Enemy_StrafeAndBurst, kind 0x4d). Like Enemy_StrafeAndShoot, but its attack fires effect 0x6d every
// fourth frame (sound 0x157 with the first shot only), and the motion speeds up by 20 while it moves (and now and
// then while it idles) instead of while it attacks. A defeated one places a breaking-apart model once ("en_025nb.it3"
// for the EN_025n texture folder, else "en_025br.it3") and marks itself in DAT_10467ba0. Returns 0.
extern "C" int FUN_10099b30(int index)
{
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    if (DAT_10daaae8[index].stats[STAT_HEALTH] <= 0)
    {
        // defeated: once, a breaking-apart model in its colour is placed where it stands, turned like the player
        if (DAT_1021f9c0[index] == 0)
        {
            if (_stricmp(DAT_10db6ee8[index].root->folder, "EN_025n") == 0)
                FUN_1003a7d0("en_025nb.it3", DAT_10db6ee8[index].matrix.m[12], DAT_10db6ee8[index].matrix.m[13],
                             DAT_10db6ee8[index].matrix.m[14], 0, 0, 0, DAT_10db6ee8[0].heading * 180.0f / PI_APPROX);
            else
                FUN_1003a7d0("en_025br.it3", DAT_10db6ee8[index].matrix.m[12], DAT_10db6ee8[index].matrix.m[13],
                             DAT_10db6ee8[index].matrix.m[14], 0, 0, 0, DAT_10db6ee8[0].heading * 180.0f / PI_APPROX);
        }
        DAT_10467ba0[index] = DEFEATED_MARK;
        DAT_1021f9c0[index] = 1;
        return 0;
    }
    DAT_1021f9c0[index] = 0;
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].attacking = STRAFE_CHASE;
        DAT_1020b430[index].wait = 0;
    }
    if (DAT_1020b430[index].attacking == STRAFE_ATTACK)
    {
        if (DAT_1020b430[index].attackTimer != 0)
            DAT_1020b430[index].attackTimer--;
        if (DAT_1020b430[index].attackTimer == 0)
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
        if (DAT_10db6ee8[index].m_238c == 0)
            return 0;
        // a shot every fourth frame of the attack
        if (DAT_1020b430[index].attackTimer % 4 != 0)
            return 0;
        DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
        Vec4 offset;
        offset.Fill(0.0f);
        EffectOptions options;
        options.m_0000 = 0;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0008 = 1.5f;
        options.owner = index;
        options.m_000e = 1;
        if (DAT_1020b430[index].shotSound == SHOT_SOUND_PENDING)
            FUN_10065b10(index, 0x157, 0, 0);
        DAT_1020b430[index].shotSound = 0;
        FUN_1001d8b0(index, 0x6d, offset, -1, -1, &DAT_10db6ee8[index].matrix, 0, 0, &DAT_10db6ee8[index].matrix,
                     &options);
        return 0;
    }
    else if (DAT_1020b430[index].attacking == STRAFE_CHASE)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (FUN_1013f880() % 100 <= 6 && DAT_1020b430[index].wait <= 60)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].shotSound = SHOT_SOUND_PENDING;
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
            if (FUN_1013f880() % 100 > 2)
                return 0;
        }
    }
    else if (DAT_1020b430[index].attacking == STRAFE_LEFT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].shotSound = SHOT_SOUND_PENDING;
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
            if (FUN_1013f880() % 100 > 2)
                return 0;
        }
    }
    else if (DAT_1020b430[index].attacking == STRAFE_RIGHT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].shotSound = SHOT_SOUND_PENDING;
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
            if (FUN_1013f880() % 100 > 2)
                return 0;
        }
    }
    else if (DAT_1020b430[index].attacking == STRAFE_BACK)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            // backing off, it attacks without turning away first
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_1020b430[index].shotSound = SHOT_SOUND_PENDING;
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * -2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * -4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
            if (FUN_1013f880() % 100 > 2)
                return 0;
        }
    }
    else
        return 0;
    if (DAT_10db6ee8[index].m_238c != 0)
        DAT_10db6ee8[index].AddMotionSpeed(MOVE_MOTION_SPEED);
    return 0;
}

// 0x1009b120 (Enemy_HopAndStrafe, kind 0x50). One frame of enemy `index`. It chases and strafes like
// Enemy_StrafeAndLunge, but its attack (motion 3 for 30 frames, facing away from the player) fires effect 0x57 with
// sound 0x153 when 14 frames are left, and it hops: while it rests in the idle motion it samples the ground under its
// feet and keeps the last three positions where it stood (LastPosition step / history2 / history3); when it leaves
// the idle motion it hops back to the oldest of them (rising for 10 frames, then moving there in ninths), the first
// hop starting a random strafe and the next one ending it. A hop towards a spot without ground is called off.
// Hopping replaces the rest of the frame; a character told to wait (DAT_102550e0) does nothing else. Returns 0.
extern "C" int FUN_1009b120(int index)
{
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    DAT_10254310[index] = 1;
    Matrix4 *matrix = &DAT_10db6ee8[index].matrix;
    self.Copy(matrix->GetTranslation());
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].attacking = STRAFE_CHASE;
        DAT_1020b430[index].wait = 0;
    }
    if (DAT_1020b430[index].hopHistory == HOP_HISTORY_EMPTY)
    {
        DAT_1020b430[index].hopHistory = HOP_HISTORY_STARTED;
        DAT_10802bf8[index].history3.Copy(self);
        DAT_10802bf8[index].history2.Copy(self);
        DAT_10802bf8[index].step.Copy(self);
    }
    if (DAT_1020b430[index].hopFrame != 0)
    {
        // a hop under way: rise, then move back to history3 in equal steps
        if (DAT_1020b430[index].hopFrame < HOP_RISE_FRAMES)
        {
            float rise = (HOP_RISE_BASE - DAT_1020b430[index].hopFrame) * 2;
            DAT_10802bf8[index].step.z += rise;
            matrix->SetTranslation(DAT_10802bf8[index].step);
        }
        else
        {
            if (DAT_1020b430[index].hopFrame == HOP_RISE_FRAMES)
                DAT_10802bf8[index].history2.Copy(
                    (DAT_10802bf8[index].history3 - DAT_10802bf8[index].step).Divided(HOP_RETURN_DIVISOR));
            DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].step + DAT_10802bf8[index].history2);
            matrix->SetTranslation(DAT_10802bf8[index].step);
        }
        if (++DAT_1020b430[index].hopFrame == HOP_FRAMES)
        {
            matrix->SetTranslation(DAT_10802bf8[index].history3);
            DAT_10802bf8[index].history2.Copy(DAT_10802bf8[index].history3);
            DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].history3);
            DAT_1020b430[index].hopFrame = 0;
        }
        return 0;
    }
    if (DAT_10db6ee8[index].m_238c == 1 && DAT_10db6ee8[index].motionSpeed == 0.0f &&
        ++DAT_1020b430[index].restFrames > HOP_REST_FRAMES)
    {
        // resting: when there is ground below, remember this position
        Vec4 below;
        below.Set(0, 0, HOP_GROUND_PROBE, 0);
        if (DAT_10db6ee8[index].IsClearOfMapLimited(&below) == 0)
        {
            DAT_10802bf8[index].history3.Copy(DAT_10802bf8[index].history2);
            DAT_10802bf8[index].history2.Copy(DAT_10802bf8[index].step);
            DAT_10802bf8[index].step.Copy(self);
            DAT_1020b430[index].hopHistory = HOP_HISTORY_READY;
        }
    }
    if (DAT_10db6ee8[index].m_238c == 0 && DAT_1020b430[index].hopHistory == HOP_HISTORY_READY)
    {
        DAT_1020b430[index].hopRequests++;
        DAT_1020b430[index].restFrames = 0;
        if (DAT_1020b430[index].hopBack == 1)
        {
            // the second hop: back to chasing
            AnimModel *model = &DAT_10db6ee8[index];
            model->PlayMotion(HOP_MOTION, 1, 5, 1.0f);
            DAT_1020b430[index].hopRequests = 0;
            self.Copy(matrix->GetTranslation());
            DAT_1020b430[index].hopFrame = 1;
            DAT_1020b430[index].hopBack = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
            DAT_1020b430[index].wait = HOP_STRAFE_FRAMES;
            DAT_10802bf8[index].history2.Copy(DAT_10802bf8[index].history3);
            DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].history3);
            DAT_10802bf8[index].step.Copy(self);
            // call the hop off when there is no ground under the landing spot
            int groundAtLanding = 0;
            Vec4 probe;
            probe.Fill(0.0f);
            matrix->SetTranslation(DAT_10802bf8[index].history3);
            if (model->IsClearOfMapLimited(&probe) != 0)
            {
                probe.Set(0, 0, HOP_GROUND_PROBE, 0);
                if (model->IsClearOfMapLimited(&probe) == 0)
                    groundAtLanding = 1;
            }
            matrix->SetTranslation(self);
            if (groundAtLanding == 0)
            {
                DAT_1020b430[index].hopFrame = 0;
                DAT_1020b430[index].hopBack = 0;
                DAT_1020b430[index].hopHistory = HOP_HISTORY_READY;
            }
            return 0;
        }
        if (DAT_1020b430[index].hopRequests > 1)
        {
            // the first hop: start a random strafe
            AnimModel *model = &DAT_10db6ee8[index];
            model->PlayMotion(HOP_MOTION, 1, 5, 1.0f);
            DAT_1020b430[index].hopRequests = 0;
            self.Copy(matrix->GetTranslation());
            DAT_1020b430[index].hopFrame = 1;
            DAT_1020b430[index].hopBack = 1;
            DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
            DAT_1020b430[index].wait = HOP_STRAFE_FRAMES;
            DAT_10802bf8[index].history2.Copy(DAT_10802bf8[index].history3);
            DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].history3);
            DAT_10802bf8[index].step.Copy(self);
            int groundAtLanding = 0;
            Vec4 probe;
            probe.Fill(0.0f);
            matrix->SetTranslation(DAT_10802bf8[index].history3);
            if (model->IsClearOfMapLimited(&probe) != 0)
            {
                probe.Set(0, 0, HOP_GROUND_PROBE, 0);
                if (model->IsClearOfMapLimited(&probe) == 0)
                    groundAtLanding = 1;
            }
            matrix->SetTranslation(self);
            if (groundAtLanding == 0)
            {
                DAT_1020b430[index].hopFrame = 0;
                DAT_1020b430[index].hopBack = 0;
                DAT_1020b430[index].hopHistory = HOP_HISTORY_READY;
            }
            return 0;
        }
    }
    if (DAT_102550e0[index] > 0)
        return 0;
    if (DAT_1020b430[index].attacking == STRAFE_ATTACK)
    {
        if (DAT_1020b430[index].attackTimer != 0)
            DAT_1020b430[index].attackTimer--;
        if (DAT_1020b430[index].attackTimer == 0)
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
        if (DAT_1020b430[index].attackTimer == HOP_ATTACK_EFFECT_FRAME)
        {
            Vec4 offset;
            offset.Fill(0.0f);
            EffectOptions options;
            options.m_0000 = 0;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0008 = 1.5f;
            options.owner = index;
            options.m_000e = 1;
            FUN_10065b10(index, HOP_ATTACK_SOUND, 0, 0);
            FUN_1001d8b0(index, HOP_ATTACK_EFFECT, offset, -1, -1, 0, 0, 0, matrix, &options);
        }
        return 0;
    }
    else if (DAT_1020b430[index].attacking == STRAFE_CHASE)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (FUN_1013f880() % 100 <= 6 && DAT_1020b430[index].wait <= 60)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_LEFT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_RIGHT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_BACK)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            // (unlike the other steps, backing off does not turn away from the player to attack)
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * -2.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * -4.0f);
            DAT_10db6ee8[index].Push(toPlayer.x, toPlayer.y, 0);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    return 0;
}

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))


// 0x1009c070 (Enemy_FlyAndStrafe, kind 0x51). One frame of flying enemy `index`. It chases and strafes like
// Enemy_HopAndStrafe, steering with SetMoveStep instead of pushing: running (15 units a frame) it climbs towards 200
// units above the player, walking (5 units a frame) towards 40 units above, at most 5 units a frame either way. Every
// 30 frames (on a frame picked by its slot number) its shadow is put on the ground below it when DAT_102022b8 is
// set. Its attack fires effect 7 with sound 0x158 when 14 frames are left. While greetings are on (DAT_104c00e0) a
// player closer than 150 units makes it back away for 15 frames, rising 8 units a frame. Returns 0.
extern "C" int FUN_1009c070(int index)
{
    DAT_10db6ee8[index].mode = 0;
    DAT_10db6ee8[index].m_0012 = 2;
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    Matrix4 *matrix = &DAT_10db6ee8[index].matrix;
    self.Copy(matrix->GetTranslation());
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    // vertical steps towards the low and high flying heights, at most FLY_CLIMB_LIMIT a frame
    float climbLow = player.z + FLY_LOW_HEIGHT - self.z;
    climbLow = max(min(climbLow, FLY_CLIMB_LIMIT), -FLY_CLIMB_LIMIT);
    float climbHigh = player.z + FLY_HIGH_HEIGHT - self.z;
    climbHigh = max(min(climbHigh, FLY_CLIMB_LIMIT), -FLY_CLIMB_LIMIT);
    toPlayer.Copy(player - self);
    toPlayer.z = 0;
    if (DAT_104bf890 % FLY_GROUND_PERIOD == index % 10 && DAT_102022b8 != 0)
    {
        // put the shadow on the ground below (as Char_PlaceOnGround does)
        Vec4 from;
        Vec4 ground;
        Vec4 top = matrix->GetTranslation();
        from.Copy(top);
        top.z += 10.0f;
        Vec4 direction = *DAT_10804bf8.LightDirection(&DAT_10db6ee8[index], &DAT_118e69e8, &DAT_118e6cb8);
        direction.Normalized3();
        float distance = 1000.0f;
        from.Copy(from - direction * distance);
        if (DAT_10255f08->CastSegment2(0, 0x4000000, &from, &top, &ground, 0) != 0)
        {
            ground.x = from.x;
            ground.y = from.y;
            DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
        }
        else
        {
            ground.x = from.x;
            ground.y = from.y;
            ground.z = -3500.0f;
            DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
        }
    }
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].attacking = STRAFE_CHASE;
        DAT_1020b430[index].wait = 0;
    }
    if (DAT_104c00e0 != 0 && toPlayer.Length() < FLEE_DISTANCE)
    {
        DAT_1020b430[index].attackTimer = FLEE_FRAMES;
        DAT_1020b430[index].attacking = STRAFE_FLEE;
        DAT_1020b430[index].wait = 0;
    }
    if (DAT_1020b430[index].attacking == STRAFE_FLEE)
    {
        // backing away from the player while rising
        if (DAT_1020b430[index].attackTimer != 0)
            DAT_1020b430[index].attackTimer--;
        DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
        toPlayer.Normalized3();
        toPlayer.Copy(toPlayer * FLEE_STEP);
        DAT_10db6ee8[index].SetMoveStep(toPlayer.x, toPlayer.y, FLEE_RISE);
        DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
        if (DAT_1020b430[index].attackTimer == 0)
        {
            DAT_10210b30[index] = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
            DAT_1020b430[index].wait = 0;
        }
        return 0;
    }
    else if (DAT_1020b430[index].attacking == STRAFE_ATTACK)
    {
        if (DAT_1020b430[index].attackTimer != 0)
            DAT_1020b430[index].attackTimer--;
        if (DAT_1020b430[index].attackTimer == 0)
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
        if (DAT_1020b430[index].attackTimer == FLY_ATTACK_EFFECT_FRAME)
        {
            Vec4 offset;
            offset.Fill(0.0f);
            EffectOptions options;
            options.m_0000 = 0;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0008 = 1.5f;
            options.owner = index;
            options.m_000e = 1;
            FUN_10065b10(index, FLY_ATTACK_SOUND, 0, 0);
            FUN_1001d8b0(index, FLY_ATTACK_EFFECT, offset, -1, -1, 0, 0, 0, matrix, &options);
        }
        return 0;
    }
    else if (DAT_1020b430[index].attacking == STRAFE_CHASE)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (FUN_1013f880() % 100 <= 6 && DAT_1020b430[index].wait <= 60)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * 3.0f);
            DAT_10db6ee8[index].SetMoveStep(toPlayer.x, toPlayer.y, climbHigh);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f);
            DAT_10db6ee8[index].SetMoveStep(toPlayer.x, toPlayer.y, climbLow);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = STRAFE_FRAMES;
                    DAT_1020b430[index].attacking = FUN_1013f880() % 3 + STRAFE_LEFT;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_LEFT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 3.0f);
            DAT_10db6ee8[index].SetMoveStep(toPlayer.x, toPlayer.y, climbHigh);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f);
            DAT_10db6ee8[index].SetMoveStep(toPlayer.x, toPlayer.y, climbLow);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_RIGHT)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 3.0f);
            DAT_10db6ee8[index].SetMoveStep(toPlayer.x, toPlayer.y, climbHigh);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f);
            DAT_10db6ee8[index].SetMoveStep(toPlayer.x, toPlayer.y, climbLow);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (DAT_1020b430[index].attacking == STRAFE_BACK)
    {
        if (DAT_1020b430[index].wait != 0)
            DAT_1020b430[index].wait--;
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].attacking = STRAFE_CHASE;
        }
        if (FUN_1013f880() % 100 <= 2 && DAT_1020b430[index].wait == 0)
        {
            // (unlike the other steps, backing off does not turn away from the player to attack)
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            DAT_1020b430[index].wait = STRAFE_ATTACK_COOLDOWN;
            DAT_1020b430[index].attacking = STRAFE_ATTACK;
            DAT_1020b430[index].attackTimer = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * -3.0f);
            DAT_10db6ee8[index].SetMoveStep(toPlayer.x, toPlayer.y, climbHigh);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * -5.0f);
            DAT_10db6ee8[index].SetMoveStep(toPlayer.x, toPlayer.y, climbLow);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].action > STRAFE_STUCK_FRAMES)
                {
                    DAT_1020b430[index].action = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].attacking = STRAFE_CHASE;
                }
            }
            else
                DAT_1020b430[index].action = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        else
            DAT_10db6ee8[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    return 0;
}
