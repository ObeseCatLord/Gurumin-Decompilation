// The map 0x17 mole game: per-frame update of the two moles (character slots 0x3a and 0x3b). C++ rather than extern "C":
// the function keeps an exception frame for its Matrix4 locals.

#include <math.h>
#include "vec4.h"
#include "matrix4.h"
#include "anim_model.h"
#include "node.h"
#include "char_state.h"
#include "char_tables.h"
#include "char_vars.h"
#include "char_hikari.h"
#include "doppler_loops.h"
#include "game_vars.h"
#include "map_vars.h"
#include "minigame_hud.h"
#include "mole_game.h"

// The game's atan2 wrapper (float arguments, so each difference is rounded to a float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

#define MOLE_SLOT_FIRST 0x3a
#define MOLE_SLOT_END 0x3c
#define HOLE_COUNT 9

// Banner sounds: effect 0x221 every 30 frames of the count-down, 0x222 when the start word shows (frame 0x5a).
#define COUNTDOWN_SOUND 0x221
#define START_SOUND 0x222
#define COUNTDOWN_FRAMES 0x5a
#define COUNTDOWN_SOUND_PERIOD 0x1e
#define SOUND_VOLUME 1.0f

// Mole behaviour steps (CharState::state).
#define MOLE_INIT 0
#define MOLE_RUN 1
#define MOLE_DIVE 2
#define MOLE_RISE 3

// Motion layers.
#define MOTION_IDLE 1
#define MOTION_DIVE 0xd
#define MOTION_HIT 7
#define MOTION_FINISH 2
#define MOTION_BLEND 5

// A hit pauses the mole for HIT_PAUSE_FRAMES and shows the score pop for SCORE_POP_FRAMES. In the first
// HIT_KICK_TIME_LIMIT frames' worth of timer it kicks up (HIT_KICK_LIFT) and may jump to another hole (a 100 in 500
// chance each).
#define HIT_PAUSE_FRAMES 5
#define SCORE_POP_FRAMES 0xf
#define HIT_KICK_TIME_LIMIT 900
#define HIT_KICK_LIFT 0x30
#define HIT_CHANCE_RANGE 500
#define HIT_CHANCE_BELOW 100
#define LIFT_DECAY 3

// Running: speed in units per frame (13 chasing, 8 otherwise), slowed in the last 900 timer ticks, faster after a
// hit; the difficulty setting DAT_10c635f8 scales it again. It dives once within DIVE_RANGE of a hole.
#define RUN_SPEED_CHASE 13.0f
#define RUN_SPEED 8.0f
#define RUN_SLOW_TIME_LIMIT 900
#define RUN_SLOW_SCALE 0.75f
#define RUN_PANIC_SCALE 1.5f
#define RUN_EASY_SCALE 0.75f
#define RUN_HARD_SCALE 1.25f
#define RUN_STEP_HEIGHT 10.0f
#define RUN_MIN_DISTANCE 1.0f
#define DIVE_WAIT_FRAMES 15
#define DIVE_HEIGHT_LIMIT 10.0f
#define DIVE_RANGE 50.0f
#define DIVE_START_WAIT -5
#define RISE_START_WAIT -30
#define DIVE_FRAMES 29
#define DIVE_BLEND_FRAMES 0x1f
#define SINK_SPEED -5.0f
#define RISE_SPEED 5.0f
#define UNDERGROUND_HEIGHT -150.0f
#define START_LIFT 0x27
#define START_PAUSE_FRAMES 30
#define DUST_EFFECT 0x2d5
#define DUST_HEIGHT_SCALE 0.25f
#define FACING_OFFSET 3.1415f
#define MOLE_HIT_RESUME_NONE -1

// 0x100524d0 (Map17_UpdateMoleGame). Runs the count-down banner sounds, then (while the map timer runs) both moles.
// A mole starts by finding the nine holes "@p1".."@p9" and the count_d texture and picks a hole. It runs towards its
// hole (MOLE_RUN, kicking up and avoiding walls) until it is within 50 units of any hole, dives into the hole it
// reaches (MOLE_DIVE, 30 frames, sinking and sliding into it with a dust effect), is moved underground to a new
// hole and rises there (MOLE_RISE) before it runs again. A hit (DAT_102550e0) scores a point, pauses the mole and
// restores its motion. Returns 0.
int Map17_UpdateMoleGame()
{
    if (DAT_1020c2e8 == 0 && DAT_10c66d14 == 4)
    {
        DAT_102028cc = -1;
        DAT_1020c2e8 = 1;
        DAT_102540ca = 0;
        DAT_102540cb = 0;
        FUN_1003ad20(COUNTDOWN_SOUND, 0, SOUND_VOLUME);
    }
    if (DAT_10c66d14 == 4 && DAT_1020c2e8 < COUNTDOWN_FRAMES)
    {
        if (DAT_1020c2e8 % COUNTDOWN_SOUND_PERIOD == 0)
            FUN_1003ad20(COUNTDOWN_SOUND, 0, SOUND_VOLUME);
        DAT_1020c2e8++;
        if (DAT_1020c2e8 == COUNTDOWN_FRAMES)
            FUN_1003ad20(START_SOUND, 0, SOUND_VOLUME);
    }
    if (DAT_10c6505c <= COUNTDOWN_FRAMES && DAT_10c6505c % COUNTDOWN_SOUND_PERIOD == 0)
        FUN_1003ad20(COUNTDOWN_SOUND, 0, SOUND_VOLUME);
    if (DAT_104c0135 == 0 && DAT_104bfe1c == 0 && DAT_1020c2ec != 0)
        DAT_1020c2ec--;
    if (DAT_1032d69c != 0 && DAT_104c0135 == 0 && DAT_104bfe1c == 0)
    {
        DAT_10c6505c--;
        Vec4 position;
        for (int slot = MOLE_SLOT_FIRST; slot < MOLE_SLOT_END; slot++)
        {
            DAT_1020d438[slot] = 1;
            position.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
            DAT_1021f3c0[slot] = 3;
            if (DAT_102550e0[slot] != 0)
            {
                DAT_1020c32c = DAT_10c65060 + 1;
                DAT_1020b430[slot].holdFrames = HIT_PAUSE_FRAMES;
                DAT_1020b430[slot].panicked = 1;
                int timeLeft = DAT_10c6505c;
                DAT_10c65060 = DAT_1020c32c;
                DAT_102550e0[slot] = 0;
                DAT_1020c2ec = SCORE_POP_FRAMES;
                if (timeLeft < HIT_KICK_TIME_LIMIT && DAT_1020b430[slot].state == MOLE_RUN)
                {
                    if (FUN_1013f880() % HIT_CHANCE_RANGE < HIT_CHANCE_BELOW)
                        DAT_1020b430[slot].lift = HIT_KICK_LIFT;
                    if (FUN_1013f880() % HIT_CHANCE_RANGE < HIT_CHANCE_BELOW)
                        DAT_1020b430[slot].holeIndex = FUN_1013f880() % HOLE_COUNT;
                }
                if (DAT_1020b430[slot].motionSaved != 0)
                {
                    if (DAT_1020b430[slot].savedLayer > -1)
                    {
                        DAT_10db6ee8[slot].PlayMotion(DAT_1020b430[slot].savedLayer, 1, 0, 1.0f);
                        DAT_10db6ee8[slot].motionFrame = DAT_1020b430[slot].savedFrame;
                        DAT_10db6ee8[slot].motion->m_0004 = (float)DAT_1020b430[slot].savedFrame;
                        DAT_10db6ee8[slot].motion->frame = DAT_1020b430[slot].savedFrame;
                        DAT_10db6ee8[slot].motion->layer = DAT_1020b430[slot].savedLayer;
                        DAT_10db6ee8[slot].motion->Advance();
                    }
                    else
                        DAT_10db6ee8[slot].PlayMotion(MOTION_HIT, 0, MOTION_BLEND, 1.0f);
                }
            }
            if (DAT_10315350[slot] == 0)
            {
                if (DAT_1020b430[slot].holdFrames != 0)
                    DAT_1020b430[slot].holdFrames--;
                else
                {
                    DAT_104686b0[slot] = 0;
                    if (DAT_1020b430[slot].state == MOLE_INIT)
                    {
                        if (DAT_10255f08 == 0)
                            return 0;
                        DAT_102028c8 = FUN_10161920("count_d");
                        if (DAT_102028c8 == -1)
                            DAT_102028c8 = FUN_1001e1a0("count_d.iti", 0);
                        DAT_10255a88[slot] = 0;
                        DAT_1021bdf8[slot][0] = DAT_10255f08->Find("@p1", 0);
                        DAT_1021bdf8[slot][1] = DAT_10255f08->Find("@p2", 0);
                        DAT_1021bdf8[slot][2] = DAT_10255f08->Find("@p3", 0);
                        DAT_1021bdf8[slot][3] = DAT_10255f08->Find("@p4", 0);
                        DAT_1021bdf8[slot][4] = DAT_10255f08->Find("@p5", 0);
                        DAT_1021bdf8[slot][5] = DAT_10255f08->Find("@p6", 0);
                        DAT_1021bdf8[slot][6] = DAT_10255f08->Find("@p7", 0);
                        DAT_1021bdf8[slot][7] = DAT_10255f08->Find("@p8", 0);
                        DAT_1021bdf8[slot][8] = DAT_10255f08->Find("@p9", 0);
                        int start = FUN_1013f880() % HOLE_COUNT;
                        DAT_1020b430[slot].state = MOLE_DIVE;
                        DAT_1020b430[slot].holdFrames = START_PAUSE_FRAMES;
                        DAT_1020b430[slot].holeIndex = start;
                    }
                    else if (DAT_1020b430[slot].state == MOLE_RUN)
                    {
                        DAT_1020b430[slot].wait++;
                        DAT_10db6ee8[slot].locked = 0;
                        DAT_10db6ee8[slot].PlayMotion(MOTION_IDLE, 0, MOTION_BLEND, 1.0f);
                        Vec4 step;
                        step.Copy(DAT_1021bdf8[slot][DAT_1020b430[slot].holeIndex]->matrix.GetTranslation() -
                                  DAT_10db6ee8[slot].matrix.GetTranslation());
                        if (step.Length2D() > RUN_MIN_DISTANCE)
                        {
                            step.z = 0;
                            step.Normalized3();
                            DAT_10db6ee8[slot].targetHeading = Atan2(step.x, step.y) + FACING_OFFSET;
                        }
                        else
                            step.Fill(0.0f);
                        if (DAT_10db6ee8[slot].m_238c != 0)
                        {
                            step.Copy(step * RUN_SPEED_CHASE);
                            if (DAT_10c6505c > RUN_SLOW_TIME_LIMIT)
                                step.Copy(step * RUN_SLOW_SCALE);
                            if (DAT_1020b430[slot].panicked != 0)
                                step.Copy(step * RUN_PANIC_SCALE);
                        }
                        else
                            step.Copy(step * RUN_SPEED);
                        if (DAT_10c635f8 == 1)
                            step.Copy(step * RUN_EASY_SCALE);
                        else if (DAT_10c635f8 == 2)
                            step.Copy(step * RUN_HARD_SCALE);
                        step.z = RUN_STEP_HEIGHT;
                        if (DAT_10db6ee8[slot].IsClearOfMapLimited(&step))
                        {
                            int lift = DAT_1020b430[slot].lift;
                            DAT_10db6ee8[slot].matrix.translation.x += step.x;
                            DAT_10db6ee8[slot].matrix.translation.y += step.y;
                            if (lift > 0)
                            {
                                DAT_1020b430[slot].lift = lift - LIFT_DECAY;
                                DAT_10db6ee8[slot].matrix.translation.z += (float)lift;
                            }
                        }
                        if (DAT_1020b430[slot].wait > DIVE_WAIT_FRAMES && position.z < DIVE_HEIGHT_LIMIT)
                        {
                            for (int hole = 0; hole < HOLE_COUNT; hole++)
                            {
                                if ((position - DAT_1021bdf8[slot][hole]->matrix.GetTranslation()).Length2D() < DIVE_RANGE)
                                {
                                    DAT_1020b430[slot].state = MOLE_DIVE;
                                    DAT_1020b430[slot].wait = DIVE_START_WAIT;
                                    DAT_1020b430[slot].holeIndex = hole;
                                    DAT_10db6ee8[slot].locked = 1;
                                    DAT_1020b430[slot].motionSaved = 1;
                                    break;
                                }
                            }
                        }
                    }
                    else if (DAT_1020b430[slot].state == MOLE_DIVE)
                    {
                        DAT_10db6ee8[slot].mode = 0;
                        DAT_10db6ee8[slot].PlayMotion(MOTION_DIVE, 0, MOTION_BLEND, 1.0f);
                        DAT_1020b430[slot].wait++;
                        if (DAT_1020b430[slot].wait >= 0)
                            DAT_10db6ee8[slot].matrix.translation.z = (float)DAT_1020b430[slot].wait * SINK_SPEED;
                        Node *hole = DAT_1021bdf8[slot][DAT_1020b430[slot].holeIndex];
                        DAT_10db6ee8[slot].m_238c = 1;
                        DAT_10db6ee8[slot].matrix.translation.x =
                            (hole->matrix.translation.x - DAT_10db6ee8[slot].matrix.translation.x) /
                                (float)(DIVE_BLEND_FRAMES - DAT_1020b430[slot].wait) +
                            DAT_10db6ee8[slot].matrix.translation.x;
                        DAT_10db6ee8[slot].matrix.translation.y =
                            (hole->matrix.translation.y - DAT_10db6ee8[slot].matrix.translation.y) /
                                (float)(DIVE_BLEND_FRAMES - DAT_1020b430[slot].wait) +
                            DAT_10db6ee8[slot].matrix.translation.y;
                        Vec4 offset;
                        Matrix4 matrix;
                        matrix.SetIdentity();
                        matrix.SetTranslation(DAT_10db6ee8[slot].matrix.GetTranslation());
                        matrix.translation.z *= DUST_HEIGHT_SCALE;
                        FUN_1001d8b0(slot, DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0);
                        if (DAT_1020b430[slot].wait > DIVE_FRAMES)
                        {
                            DAT_1020b430[slot].state = MOLE_RISE;
                            DAT_1020b430[slot].wait = RISE_START_WAIT;
                            DAT_1020b430[slot].motionSaved = 0;
                            DAT_10db6ee8[slot].mode = 1;
                            int risen = FUN_1013f880() % HOLE_COUNT;
                            DAT_10db6ee8[slot].SetPosition(DAT_1021bdf8[slot][risen]->matrix.translation.x,
                                                           DAT_1021bdf8[slot][risen]->matrix.translation.y,
                                                           UNDERGROUND_HEIGHT);
                            int next;
                            do
                            {
                                next = FUN_1013f880() % HOLE_COUNT;
                                DAT_1020b430[slot].holeIndex = next;
                            } while (next == risen);
                        }
                    }
                    else if (DAT_1020b430[slot].state == MOLE_RISE)
                    {
                        DAT_10db6ee8[slot].PlayMotion(MOTION_IDLE, 0, MOTION_BLEND, 1.0f);
                        DAT_1020b430[slot].wait++;
                        if (DAT_1020b430[slot].wait < 0)
                        {
                            DAT_10db6ee8[slot].m_238c = 1;
                            DAT_10db6ee8[slot].matrix.translation.z = (float)DAT_1020b430[slot].wait * RISE_SPEED;
                        }
                        else if (DAT_1020b430[slot].wait == 0)
                        {
                            DAT_10db6ee8[slot].SetPosition(DAT_10db6ee8[slot].matrix.translation.x,
                                                           DAT_10db6ee8[slot].matrix.translation.y, 0);
                            DAT_1020b430[slot].state = MOLE_RUN;
                            DAT_1020b430[slot].wait = 0;
                            DAT_1020b430[slot].lift = START_LIFT;
                            DAT_1020b430[slot].panicked = 0;
                        }
                        Vec4 step;
                        step.Copy(DAT_1021bdf8[slot][DAT_1020b430[slot].holeIndex]->matrix.GetTranslation() -
                                  DAT_10db6ee8[slot].matrix.GetTranslation());
                        step.z = 0;
                        step.Normalized3();
                        DAT_10db6ee8[slot].targetHeading = Atan2(step.x, step.y) + FACING_OFFSET;
                        Vec4 offset;
                        Matrix4 matrix;
                        matrix.SetIdentity();
                        matrix.SetTranslation(DAT_10db6ee8[slot].matrix.GetTranslation());
                        matrix.translation.z *= DUST_HEIGHT_SCALE;
                        FUN_1001d8b0(slot, DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0);
                    }
                }
                if (DAT_10db6ee8[slot].motion->blending != 0)
                    DAT_1020b430[slot].savedLayer = MOLE_HIT_RESUME_NONE;
                else
                    DAT_1020b430[slot].savedLayer = DAT_10db6ee8[slot].motion->layer;
                DAT_1020b430[slot].savedFrame = (int)(DAT_10db6ee8[slot].motion->m_0004 + 0.5f);
                if (DAT_10c6505c == 0)
                    DAT_10db6ee8[slot].PlayMotion(MOTION_FINISH, 1, MOTION_BLEND, 1.0f);
            }
        }
    }
    return 0;
}
