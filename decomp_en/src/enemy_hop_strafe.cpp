// Character kind 0x50: a strafing enemy that hops (EN build; JP: decomp/src/enemy_strafe.cpp, Enemy_HopAndStrafe). It chases
// and strafes like the lunging strafer (enemy_strafe_lunge.cpp), but its attack is a plain attack motion that fires an effect
// near its end, and it hops: while it rests in the idle motion it samples the ground under its feet and remembers the last
// three positions where it stood (the shot slot's step, history2 and history3); when it leaves the idle motion it hops back to
// the oldest of them. Its CharState::values words are named in strafe_enemy.h.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_dispatch.h"
#include "char_inline.h"
#include "char_move.h"
#include "char_shots.h"
#include "effect_manager.h"
#include "effect_options.h"
#include "engine.h"
#include "node.h"
#include "strafe_enemy.h"
#include "vec4.h"

// Highest roll (out of PERCENT_RANGE) that starts an attack while chasing, and while strafing or backing off.
const int CHASE_ATTACK_ROLL = 6;
const int STRAFE_ATTACK_ROLL = 2;
// Cooldown frames at or below which a chasing enemy may attack.
const int ATTACK_COOLDOWN_READY = 60;
// Percent roll range, and the number of strafe steps picked from when stuck (left, right, back).
const int PERCENT_RANGE = 100;
const int STRAFE_PICKS = 3;
// Move factors are written as literals: Vec4::operator* takes its factor by reference, so a named constant would not
// compile to the same code. The way back to history3 is covered in steps of a ninth (Divided(9.0f)).

// Character kind 0x50, one frame of enemy `index`. Marks itself active, then runs a hop under way (rise for the first
// frames, then move back to history3 in equal steps), samples the ground while resting, starts a hop when it leaves the idle
// motion (the first hop also starts a random strafe, the next one ends it; a hop towards a spot without ground is called off),
// and otherwise chases, strafes and attacks like the other strafing enemies. Returns 0. Called by the character dispatch every
// frame.
extern "C" int CharKind50_Update(int index)
{
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    g_hopperActive[index] = 1;
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    if (g_charToldToReact[index] != 0)
    {
        g_charToldToReact[index] = 0;
        g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
        g_charState[index].values[STRAFE_VALUE_WAIT] = 0;
    }
    if (g_charState[index].values[HOP_VALUE_HISTORY] == HOP_HISTORY_EMPTY)
    {
        g_charState[index].values[HOP_VALUE_HISTORY] = HOP_HISTORY_STARTED;
        g_shotSlots[index].history3.Copy(self);
        g_shotSlots[index].history2.Copy(self);
        g_shotSlots[index].step.Copy(self);
    }
    if (g_charState[index].values[HOP_VALUE_FRAME] != 0)
    {
        // a hop under way: rise, then move back to history3 in equal steps
        if (g_charState[index].values[HOP_VALUE_FRAME] < HOP_RISE_FRAMES)
        {
            float rise = (HOP_RISE_BASE - g_charState[index].values[HOP_VALUE_FRAME]) * 2;
            g_shotSlots[index].step.z += rise;
            g_characterModels[index].matrix.SetTranslation(g_shotSlots[index].step);
        }
        else
        {
            if (g_charState[index].values[HOP_VALUE_FRAME] == HOP_RISE_FRAMES)
                g_shotSlots[index].history2.Copy(
                    (g_shotSlots[index].history3 - g_shotSlots[index].step).Divided(9.0f));
            g_shotSlots[index].step.Copy(g_shotSlots[index].step + g_shotSlots[index].history2);
            g_characterModels[index].matrix.SetTranslation(g_shotSlots[index].step);
        }
        if (++g_charState[index].values[HOP_VALUE_FRAME] == HOP_FRAMES)
        {
            g_characterModels[index].matrix.SetTranslation(g_shotSlots[index].history3);
            g_shotSlots[index].history2.Copy(g_shotSlots[index].history3);
            g_shotSlots[index].step.Copy(g_shotSlots[index].history3);
            g_charState[index].values[HOP_VALUE_FRAME] = 0;
        }
        return 0;
    }
    if (g_characterModels[index].idleMotionEnabled == 1 && g_characterModels[index].verticalSpeed == 0.0f &&
        ++g_charState[index].values[HOP_VALUE_REST] > HOP_REST_FRAMES)
    {
        // resting: when there is ground below, remember this position
        Vec4 below;
        below.Set(0, 0, HOP_GROUND_PROBE, 0);
        if (g_characterModels[index].IsClearOfMapLimited(&below) == 0)
        {
            g_shotSlots[index].history3.Copy(g_shotSlots[index].history2);
            g_shotSlots[index].history2.Copy(g_shotSlots[index].step);
            g_shotSlots[index].step.Copy(self);
            g_charState[index].values[HOP_VALUE_HISTORY] = HOP_HISTORY_READY;
        }
    }
    if (g_characterModels[index].idleMotionEnabled == 0 && g_charState[index].values[HOP_VALUE_HISTORY] == HOP_HISTORY_READY)
    {
        g_charState[index].values[HOP_VALUE_REQUESTS]++;
        g_charState[index].values[HOP_VALUE_REST] = 0;
        if (g_charState[index].values[HOP_VALUE_BACK] == 1)
        {
            // the second hop: back to chasing
            AnimModel *model = &g_characterModels[index];
            model->PlayMotion(HOP_MOTION, 1, 5, 1.0f);
            g_charState[index].values[HOP_VALUE_REQUESTS] = 0;
            self.Copy(g_characterModels[index].matrix.GetTranslation());
            g_charState[index].values[HOP_VALUE_FRAME] = 1;
            g_charState[index].values[HOP_VALUE_BACK] = 0;
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
            g_charState[index].values[STRAFE_VALUE_WAIT] = HOP_STRAFE_FRAMES;
            g_shotSlots[index].history2.Copy(g_shotSlots[index].history3);
            g_shotSlots[index].step.Copy(g_shotSlots[index].history3);
            g_shotSlots[index].step.Copy(self);
            // call the hop off when there is no ground under the landing spot
            int groundAtLanding = 0;
            Vec4 probe;
            probe.Fill(0.0f);
            g_characterModels[index].matrix.SetTranslation(g_shotSlots[index].history3);
            if (model->IsClearOfMapLimited(&probe) != 0)
            {
                probe.Set(0, 0, HOP_GROUND_PROBE, 0);
                if (model->IsClearOfMapLimited(&probe) == 0)
                    groundAtLanding = 1;
            }
            g_characterModels[index].matrix.SetTranslation(self);
            if (groundAtLanding == 0)
            {
                g_charState[index].values[HOP_VALUE_FRAME] = 0;
                g_charState[index].values[HOP_VALUE_BACK] = 0;
                g_charState[index].values[HOP_VALUE_HISTORY] = HOP_HISTORY_READY;
            }
            return 0;
        }
        if (g_charState[index].values[HOP_VALUE_REQUESTS] > 1)
        {
            // the first hop: start a random strafe
            AnimModel *model = &g_characterModels[index];
            model->PlayMotion(HOP_MOTION, 1, 5, 1.0f);
            g_charState[index].values[HOP_VALUE_REQUESTS] = 0;
            self.Copy(g_characterModels[index].matrix.GetTranslation());
            g_charState[index].values[HOP_VALUE_FRAME] = 1;
            g_charState[index].values[HOP_VALUE_BACK] = 1;
            g_charState[index].values[STRAFE_VALUE_STEP] = Random() % STRAFE_PICKS + STRAFE_LEFT;
            g_charState[index].values[STRAFE_VALUE_WAIT] = HOP_STRAFE_FRAMES;
            g_shotSlots[index].history2.Copy(g_shotSlots[index].history3);
            g_shotSlots[index].step.Copy(g_shotSlots[index].history3);
            g_shotSlots[index].step.Copy(self);
            int groundAtLanding = 0;
            Vec4 probe;
            probe.Fill(0.0f);
            g_characterModels[index].matrix.SetTranslation(g_shotSlots[index].history3);
            if (model->IsClearOfMapLimited(&probe) != 0)
            {
                probe.Set(0, 0, HOP_GROUND_PROBE, 0);
                if (model->IsClearOfMapLimited(&probe) == 0)
                    groundAtLanding = 1;
            }
            g_characterModels[index].matrix.SetTranslation(self);
            if (groundAtLanding == 0)
            {
                g_charState[index].values[HOP_VALUE_FRAME] = 0;
                g_charState[index].values[HOP_VALUE_BACK] = 0;
                g_charState[index].values[HOP_VALUE_HISTORY] = HOP_HISTORY_READY;
            }
            return 0;
        }
    }
    if (g_charHitLevel[index] > 0)
        return 0;
    if (g_charState[index].values[STRAFE_VALUE_STEP] == STRAFE_ATTACK)
    {
        if (g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] != 0)
            g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER]--;
        if (g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] == 0)
            g_charState[index].values[STRAFE_VALUE_STEP] = STRAFE_CHASE;
        g_characterModels[index].FaceAwayFrom(&g_characterModels[0]);
        if (g_charState[index].values[STRAFE_VALUE_ATTACK_TIMER] == HOP_ATTACK_EFFECT_FRAME)
        {
            Vec4 offset;
            offset.Fill(0.0f);
            EffectOptions options;
            options.m_0000 = 0;
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = HOP_ATTACK_EFFECT_SCALE;
            options.owner = index;
            options.m_000e = HOP_ATTACK_EFFECT_MODE;
            Character_PlaySound(index, HOP_ATTACK_SOUND, 0, 0);
            Char_StartEffectInline(index, HOP_ATTACK_EFFECT, offset, -1, -1, 0, 0, 0, &g_characterModels[index].matrix, &options);
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
            if (g_shotSlots[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
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
            if (g_shotSlots[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
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
            if (g_shotSlots[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
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
            if (g_shotSlots[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
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
            if (g_shotSlots[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
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
            if (g_shotSlots[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
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
            // (unlike the other steps, backing off does not turn away from the player to attack)
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
            if (g_shotSlots[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
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
            if (g_shotSlots[index].position.Distance2D(self) < STRAFE_STUCK_DISTANCE)
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
