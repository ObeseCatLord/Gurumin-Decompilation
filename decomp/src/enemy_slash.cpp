// Character kind 0x4f: an enemy that runs at the player, gets ready when close, stands guard and slashes with its
// right arm (a trail effect drawn between the hand and the forearm bones); from far away it taunts instead. When it
// gets stuck while running it side-steps or backs off for a while. Dispatched by FUN_1009e2d0 (char_dispatch.cpp).
// Its CharState words: `state` = the SlashStep, `wait` = frames in the current step, `stuckFrames` = frames counted
// without moving.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "effect_manager.h"

#include "char_slot.h"

#include "char_state.h"

#include "char_tables.h"

#include "last_position.h"

#include "slash_enemy.h"

#define PI_APPROX 3.1415f              // pi as the game writes it
#define QUARTER_TURN (PI_APPROX / 2)   // a quarter turn (1.57075)

extern "C" int FUN_1013f880(void);                                          // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
// Char_StartEffect: this caller passes a second matrix (the trail's other end) where most pass 0.
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            const Matrix4 *target, int frame, const Matrix4 *matrix, const void *options);

// 0x1009d490 (Enemy_ChaseAndSlash, kind 0x4f). One frame of enemy `index`.
// - Chase: runs at the player (motion 2, 22 units a frame) while farther than 90 units; gets ready when closer than
//   200, or at random (6 % a frame) when closer than 300 after 30 frames. Stuck for 16 frames, it side-steps left,
//   right or backs off (picked at random).
// - Ready: faces the player for 31 frames, then guards.
// - Guard: faces the player (motion 4); after 20 frames, farther than 350 units, it may taunt (4 % a frame); after
//   5 frames, closer than 320, it may slash (51 % a frame).
// - Slash: faces the player for 51 frames; on frame 20 it plays sound 0x152 and starts trail effect 0x4c twice,
//   once from the hand towards the forearm and once the other way.
// - Taunt: 31 frames, then chase. Side-steps: like the chase run but turned (or backwards), for at most 120 frames
//   or until stuck again.
// Being told to react puts it on guard. It marks itself as not held while chasing or side-stepping. Returns 0.
extern "C" int FUN_1009d490(int index)
{
    DAT_10255a88[index] = 0;
    DAT_10db6ee8[index].m_02a1 = 1;
    Vec4 move;
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    move.Copy(toPlayer);
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].state = SLASH_GUARD;
    }
    if (DAT_1020b430[index].state == SLASH_ATTACK)
    {
        DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
        if (++DAT_1020b430[index].wait > SLASH_ATTACK_FRAMES)
        {
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = SLASH_GUARD;
        }
        if (DAT_1020b430[index].wait != SLASH_EFFECT_FRAME)
            return 0;
        Vec4 offset;
        offset.Fill(0.0f);
        EffectOptions options;
        options.m_0000 = 0;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0008 = SLASH_EFFECT_SCALE;
        options.owner = index;
        options.m_000e = SLASH_EFFECT_SETTING;
        // the trail runs between the right hand and the right forearm (the model's own matrix if a bone is missing)
        AnimModel *model = &DAT_10db6ee8[index];
        const Matrix4 *hand = &model->matrix;
        const Matrix4 *forearm = &model->matrix;
        Node *bone = model->root->Find("@Bip01 R Fin", 0);
        if (bone != 0)
            hand = &bone->worldMatrix;
        bone = model->root->Find("@Bip01 R Fore", 0);
        if (bone != 0)
            forearm = &bone->worldMatrix;
        FUN_10065b10(index, SLASH_SOUND, 0, 0);
        FUN_1001d8b0(index, SLASH_EFFECT, offset, -1, -1, hand, forearm, 0, &model->matrix, &options);
        FUN_1001d8b0(index, SLASH_EFFECT, offset, -1, -1, forearm, hand, 0, &model->matrix, &options);
        return 0;
    }
    else if (DAT_1020b430[index].state == SLASH_GUARD)
    {
        DAT_10daaae8[index].held = 0;
        DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_GUARD, 0, 5, 1.0f);
        DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
        if (++DAT_1020b430[index].wait > SLASH_TAUNT_MIN_FRAMES && FUN_1013f880() % 100 <= 3 &&
            toPlayer.Length2D() > SLASH_TAUNT_DISTANCE)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_TAUNT, 0, 5, 1.0f);
            DAT_1020b430[index].state = SLASH_TAUNT;
        }
        else if (DAT_1020b430[index].wait > SLASH_GUARD_MIN_FRAMES && FUN_1013f880() % 100 <= 50 &&
                 toPlayer.Length2D() < SLASH_ATTACK_DISTANCE)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_SLASH, 0, 5, 1.0f);
            DAT_1020b430[index].state = SLASH_ATTACK;
        }
        else
            return 0;
    }
    else if (DAT_1020b430[index].state == SLASH_READY)
    {
        DAT_10daaae8[index].held = 0;
        DAT_10db6ee8[index].FaceAwayFrom(&DAT_10db6ee8[0]);
        if (++DAT_1020b430[index].wait <= SLASH_READY_FRAMES)
            return 0;
        DAT_1020b430[index].state = SLASH_GUARD;
    }
    else if (DAT_1020b430[index].state == SLASH_TAUNT)
    {
        DAT_10daaae8[index].held = 0;
        if (++DAT_1020b430[index].wait <= SLASH_TAUNT_FRAMES)
            return 0;
        DAT_1020b430[index].state = SLASH_CHASE;
    }
    else if (DAT_1020b430[index].state == SLASH_CHASE)
    {
        DAT_10daaae8[index].held = 1;
        DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_RUN, 0, 5, 1.0f);
        if (toPlayer.Length2D() > SLASH_STOP_DISTANCE)
        {
            move.Normalized3();
            move.Copy(move * SLASH_RUN_SPEED);
            DAT_10db6ee8[index].Push(move.x, move.y, 0);
            float heading = (float)atan2((double)move.x, (double)move.y);
            DAT_10db6ee8[index].targetHeading = heading + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < SLASH_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].stuckFrames > SLASH_STUCK_FRAMES)
                {
                    DAT_1020b430[index].stuckFrames = 0;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].state = FUN_1013f880() % 3 + SLASH_SIDESTEP_LEFT;
                }
            }
            else
                DAT_1020b430[index].stuckFrames = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        DAT_1020b430[index].wait++;
        if (toPlayer.Length2D() < SLASH_LUNGE_DISTANCE && DAT_1020b430[index].wait > SLASH_CHASE_MIN_FRAMES &&
            FUN_1013f880() % 100 <= 5)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_READY, 0, 5, 1.0f);
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = SLASH_READY;
        }
        if (toPlayer.Length2D() < SLASH_READY_DISTANCE)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_READY, 0, 5, 1.0f);
            DAT_1020b430[index].state = SLASH_READY;
        }
        else
            return 0;
    }
    else if (DAT_1020b430[index].state == SLASH_SIDESTEP_LEFT)
    {
        DAT_10daaae8[index].held = 1;
        DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_RUN, 0, 5, 1.0f);
        if (toPlayer.Length2D() > SLASH_STOP_DISTANCE)
        {
            move.Normalized3();
            move.RotateZ(QUARTER_TURN);
            move.Copy(move * SLASH_RUN_SPEED);
            DAT_10db6ee8[index].Push(move.x, move.y, 0);
            float heading = (float)atan2((double)move.x, (double)move.y);
            DAT_10db6ee8[index].targetHeading = heading + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < SLASH_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].stuckFrames > SLASH_STUCK_FRAMES)
                {
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].state = SLASH_CHASE;
                    DAT_1020b430[index].stuckFrames = 0;
                }
            }
            else
                DAT_1020b430[index].stuckFrames = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        DAT_1020b430[index].wait++;
        if (toPlayer.Length2D() < SLASH_LUNGE_DISTANCE && DAT_1020b430[index].wait > SLASH_CHASE_MIN_FRAMES &&
            FUN_1013f880() % 100 <= 5)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_READY, 0, 5, 1.0f);
            DAT_1020b430[index].state = SLASH_READY;
        }
        else if (toPlayer.Length2D() < SLASH_READY_DISTANCE)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_READY, 0, 5, 1.0f);
            DAT_1020b430[index].state = SLASH_READY;
        }
        else if (DAT_1020b430[index].wait > SLASH_SIDESTEP_FRAMES)
        {
            DAT_1020b430[index].stuckFrames = 0;
            DAT_1020b430[index].state = SLASH_CHASE;
        }
        else
            return 0;
    }
    else if (DAT_1020b430[index].state == SLASH_SIDESTEP_RIGHT)
    {
        DAT_10daaae8[index].held = 1;
        DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_RUN, 0, 5, 1.0f);
        if (toPlayer.Length2D() > SLASH_STOP_DISTANCE)
        {
            move.Normalized3();
            move.RotateZ(-QUARTER_TURN);
            move.Copy(move * SLASH_RUN_SPEED);
            DAT_10db6ee8[index].Push(move.x, move.y, 0);
            float heading = (float)atan2((double)move.x, (double)move.y);
            DAT_10db6ee8[index].targetHeading = heading + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < SLASH_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].stuckFrames > SLASH_STUCK_FRAMES)
                {
                    DAT_1020b430[index].state = SLASH_CHASE;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].stuckFrames = 0;
                }
            }
            else
                DAT_1020b430[index].stuckFrames = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        DAT_1020b430[index].wait++;
        if (toPlayer.Length2D() < SLASH_LUNGE_DISTANCE && DAT_1020b430[index].wait > SLASH_CHASE_MIN_FRAMES &&
            FUN_1013f880() % 100 <= 5)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_READY, 0, 5, 1.0f);
            DAT_1020b430[index].state = SLASH_READY;
        }
        else if (toPlayer.Length2D() < SLASH_READY_DISTANCE)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_READY, 0, 5, 1.0f);
            DAT_1020b430[index].state = SLASH_READY;
        }
        else if (DAT_1020b430[index].wait > SLASH_SIDESTEP_FRAMES)
        {
            DAT_1020b430[index].stuckFrames = 0;
            DAT_1020b430[index].state = SLASH_CHASE;
        }
        else
            return 0;
    }
    else if (DAT_1020b430[index].state == SLASH_BACK_OFF)
    {
        DAT_10daaae8[index].held = 1;
        DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_RUN, 0, 5, 1.0f);
        if (toPlayer.Length2D() > SLASH_STOP_DISTANCE)
        {
            move.Normalized3();
            move.Copy(move * -SLASH_RUN_SPEED);
            DAT_10db6ee8[index].Push(move.x, move.y, 0);
            float heading = (float)atan2((double)move.x, (double)move.y);
            DAT_10db6ee8[index].targetHeading = heading + PI_APPROX;
            if (DAT_10802bf8[index].position.Distance(self) < SLASH_STUCK_DISTANCE)
            {
                if (++DAT_1020b430[index].stuckFrames > SLASH_STUCK_FRAMES)
                {
                    DAT_1020b430[index].state = SLASH_CHASE;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].stuckFrames = 0;
                }
            }
            else
                DAT_1020b430[index].stuckFrames = 0;
            DAT_10802bf8[index].position.Copy(self);
        }
        DAT_1020b430[index].wait++;
        if (toPlayer.Length2D() < SLASH_LUNGE_DISTANCE && DAT_1020b430[index].wait > SLASH_CHASE_MIN_FRAMES &&
            FUN_1013f880() % 100 <= 5)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_READY, 0, 5, 1.0f);
            DAT_1020b430[index].state = SLASH_READY;
        }
        else if (toPlayer.Length2D() < SLASH_READY_DISTANCE)
        {
            DAT_10db6ee8[index].PlayMotion(SLASH_MOTION_READY, 0, 5, 1.0f);
            DAT_1020b430[index].state = SLASH_READY;
        }
        else if (DAT_1020b430[index].wait > SLASH_SIDESTEP_FRAMES)
        {
            DAT_1020b430[index].stuckFrames = 0;
            DAT_1020b430[index].state = SLASH_CHASE;
        }
        else
            return 0;
    }
    else
        return 0;
    // every step change ends here: the new step starts counting from 0
    DAT_1020b430[index].wait = 0;
    return 0;
}
