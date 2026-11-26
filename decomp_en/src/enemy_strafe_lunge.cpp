// Character kind 0x4e: an enemy that runs at the player and, now and then, lunges at him (EN build; JP:
// decomp/src/enemy_strafe.cpp, Enemy_StrafeAndLunge). When it gets stuck (barely moving for 16 frames) it strafes left,
// strafes right or backs off for a while. Its CharState::values words are named in strafe_enemy.h.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_dispatch.h"
#include "char_inline.h"
#include "char_shots.h"
#include "effect_manager.h"
#include "effect_options.h"
#include "engine.h"
#include "node.h"
#include "strafe_enemy.h"
#include "vec4.h"

// Sound and effect of the lunge, and the effect's size and mode.
const int LUNGE_SOUND = 0x155;
const int LUNGE_EFFECT = 0x17;
const float LUNGE_EFFECT_SCALE = 1.5f;
const char LUNGE_EFFECT_MODE = 1;
// Highest roll (out of PERCENT_RANGE) that starts an attack while chasing, and while strafing or backing off.
const int CHASE_ATTACK_ROLL = 6;
const int STRAFE_ATTACK_ROLL = 2;
// Cooldown frames at or below which a chasing enemy may attack.
const int ATTACK_COOLDOWN_READY = 60;
// Percent roll range, and the number of strafe steps picked from when stuck (left, right, back).
const int PERCENT_RANGE = 100;
const int STRAFE_PICKS = 3;
// Move factors are written as literals: Vec4::operator* takes its factor by reference, so a named constant would not
// compile to the same code. The lunge rush is 5 * 5 = 25 units a frame, the run 5 * 2 = 10 and the walk 4 (negated
// while backing off).

// Character kind 0x4e, one frame of enemy `index`. Like the other strafing enemies, but its attack (sped-up motion 3
// for 30 frames) is a lunge: for the first 16 frames it rushes at the player (25 units a frame), and one frame into
// the attack it fires effect 0x17 with sound 0x155. It measures being stuck in 3D, and attacks while backing off
// too. Returns 0. Called by the character dispatch every frame.
extern "C" int CharKind4e_Update(int index)
{
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    if (g_charToldToReact[index] != 0)
    {
        g_charToldToReact[index] = 0;
        g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
        g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
    }
    if (g_charState[index].values[STRAFE_VALUE_STEP] == STRAFE_ATTACK)
    {
        if (g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] != 0)
            g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER]--;
        if (g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] == 0)
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
        if (g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] >= LUNGE_MIN_FRAMES)
        {
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * 5.0f);
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
        }
        if (g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] == LUNGE_EFFECT_FRAME)
        {
            Vec4 offset;
            offset.Fill(0.0f);
            EffectOptions options;
            options.m_0000 = 0;
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = LUNGE_EFFECT_SCALE;
            options.owner = index;
            options.m_000e = LUNGE_EFFECT_MODE;
            Character_PlaySoundInline(index, LUNGE_SOUND, 0, 0);
            Char_StartEffectInline(index, LUNGE_EFFECT, offset, -1, -1, &g_characterModels[index].matrix, 0, 0,
                                   &g_characterModels[index].matrix, &options);
        }
        return 0;
    }
    else if (g_charState[index].values[STRAFE_VALUE_STEP] == STRAFE_CHASE)
    {
        if (g_charState[index].values[STRAFE_VALUE_WAIT] != 0)
            g_charState[index].values[STRAFE_VALUE_WAIT]--;
        if (Random() % PERCENT_RANGE <= CHASE_ATTACK_ROLL &&
            g_charState[index].values[STRAFE_VALUE_WAIT] <= ATTACK_COOLDOWN_READY)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            g_characterModels[index].FaceAwayFrom(&g_characterModels[0]);
            g_characterModels[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
            g_charState[index].values[STRAFE_VALUE_WAIT] = STRAFE_ATTACK_COOLDOWN;
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_ATTACK;
            g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (g_shotSlots[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++g_charState[index].values[STRAFE_VALUE_STUCK] > STRAFE_STUCK_FRAMES)
                {
                    g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
                    g_charState[index].values[STRAFE_VALUE_WAIT] = STRAFE_FRAMES;
                    g_charState[index].values[STRAFE_VALUE_STEP] = Random() % STRAFE_PICKS + STRAFE_LEFT;
                }
            }
            else
                g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_shotSlots[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 4.0f);
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (g_shotSlots[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++g_charState[index].values[STRAFE_VALUE_STUCK] > STRAFE_STUCK_FRAMES)
                {
                    g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
                    g_charState[index].values[STRAFE_VALUE_WAIT] = STRAFE_FRAMES;
                    g_charState[index].values[STRAFE_VALUE_STEP] = Random() % STRAFE_PICKS + STRAFE_LEFT;
                }
            }
            else
                g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_shotSlots[index].position.Copy(self);
        }
        else
            g_characterModels[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (g_charState[index].values[STRAFE_VALUE_STEP] == STRAFE_LEFT)
    {
        if (g_charState[index].values[STRAFE_VALUE_WAIT] != 0)
            g_charState[index].values[STRAFE_VALUE_WAIT]--;
        if (g_charState[index].values[STRAFE_VALUE_WAIT] == 0)
        {
            g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
        }
        if (Random() % PERCENT_RANGE <= STRAFE_ATTACK_ROLL && g_charState[index].values[STRAFE_VALUE_WAIT] == 0)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            g_characterModels[index].FaceAwayFrom(&g_characterModels[0]);
            g_characterModels[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
            g_charState[index].values[STRAFE_VALUE_WAIT] = STRAFE_ATTACK_COOLDOWN;
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_ATTACK;
            g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (g_shotSlots[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++g_charState[index].values[STRAFE_VALUE_STUCK] > STRAFE_STUCK_FRAMES)
                {
                    g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
                    g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
                    g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
                }
            }
            else
                g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_shotSlots[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(-QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (g_shotSlots[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++g_charState[index].values[STRAFE_VALUE_STUCK] > STRAFE_STUCK_FRAMES)
                {
                    g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
                    g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
                    g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
                }
            }
            else
                g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_shotSlots[index].position.Copy(self);
        }
        else
            g_characterModels[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (g_charState[index].values[STRAFE_VALUE_STEP] == STRAFE_RIGHT)
    {
        if (g_charState[index].values[STRAFE_VALUE_WAIT] != 0)
            g_charState[index].values[STRAFE_VALUE_WAIT]--;
        if (g_charState[index].values[STRAFE_VALUE_WAIT] == 0)
        {
            g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
        }
        if (Random() % PERCENT_RANGE <= STRAFE_ATTACK_ROLL && g_charState[index].values[STRAFE_VALUE_WAIT] == 0)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            g_characterModels[index].FaceAwayFrom(&g_characterModels[0]);
            g_characterModels[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
            g_charState[index].values[STRAFE_VALUE_WAIT] = STRAFE_ATTACK_COOLDOWN;
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_ATTACK;
            g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 5.0f * 2.0f);
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (g_shotSlots[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++g_charState[index].values[STRAFE_VALUE_STUCK] > STRAFE_STUCK_FRAMES)
                {
                    g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
                    g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
                    g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
                }
            }
            else
                g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_shotSlots[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.RotateZ(QUARTER_TURN);
            toPlayer.Copy(toPlayer * 4.0f);
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (g_shotSlots[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++g_charState[index].values[STRAFE_VALUE_STUCK] > STRAFE_STUCK_FRAMES)
                {
                    g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
                    g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
                    g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
                }
            }
            else
                g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_shotSlots[index].position.Copy(self);
        }
        else
            g_characterModels[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    else if (g_charState[index].values[STRAFE_VALUE_STEP] == STRAFE_BACK)
    {
        if (g_charState[index].values[STRAFE_VALUE_WAIT] != 0)
            g_charState[index].values[STRAFE_VALUE_WAIT]--;
        if (g_charState[index].values[STRAFE_VALUE_WAIT] == 0)
        {
            g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
        }
        if (Random() % PERCENT_RANGE <= STRAFE_ATTACK_ROLL && g_charState[index].values[STRAFE_VALUE_WAIT] == 0)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_ATTACK, 1, 5, 1.0f);
            g_characterModels[index].FaceAwayFrom(&g_characterModels[0]);
            g_characterModels[index].AddMotionSpeed(ATTACK_MOTION_SPEED);
            g_charState[index].values[STRAFE_VALUE_WAIT] = STRAFE_ATTACK_COOLDOWN;
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_ATTACK;
            g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] = STRAFE_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > STRAFE_FAR)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_RUN, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * -2.0f);
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (g_shotSlots[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++g_charState[index].values[STRAFE_VALUE_STUCK] > STRAFE_STUCK_FRAMES)
                {
                    g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
                    g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
                    g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
                }
            }
            else
                g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_shotSlots[index].position.Copy(self);
        }
        else if (toPlayer.Length() > STRAFE_NEAR)
        {
            g_characterModels[index].PlayMotion(STRAFE_MOTION_WALK, 0, 5, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * -4.0f);
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
            if (g_shotSlots[index].position.Distance(self) < STRAFE_STUCK_DISTANCE)
            {
                if (++g_charState[index].values[STRAFE_VALUE_STUCK] > STRAFE_STUCK_FRAMES)
                {
                    g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
                    g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
                    g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
                }
            }
            else
                g_charState[index].values[STRAFE_VALUE_STUCK] = 0;
            g_shotSlots[index].position.Copy(self);
        }
        else
            g_characterModels[index].PlayMotion(STRAFE_MOTION_IDLE, 0, 5, 1.0f);
    }
    return 0;
}
