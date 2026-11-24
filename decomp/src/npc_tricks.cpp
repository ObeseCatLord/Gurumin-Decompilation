// An NPC that performs one of several tricks while a neighbour watches (0x100b84a0, behaviour 0x2d of
// Char_RunBehaviour 0x100dfcf0): the sibling of the gesturer in npc_gesture.cpp. The character `talk / 100` slots
// after it is the one it plays to and cheers along with.

#include "last_position.h"
#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

typedef char check_walker[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "char_tables.h"
#include "char_vars.h"
#include "world.h"

#include "enemy_chase_hop.h"
#include "npc_tricks.h"

#define TRICKS_FIRST_NEIGHBOUR 0x3a
#define TRICKS_NEIGHBOUR_LIMIT 0x5c
#define TRICKS_STATE_START 0
#define TRICKS_STATE_FLOURISH 1
#define TRICKS_STATE_SPIN 2
#define TRICKS_STATE_WAVE 3
#define TRICKS_STATE_STRUT 4
#define TRICKS_STATE_DASH 5
#define TRICKS_STATE_BOW 6
#define TRICKS_STATE_CHEER 99
#define TRICKS_STATE_DONE 100
#define TRICKS_STATE_SLEEP 0x65
#define TRICKS_TRICK_COUNT 5
#define TRICKS_BOW_CHANCE 10
#define TRICKS_BOW_FRAMES 0x3c
#define TRICKS_CHEER_FRAMES 0xd
#define TRICKS_CHEER_MOTION 0x5a
#define TRICKS_CHEER_VARIANTS 4
#define TRICKS_SLEEP_FRAMES 0x1e
#define TRICKS_IDLE_MOTION 2
#define TRICKS_SLEEP_SOUND 0x356
#define TRICKS_SLEEP_PROMPT 0xf
#define TRICKS_SLEEP_PROMPT_FRAMES 0x3c
#define TRICKS_BLEND 5
#define TRICKS_FLOURISH_MOTION 6
#define TRICKS_FLOURISH_SOUND 0x1d6
#define TRICKS_SHOW_FROM 0xf
#define TRICKS_FLOURISH_SHOW_TO 0x1e
#define TRICKS_FLOURISH_END 0x2d
#define TRICKS_SPIN_MOTION 7
#define TRICKS_SPIN_SOUND 0x1d4
#define TRICKS_SPIN_SHOW_TO 0x19
#define TRICKS_SPIN_END 0x23
#define TRICKS_WAVE_MOTION 8
#define TRICKS_WAVE_SOUND 0x1d5
#define TRICKS_VOICE_FRAME 5
#define TRICKS_STRUT_MOTION 0x15
#define TRICKS_STRUT_FRAMES 0x3c
#define TRICKS_STRUT_END_MOTION 0x17
#define TRICKS_STRUT_END 0x46
#define TRICKS_DASH_MOTION 0x11
#define TRICKS_DASH_ROAR_FRAME 7
#define TRICKS_DASH_ROAR_SOUND 0x1c6
#define TRICKS_DASH_ROAR_SPEED 20.0f
#define TRICKS_DASH_ROAR_STRETCH 2.0f
#define TRICKS_DASH_FRAMES 0x14
#define TRICKS_DASH_STOP_FRAME 0x1e
#define TRICKS_DASH_ANGLE_RANGE 60
#define TRICKS_DASH_ANGLE_CENTRE 30
#define TRICKS_DASH_STEP_SLOW 4.0f
#define TRICKS_DASH_STEP_FAST 10.0f
#define TRICKS_DASH_STEP_BURST 48.0f
#define TRICKS_DASH_SLOW_FRAMES 5
#define TRICKS_DASH_RAMP_FRAMES 8
#define TRICKS_DASH_BURST_FRAMES 0x11
#define TRICKS_RETURN_MOTION 1
#define TRICKS_RETURN_STEP 12.0f
#define TRICKS_RETURN_SPEED 25.0f
#define TRICKS_HALF_TURN 3.1415f
#define TRICKS_DEGREE_DIVISOR 180.0f
#define TRICKS_SPARK_EFFECT 0x28a
#define TRICKS_SPARK_SOUND 0x1d7
#define TRICKS_SPARK_COOLDOWN 10
#define TRICKS_SPARK_HEIGHT 75.0f
#define TRICKS_CHEER_EFFECT 0x665
#define TRICKS_CHEER_SOUND 0x1d8
#define TRICKS_SPARK_SPREAD 50.0f
#define TRICKS_WAKE_RANGE 1500.0f

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1001b670(int index, float range, int count);               // Enemies_WakeNearby
extern "C" int FUN_10043cc0(int index);                                       // Char_ResetState
extern "C" int FUN_1000d620(int index, int key, int frames);                  // KeyPrompt_Show
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Length of an offset with its height counted twice.
inline float StretchedLength(Vec4 offset)
{
    offset.z = offset.z + offset.z;
    return offset.Length();
}

// 0x100b84a0 (Npc_PerformTricks). The character it plays to is slot `talk / 100` above its own (it must be a
// character slot 0x3a..0x5b that is not flagged in DAT_10210d30, else the state is reset). State 0 remembers its
// home position and turns to the neighbour. States 1..3 are flourishes (motions 6, 7, 8 with a voice line on frame 5
// and "gesturing" for a few frames so the neighbour can react), state 4 struts (motion 0x15 for 60 frames, then 0x17
// until frame 70), state 5 dashes off at a random angle (motion 0x11 for 20 frames with a roar and a speed boost on
// frame 7, then walks home at 12 units per frame); state 6 bows. State 100 picks the next trick at random (a bow
// 10 % of the time) or sleeps in state 101 once it saw the player. A character whose neighbour is gesturing cheers
// (state 99, motion 0x5a..0x5d, 13 frames) with a spark effect 0x665 above it; while it struts or flourishes it
// throws sparks 0x28a every 10 frames. It wakes the enemies within 1500 units and is flagged as having seen the
// player when the player comes within the talk id's units * 10 (or it was told to react). Returns 0.
int Npc_PerformTricks(int index)
{
    Vec4 player;
    Vec4 self;
    Vec4 neighbour;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float distance = StretchedLength(player - self);
    if (DAT_1020b430[index].state != TRICKS_STATE_START)
    {
        if (DAT_1020b430[index].neighbour < TRICKS_FIRST_NEIGHBOUR ||
            DAT_1020b430[index].neighbour >= TRICKS_NEIGHBOUR_LIMIT)
        {
            FUN_10043cc0(index);
            return 0;
        }
        neighbour.Copy(DAT_10db6ee8[DAT_1020b430[index].neighbour].matrix.GetTranslation());
        if (DAT_10210d30[DAT_1020b430[index].neighbour] != 0)
        {
            FUN_10043cc0(index);
            return 0;
        }
    }
    if (DAT_1020b430[index].state == TRICKS_STATE_START)
    {
        DAT_1020b430[index].neighbour = DAT_1020b430[index].talk / 100 + index;
        DAT_1020b430[index].state = TRICKS_STATE_FLOURISH;
        DAT_10802bf8[index].position.Copy(self);
        if (DAT_1020b430[index].neighbour < TRICKS_FIRST_NEIGHBOUR ||
            DAT_1020b430[index].neighbour >= TRICKS_NEIGHBOUR_LIMIT ||
            DAT_10210d30[DAT_1020b430[index].neighbour] != 0)
        {
            FUN_10043cc0(index);
            return 0;
        }
        DAT_10db6ee8[index].targetHeading =
            Atan2(DAT_10db6ee8[DAT_1020b430[index].neighbour].matrix.translation.x - self.x,
                  DAT_10db6ee8[DAT_1020b430[index].neighbour].matrix.translation.y - self.y) + TRICKS_HALF_TURN;
    }
    else if (DAT_1020b430[index].state == TRICKS_STATE_FLOURISH)
    {
        DAT_1020b430[index].wait++;
        DAT_10db6ee8[index].PlayMotion(TRICKS_FLOURISH_MOTION, 0, TRICKS_BLEND, 1.0f);
        if (DAT_1020b430[index].wait == 1)
            DAT_10db6ee8[index].PlayMotion(TRICKS_FLOURISH_MOTION, 1, TRICKS_BLEND, 1.0f);
        if (DAT_1020b430[index].wait == TRICKS_VOICE_FRAME)
            FUN_10065b10(index, TRICKS_FLOURISH_SOUND, 0, 0);
        if (DAT_1020b430[index].wait == TRICKS_SHOW_FROM)
            DAT_1020b430[index].gesturing = 1;
        if (DAT_1020b430[index].wait == TRICKS_FLOURISH_SHOW_TO)
            DAT_1020b430[index].gesturing = 0;
        if (DAT_1020b430[index].wait == TRICKS_FLOURISH_END)
            DAT_1020b430[index].state = TRICKS_STATE_DONE;
    }
    else if (DAT_1020b430[index].state == TRICKS_STATE_SPIN)
    {
        DAT_1020b430[index].wait++;
        DAT_10db6ee8[index].PlayMotion(TRICKS_SPIN_MOTION, 0, TRICKS_BLEND, 1.0f);
        if (DAT_1020b430[index].wait == 1)
            DAT_10db6ee8[index].PlayMotion(TRICKS_SPIN_MOTION, 1, TRICKS_BLEND, 1.0f);
        if (DAT_1020b430[index].wait == TRICKS_VOICE_FRAME)
            FUN_10065b10(index, TRICKS_SPIN_SOUND, 0, 0);
        if (DAT_1020b430[index].wait == TRICKS_SHOW_FROM)
            DAT_1020b430[index].gesturing = 1;
        if (DAT_1020b430[index].wait == TRICKS_SPIN_SHOW_TO)
            DAT_1020b430[index].gesturing = 0;
        if (DAT_1020b430[index].wait == TRICKS_SPIN_END)
            DAT_1020b430[index].state = TRICKS_STATE_DONE;
    }
    else if (DAT_1020b430[index].state == TRICKS_STATE_WAVE)
    {
        DAT_1020b430[index].wait++;
        DAT_10db6ee8[index].PlayMotion(TRICKS_WAVE_MOTION, 0, TRICKS_BLEND, 1.0f);
        if (DAT_1020b430[index].wait == 1)
            DAT_10db6ee8[index].PlayMotion(TRICKS_WAVE_MOTION, 1, TRICKS_BLEND, 1.0f);
        if (DAT_1020b430[index].wait == TRICKS_VOICE_FRAME)
            FUN_10065b10(index, TRICKS_WAVE_SOUND, 0, 0);
        if (DAT_1020b430[index].wait == TRICKS_SHOW_FROM)
            DAT_1020b430[index].gesturing = 1;
        if (DAT_1020b430[index].wait == TRICKS_FLOURISH_SHOW_TO)
            DAT_1020b430[index].gesturing = 0;
        if (DAT_1020b430[index].wait == TRICKS_FLOURISH_END)
            DAT_1020b430[index].state = TRICKS_STATE_DONE;
    }
    else if (DAT_1020b430[index].state == TRICKS_STATE_STRUT)
    {
        DAT_1020b430[index].wait++;
        if (DAT_1020b430[index].wait < TRICKS_STRUT_FRAMES)
        {
            DAT_10db6ee8[index].PlayMotion(TRICKS_STRUT_MOTION, 0, TRICKS_BLEND, 1.0f);
            DAT_1020b430[index].cheerReady = 1;
        }
        else if (DAT_1020b430[index].wait < TRICKS_STRUT_END)
        {
            DAT_1020b430[index].cheerReady = 0;
            DAT_10db6ee8[index].PlayMotion(TRICKS_STRUT_END_MOTION, 0, TRICKS_BLEND, 1.0f);
        }
        else
            DAT_1020b430[index].state = TRICKS_STATE_DONE;
    }
    else if (DAT_1020b430[index].state == TRICKS_STATE_DASH)
    {
        DAT_1020b430[index].wait++;
        DAT_1020b430[index].cheerReady = 2;
        Vec4 step;
        if (DAT_1020b430[index].wait < TRICKS_DASH_FRAMES)
        {
            DAT_10db6ee8[index].PlayMotion(TRICKS_DASH_MOTION, 0, TRICKS_BLEND, 1.0f);
            if (DAT_1020b430[index].wait == 1)
            {
                float angle = (float)(FUN_1013f880() % TRICKS_DASH_ANGLE_RANGE - TRICKS_DASH_ANGLE_CENTRE);
                DAT_10255160[index][0] =
                    angle * TRICKS_HALF_TURN / TRICKS_DEGREE_DIVISOR + DAT_10db6ee8[index].targetHeading;
            }
            if (DAT_1020b430[index].wait < TRICKS_DASH_SLOW_FRAMES)
            {
                step.y = TRICKS_DASH_STEP_SLOW;
                step.RotateZ(-DAT_10255160[index][0]);
            }
            else if (DAT_1020b430[index].wait < TRICKS_DASH_RAMP_FRAMES)
            {
                step.y = TRICKS_DASH_STEP_FAST;
                step.RotateZ(-DAT_10255160[index][0]);
                if (DAT_1020b430[index].wait == TRICKS_DASH_ROAR_FRAME)
                {
                    step.Copy(step * TRICKS_DASH_ROAR_STRETCH);
                    FUN_10065b10(index, TRICKS_DASH_ROAR_SOUND, 0, 0);
                    DAT_10db6ee8[index].AddMotionSpeed(TRICKS_DASH_ROAR_SPEED);
                }
            }
            else if (DAT_1020b430[index].wait < TRICKS_DASH_BURST_FRAMES)
            {
                step.y = TRICKS_DASH_STEP_BURST;
                step.RotateZ(-DAT_10255160[index][0]);
            }
            else
            {
                step.y = TRICKS_DASH_STEP_FAST;
                step.RotateZ(-DAT_10255160[index][0]);
            }
            FUN_100241e0(index, step.x, step.y, 0);
        }
        else
        {
            DAT_10db6ee8[index].PlayMotion(TRICKS_RETURN_MOTION, 0, TRICKS_BLEND, 1.0f);
            if (DAT_1020b430[index].wait < TRICKS_DASH_STOP_FRAME)
            {
                step.Fill(0.0f);
                DAT_10db6ee8[index].PlayMotion(TRICKS_IDLE_MOTION, 0, TRICKS_BLEND, 1.0f);
            }
            else
            {
                step.Copy(DAT_10802bf8[index].position - self);
                if (step.Length2D() > TRICKS_RETURN_STEP)
                {
                    step.z = 0.0f;
                    step.Normalized3();
                    step.Copy(step * TRICKS_RETURN_STEP);
                    DAT_10db6ee8[index].targetHeading = Atan2(step.x, step.y) + TRICKS_HALF_TURN;
                    if (DAT_10db6ee8[index].IsClearOfMapLimited(&step) == 0)
                        DAT_10db6ee8[index].AddMotionSpeed(TRICKS_RETURN_SPEED);
                }
                else
                {
                    DAT_1020b430[index].state = TRICKS_STATE_DONE;
                    DAT_10db6ee8[index].targetHeading =
                        Atan2(neighbour.x - self.x, neighbour.y - self.y) + TRICKS_HALF_TURN;
                    DAT_1020b430[index].cheerReady = 0;
                    if (DAT_10db6ee8[index].IsClearOfMapLimited(&step) == 0)
                        DAT_10db6ee8[index].AddMotionSpeed(TRICKS_RETURN_SPEED);
                }
            }
        }
        FUN_100241e0(index, step.x, step.y, 0);
    }
    else if (DAT_1020b430[index].state == TRICKS_STATE_BOW)
    {
        DAT_10db6ee8[index].PlayMotion(TRICKS_IDLE_MOTION, 0, TRICKS_BLEND, 1.0f);
        if (++DAT_1020b430[index].wait > TRICKS_BOW_FRAMES)
            DAT_1020b430[index].state = TRICKS_STATE_DONE;
    }
    else if (DAT_1020b430[index].state == TRICKS_STATE_CHEER)
    {
        DAT_1020b430[index].gesturing = 0;
        DAT_10db6ee8[index].PlayMotion(DAT_1020b430[index].cheerMotion, 0, TRICKS_BLEND, 1.0f);
        if (++DAT_1020b430[index].wait > TRICKS_CHEER_FRAMES)
            DAT_1020b430[index].state = TRICKS_STATE_DONE;
    }
    else if (DAT_1020b430[index].state == TRICKS_STATE_DONE)
    {
        DAT_1020b430[index].cheerReady = 0;
        if (DAT_1020b430[index].sawPlayer != 0)
        {
            DAT_1020b430[index].state = TRICKS_STATE_SLEEP;
            DAT_1020b430[index].wait = 0;
        }
        else
        {
            DAT_1020b430[index].state = FUN_1013f880() % TRICKS_TRICK_COUNT + 1;
            DAT_1020b430[index].wait = 0;
        }
        if (FUN_1013f880() % 100 < TRICKS_BOW_CHANCE && DAT_1020b430[index].state != TRICKS_STATE_SLEEP)
            DAT_1020b430[index].state = TRICKS_STATE_BOW;
    }
    else if (DAT_1020b430[index].state == TRICKS_STATE_SLEEP)
    {
        DAT_10db6ee8[index].PlayMotion(TRICKS_IDLE_MOTION, 0, TRICKS_BLEND, 1.0f);
        DAT_1020b430[index].wait++;
        if (DAT_1020b430[index].wait == 1)
        {
            FUN_10065b10(index, TRICKS_SLEEP_SOUND, 0, 0);
            FUN_1000d620(index, TRICKS_SLEEP_PROMPT, TRICKS_SLEEP_PROMPT_FRAMES);
            DAT_10db6ee8[index].targetHeading =
                Atan2(player.x - self.x, player.y - self.y) + TRICKS_HALF_TURN;
        }
        else if (DAT_1020b430[index].wait == TRICKS_SLEEP_FRAMES || DAT_10210b30[index] != 0)
            FUN_10043cc0(index);
    }
    if (DAT_1020b430[index].state != TRICKS_STATE_SLEEP)
    {
        if (DAT_1020b430[DAT_1020b430[index].neighbour].gesturing != 0 &&
            DAT_1020b430[index].state != TRICKS_STATE_CHEER)
        {
            if (DAT_1020b430[index].cheerReady == 0)
                DAT_1020b430[index].cheerPending = 1;
            else if (DAT_1020b430[index].cheerReady != 2 && DAT_1020b430[index].sparkCooldown == 0)
            {
                Matrix4 matrix;
                Vec4 offset;
                matrix.SetIdentity();
                offset.Copy(self);
                offset.z += TRICKS_SPARK_HEIGHT;
                offset.x = (float)(FUN_1013f880() % 100) - TRICKS_SPARK_SPREAD + offset.x;
                offset.y = (float)(FUN_1013f880() % 100) - TRICKS_SPARK_SPREAD + offset.y;
                offset.z = (float)(FUN_1013f880() % 100) - TRICKS_SPARK_SPREAD + offset.z;
                matrix = RotationZ(DAT_10db6ee8[index].heading);
                matrix.SetTranslation(offset);
                offset.Fill(0.0f);
                FUN_1001d8b0(index, TRICKS_SPARK_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0);
                DAT_1020b430[index].sparkCooldown = TRICKS_SPARK_COOLDOWN;
                FUN_10065b10(index, TRICKS_SPARK_SOUND, 0, 0);
            }
        }
        if (DAT_1020b430[index].cheerPending != 0)
        {
            DAT_1020b430[index].state = TRICKS_STATE_CHEER;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].cheerMotion = FUN_1013f880() % TRICKS_CHEER_VARIANTS + TRICKS_CHEER_MOTION;
            DAT_10db6ee8[index].PlayMotion(DAT_1020b430[index].cheerMotion, 1, TRICKS_BLEND, 1.0f);
            DAT_1020b430[index].cheerPending = 0;
            DAT_1020b430[index].gesturing = 0;
            Matrix4 matrix;
            Vec4 offset;
            matrix.SetIdentity();
            offset.Copy(self);
            offset.z += TRICKS_SPARK_HEIGHT;
            offset.x = (float)(FUN_1013f880() % 100) - TRICKS_SPARK_SPREAD + offset.x;
            offset.y = (float)(FUN_1013f880() % 100) - TRICKS_SPARK_SPREAD + offset.y;
            offset.z = (float)(FUN_1013f880() % 100) - TRICKS_SPARK_SPREAD + offset.z;
            matrix = RotationZ(DAT_10db6ee8[index].heading);
            matrix.SetTranslation(offset);
            offset.Fill(0.0f);
            FUN_1001d8b0(index, TRICKS_CHEER_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0);
            FUN_10065b10(index, TRICKS_CHEER_SOUND, 0, 0);
        }
        if (distance < (float)(DAT_1020b430[index].talk % 100 * 10) || DAT_10210b30[index] != 0)
        {
            FUN_1001b670(index, TRICKS_WAKE_RANGE, 0);
            DAT_1020b430[index].sawPlayer = 1;
            DAT_10210b30[index] = 0;
        }
    }
    if (DAT_1020b430[index].sparkCooldown > 0)
        DAT_1020b430[index].sparkCooldown--;
    return 0;
}
