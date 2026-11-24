// The pod lobber (0x10070200, a per-kind behaviour of 0x100f5a80 and 0x100f9670): a plant-like enemy whose model has
// six pod nodes (Dummy06..Dummy11). Pods ripen on the nodes (a timer started at 1, glowing once it passes 29), and the
// enemy lobs the ripe ones at the player in an arc (state 2, one pod a frame) or bursts them all at once in a big
// attack (state 3). A lobbed pod is one of the six matrices DAT_10868838[index][0..5]: its movement is the first three
// words, m[3] its age (a pod that is older than 0.9 is flying, one that reaches 65 is dropped) and m[6] its gravity.
// See Enemy_PodLobber for the states.

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
#include "map_vars.h"
#include "world.h"

#include "doppler_loops.h"
#include "enemy_chase_hop.h"
#include "hop_spit_enemy.h"
#include "route_flier.h"
#include "target_race.h"

#include "pod_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40

inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

#define POD_COUNT 6
#define POD_FIRST_WORD 6                   // CharState::words index of the timer of pod 0 (pod k is words[6 + k])
#define POD_NODE_HEAD 6                    // DAT_1021bdf8[index]: pods 0..5, then the head, Bip01 and Bone01 nodes
#define POD_NODE_BIP 7
#define POD_NODE_BONE 8
#define POD_STATE_GROW 0                   // fading in
#define POD_STATE_IDLE 1
#define POD_STATE_LOB 2                    // lobs the ripe pods one at a time
#define POD_STATE_BURST 3                  // bursts all ripe pods and the head effects
#define POD_STATE_FADE 4                   // fading out after the map moved on
#define POD_MOTION_IDLE 0
#define POD_MOTION_BURST 1
#define POD_MOTION_GROW 2
#define POD_MOTION_LOB 4
#define POD_MOTION_BLEND 5                 // PlayMotion blend frames
#define POD_GROWTH_FULL 150                // CharState::podGrowth when the enemy is fully there
#define POD_GROWTH_FADE_FRAMES 30
#define POD_GROWTH_ALPHA_FRAMES 15.0f
#define POD_FLICKER_LEVEL 9                // DAT_102550e0 value that makes the model flicker
#define POD_FLICKER_FRAMES 30
#define POD_FLICKER_CHANCE 20              // out of 10000 a frame
#define POD_APPEAR_RANGE 600.0f
#define POD_ACT_RANGE 1200.0f
#define POD_NEAR_RANGE 400.0f              // x/y distance at which it may burst
#define POD_BURST_CHANCE 80                // out of 1000 a frame inside POD_NEAR_RANGE
#define POD_LOB_START_FRAMES 60
#define POD_LOB_END_FRAMES 30
#define POD_BURST_MOTION_FRAME 0x90        // motion frame of the burst at which the pods fly
#define POD_BURST_END 99
#define POD_BURST_GLOW_FROM 12
#define POD_BURST_GLOW_TO 72
#define POD_PARTNER_FIRST 0x3a             // slots of its own kind are looked for from here
#define POD_PARTNER_END 0x5c
#define POD_PARTNER_FAR_SQUARED 1000000.0f
#define POD_PARTNER_NEAR 1000.0f
#define POD_PARTNER_CHANCE 10              // out of 100
#define POD_PARTNER_PERIOD 10              // motion frames between looks for partners
#define POD_RIPE_TIMER 30                  // a pod that is above this is ripe
#define POD_NEW_CHANCE 20                  // a pod starts ripening with a chance of this times podRipeness in 1000
#define POD_RIPENING_MAX 3                 // no new pod starts while this many are growing
#define POD_LOB_PERIOD 20
#define POD_LOB_PICK 18
#define POD_LOB_SPEED_SCALE 0.75f
#define POD_BURST_PERIOD 30
#define POD_BURST_SPEED_SCALE 0.5f
#define POD_LAUNCH_Y -25.0f
#define POD_PIECE_LIFT 30.0f
#define POD_PIECE_BORN 1.0f
#define POD_PIECE_COUNT 6
#define POD_PIECE_LIFE_MIN 0.9f
#define POD_PIECE_LIFE_STEP 1.0f
#define POD_PIECE_LIFE_END 65.0f
#define POD_PIECE_PROBE_DEPTH 35.0f
#define POD_PIECE_GRAVITY -1.5f
#define POD_PIECE_BOUNCE_SIDE 0.75f
#define POD_PIECE_BOUNCE_UP -0.5f
#define POD_PIECE_FLOOR_LIFT 30.0f
#define POD_PIECE_LOOKAHEAD 3.0f
#define POD_PIECE_REACH 6.0f
#define POD_PIECE_FALL_LIMIT -30.0f
#define POD_PIECE_SOLID_PERIOD 4
#define POD_PIECE_MASK 4
#define POD_NO_HIT -200000.0f
#define POD_WALK_BACK -6.0f
#define POD_PI 3.1415f
#define POD_PI_PRECISE 3.14159f
#define POD_TWO_PI 6.283f
#define POD_HALF_TURN 180.0f
#define POD_TURN_EASE 4.5f
#define POD_SCALE 0.5f
#define POD_SPAWN_EFFECT 0x6aa             // when it appears (0x6c4 in the second costume)
#define POD_SPAWN_EFFECT_ALT 0x6c4
#define POD_HIDE_EFFECT 0x6ab              // when it fades out (0x6c5)
#define POD_HIDE_EFFECT_ALT 0x6c5
#define POD_GROUND_EFFECT 0x6ac            // while it is hidden (0x6c6)
#define POD_GROUND_EFFECT_ALT 0x6c6
#define POD_POP_EFFECT 0x686               // a pod pops when the model fades out
#define POD_SPROUT_EFFECT 0x684            // a pod that just started ripening
#define POD_RIPE_EFFECT 0x682              // a ripe pod
#define POD_FLIGHT_EFFECT 0x688            // the effect that follows a lobbed pod
#define POD_BURST_HEAD_EFFECT 0x450
#define POD_BURST_RING_EFFECT_A 0x44c
#define POD_BURST_RING_EFFECT_B 0x44d
#define POD_BURST_RING_EFFECT_C 0x44f
#define POD_BURST_RING_FRAME 3
#define POD_BURST_SHADOW_LIFT 5.0f
#define POD_LOB_SOUND 0x2bf
#define POD_BURST_SOUND 0x2c0
#define POD_LOB_OPTION_SCALE 1.2f
#define POD_BURST_OPTION_SCALE 1.5f
#define POD_BURST_CAPTION "829182"
#define POD_BURST_LOUD_SQUARED 6250000.0f
#define POD_NODE_KIND_GLOW 0x22
#define POD_NODE_KIND_PLAIN 0x2f
#define POD_NODE_MASK_A 0x400
#define POD_NODE_MASK_B 0x200

// 0x10070200 (Enemy_PodLobber; C++ linkage: the original has an EH frame). One frame of the pod lobber `index`.
// First every lobbed pod in DAT_10868838[index] moves: a pod aged over 0.9 grows older by one (the count of those
// that fly goes to podPiecesLive), falls with gravity -1.5 (at most 30 a frame), bounces (x/y by 0.75, z by -0.5) off
// the floor below it, stops against walls and is dropped at age 65. A character that is defeated (DAT_10210d30)
// returns 1; one that is not yet placed (DAT_104483e8 == 0) returns 0 without doing anything else. Then the states
// (CharState::state):
// 0: the model fades in (alpha growth / 15) and the appear effect starts the first time the player is within 600;
//    at growth 30 it becomes state 1.
// 1: idle, walking back at 6 units a frame (except during the motion frames of its attacks); within 1200 of the
//    player a pod starts ripening now and then (at most 3 ripening at once); it turns to the player, and after 60
//    frames a ripe pod (chance 1 % a ripe pod) starts the lob (state 2), or a player closer than 400 in x/y starts the
//    burst (state 3, 8 % a frame). Out of range the growth counts down and the model fades out (state 4).
// 2: lobs ripe pods, step 7..12, 90 % a pod; each pod flies as one of the six lobbed matrices with a random
//    angle; at step 30 back to state 1.
// 3: at motion frame 0x90 all ripe pods burst (flying slower) and the head effects start; the camera caption and
//    knock-back of the player follow; after 99 frames back to state 1.
// 4: the model fades out with growth 30 down to 0, then state 0.
// Level 9 enemies (DAT_102550e0 == 9) flicker: they hide for 30 frames now and then. At the end it starts the pod
// effects, keeps the model's turning towards its heading and plays its motion. Returns 0 (1 when defeated).
int Enemy_PodLobber(int index)
{
    DAT_10db6ee8[index].uniformScale = POD_SCALE;
    DAT_1020b430[index].podPiecesLive = 0;
    Matrix4 *flying = DAT_10868838[index];
    for (int remaining = POD_PIECE_COUNT; remaining != 0; remaining--, flying++)
    {
        Matrix4 &piece = *flying;
        if (piece.m[3] > POD_PIECE_LIFE_MIN)
        {
            DAT_1020b430[index].podPiecesLive++;
            float age = piece.m[3] + POD_PIECE_LIFE_STEP;
            piece.m[3] = age;
            if (age >= POD_PIECE_LIFE_END)
                piece.SetIdentity();
            else
            {
                int landed = 0;
                Vec4 at;
                Vec4 below;
                Vec4 hit;
                hit.x = POD_NO_HIT;
                at.Copy(piece.GetTranslation());
                below.Copy(at);
                below.z = below.z - POD_PIECE_PROBE_DEPTH;
                if (DAT_10255f08 != 0 && DAT_10255f08->CastSegment(0, POD_PIECE_MASK, &below, &at, &hit, 0) != 0)
                    landed = 1;
                piece.m[6] = POD_PIECE_GRAVITY;
                if (landed)
                {
                    if (piece.m[2] < 0.0f)
                    {
                        piece.m[0] = piece.m[0] * POD_PIECE_BOUNCE_SIDE;
                        piece.m[1] = piece.m[1] * POD_PIECE_BOUNCE_SIDE;
                        piece.m[2] = piece.m[2] * POD_PIECE_BOUNCE_UP;
                    }
                    float floor = hit.z + POD_PIECE_FLOOR_LIFT;
                    if (piece.m[14] < floor)
                        piece.m[14] = floor;
                }
                hit.x = POD_NO_HIT;
                at.Copy(piece.GetTranslation());
                at.x = at.x - piece.m[0] * POD_PIECE_LOOKAHEAD;
                at.y = at.y - piece.m[1] * POD_PIECE_LOOKAHEAD;
                float zero = 0.0f;
                below.Fill(zero);
                below.x = piece.m[0] * POD_PIECE_REACH;
                below.y = piece.m[1] * POD_PIECE_REACH;
                below.Copy(at + below);
                if ((int)piece.m[3] % POD_PIECE_SOLID_PERIOD == 0 && DAT_10255f08 != 0 &&
                    DAT_10255f08->Collide(0, POD_PIECE_MASK, &below, &at, &hit) != 0)
                {
                    piece.m[0] = 0.0f;
                    piece.m[1] = 0.0f;
                }
                float fall = piece.m[6] + piece.m[2];
                piece.m[2] = fall;
                if (fall < POD_PIECE_FALL_LIMIT)
                    piece.m[2] = POD_PIECE_FALL_LIMIT;
                piece.m[12] = piece.m[12] + piece.m[0];
                piece.m[13] = piece.m[13] + piece.m[1];
                piece.m[14] = piece.m[14] + piece.m[2];
            }
        }
    }
    if (DAT_10210d30[index] != 0)
        return 1;
    if (DAT_104483e8[index] != 0)
    {
        DAT_10255a88[index] = 0;
        if (DAT_102550e0[index] == POD_FLICKER_LEVEL)
        {
            if (DAT_1020b430[index].podGrowth > POD_GROWTH_FADE_FRAMES)
            {
                DAT_1020b430[index].state = POD_STATE_IDLE;
                DAT_1020b430[index].podFrames = 0;
            }
            if (DAT_1020b430[index].podGrowth == 0)
                DAT_1020b430[index].podGrowth = 1;
            if (FUN_1013f880() % 10000 < POD_FLICKER_CHANCE)
                DAT_1020b430[index].podHideFrames = POD_FLICKER_FRAMES;
        }
        int hideFrames = DAT_1020b430[index].podHideFrames;
        DAT_10218fc0[index] = 0;
        if (hideFrames != 0)
        {
            DAT_10daaae8[index].m_0141 = 1;
            DAT_1020b430[index].podHideFrames = hideFrames - 1;
        }
        else
            DAT_10daaae8[index].m_0141 = 0;
        if (DAT_1020b430[index].podNodesFound == 0)
        {
            DAT_1020b430[index].podNodesFound = 1;
            DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find("Dummy06", 0);
            DAT_1021bdf8[index][1] = DAT_10db6ee8[index].root->Find("Dummy07", 0);
            DAT_1021bdf8[index][2] = DAT_10db6ee8[index].root->Find("Dummy08", 0);
            DAT_1021bdf8[index][3] = DAT_10db6ee8[index].root->Find("Dummy09", 0);
            DAT_1021bdf8[index][4] = DAT_10db6ee8[index].root->Find("Dummy10", 0);
            DAT_1021bdf8[index][5] = DAT_10db6ee8[index].root->Find("Dummy11", 0);
            DAT_1021bdf8[index][POD_NODE_HEAD] = DAT_10db6ee8[index].root->Find("Bip01 Head", 0);
            DAT_1021bdf8[index][POD_NODE_BIP] = DAT_10db6ee8[index].root->Find("Bip01", 0);
            DAT_1021bdf8[index][POD_NODE_BONE] = DAT_10db6ee8[index].root->Find("Bone01", 0);
            float heading = (float)(FUN_1013f880() % 360) / POD_HALF_TURN * POD_PI;
            DAT_10db6ee8[index].targetHeading = heading;
            DAT_10db6ee8[index].heading = heading;
        }
        Vec4 playerPosition;
        Vec4 selfPosition;
        Vec4 toPlayer;
        Vec4 walk;
        Matrix4 pose;
        Vec4 effectPosition;
        EffectOptions options;
        playerPosition.Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
        selfPosition.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        toPlayer.Copy(playerPosition - selfPosition);
        float distance = toPlayer.Length();
        float distance2D = toPlayer.Length2D();
        if (DAT_1020b430[index].podGrowth == POD_GROWTH_FULL && DAT_10db6ee8[index].motionFrame % POD_PARTNER_PERIOD == 0)
        {
            Vec4 otherPosition;
            for (int other = POD_PARTNER_FIRST; other < POD_PARTNER_END; other++)
            {
                if (DAT_10daaae8[index].kind == DAT_10daaae8[other].kind && other != index)
                {
                    otherPosition.Copy(DAT_10db6ee8[other].matrix.GetTranslation());
                    if (DAT_1030b8f8[other] != 0 ||
                        (playerPosition - otherPosition).LengthSquared3() < POD_PARTNER_FAR_SQUARED)
                    {
                        if ((selfPosition - otherPosition).Length2D() < POD_PARTNER_NEAR &&
                            FUN_1013f880() % 100 < POD_PARTNER_CHANCE)
                        {
                            if (DAT_1020b430[index].podGrowth > POD_RIPE_TIMER)
                                DAT_1020b430[index].podGrowth = POD_GROWTH_FULL;
                            else if (DAT_1020b430[index].podGrowth == 0)
                                DAT_1020b430[index].podGrowth = 1;
                        }
                    }
                }
            }
        }
        DAT_10db6ee8[index].mode = 1;
        float walkZero = 0.0f;
        walk.Fill(walkZero);
        DAT_1020b430[index].podMotion = POD_MOTION_GROW;
        if (DAT_1021bbf8[index] != 0)
        {
            DAT_1020b430[index].state = POD_STATE_FADE;
            DAT_1020b430[index].podGrowth = 0;
            for (int pod = 0; pod < POD_COUNT; pod++)
            {
                if (DAT_1020b430[index].words[POD_FIRST_WORD + pod] != 0)
                    DAT_1020b430[index].words[POD_FIRST_WORD + pod] = 0;
            }
            DAT_1021bbf8[index] = 0;
        }
        if (DAT_102550e0[index] == 0)
        {
            CharState &state = DAT_1020b430[index];
            if (state.state == POD_STATE_GROW)
            {
                if (distance < POD_APPEAR_RANGE && state.podGrowth == 0)
                {
                    EffectOptions appearOptions;
                    appearOptions.flags |= 2;
                    state.podGrowth = 1;
                    pose.SetIdentity();
                    pose = RotationZ(DAT_10db6ee8[index].heading);
                    Matrix4 &self = DAT_10db6ee8[index].matrix;
                    pose.SetTranslation(self.GetTranslation());
                    float zero = 0.0f;
                    effectPosition.Fill(zero);
                    if (DAT_10c655e0 != 0)
                        FUN_1001d8b0(index, POD_SPAWN_EFFECT_ALT, effectPosition, -1, -1, &self, 0, 0, &pose,
                                     &appearOptions);
                    else
                        FUN_1001d8b0(index, POD_SPAWN_EFFECT, effectPosition, -1, -1, &self, 0, 0, &pose,
                                     &appearOptions);
                }
                if (state.podGrowth < POD_GROWTH_FADE_FRAMES && state.podGrowth != 0)
                {
                    int growth = state.podGrowth + 1;
                    state.podGrowth = growth;
                    if (growth < 15)
                        DAT_10db6ee8[index].alpha = (float)growth / POD_GROWTH_ALPHA_FRAMES;
                    else
                        DAT_10db6ee8[index].alpha = 1.0f;
                    if (growth == POD_GROWTH_FADE_FRAMES)
                    {
                        state.state = POD_STATE_IDLE;
                        state.podGrowth = POD_GROWTH_FULL;
                    }
                    state.podMotion = POD_MOTION_GROW;
                    state.podMotion = POD_MOTION_GROW;
                }
                else
                {
                    DAT_10db6ee8[index].alpha = 0.0f;
                    state.podMotion = POD_MOTION_GROW;
                }
            }
            else if (state.state == POD_STATE_IDLE)
            {
                int frame = DAT_10db6ee8[index].motionFrame;
                DAT_10db6ee8[index].alpha = 1.0f;
                state.podMotion = POD_MOTION_IDLE;
                if ((frame <= 0x52 || frame >= 0x5c) && (frame <= 0x76 || frame >= 0x80) && (frame <= 0x36 || frame >= 0x3a))
                {
                    walk.y = POD_WALK_BACK;
                    walk.RotateZ(-DAT_10db6ee8[index].heading);
                }
                if (distance > POD_ACT_RANGE)
                {
                    int growth = state.podGrowth - 1;
                    state.podGrowth = growth;
                    if (growth == POD_GROWTH_FADE_FRAMES - 1)
                    {
                        state.state = POD_STATE_FADE;
                        EffectOptions popOptions;
                        popOptions.flags |= 2;
                        int pod = 6;
                        int *timer = &state.words[6];
                        do
                        {
                            if (*timer != 0)
                            {
                                pose.SetIdentity();
                                pose.SetTranslation(DAT_1021bdf8[index][pod - 6]->worldMatrix.GetTranslation());
                                float zero = 0.0f;
                                effectPosition.Fill(zero);
                                DAT_10daaa80.Start(POD_POP_EFFECT, effectPosition, -1, -1, 0, 0, 0, &pose, &popOptions,
                                                   &DAT_10219770[index]);
                                *timer = 0;
                            }
                            pod++;
                            timer++;
                        } while (pod < 12);
                        pose.SetIdentity();
                        pose = RotationZ(DAT_10db6ee8[index].heading);
                        Matrix4 &self = DAT_10db6ee8[index].matrix;
                        pose.SetTranslation(self.GetTranslation());
                        float zero = 0.0f;
                        effectPosition.Fill(zero);
                        if (DAT_10c655e0 != 0)
                            DAT_10daaa80.Start(POD_HIDE_EFFECT_ALT, effectPosition, -1, -1, &self, 0, 0, &pose,
                                               &popOptions, &DAT_10219770[index]);
                        else
                            DAT_10daaa80.Start(POD_HIDE_EFFECT, effectPosition, -1, -1, &self, 0, 0, &pose, &popOptions,
                                               &DAT_10219770[index]);
                    }
                }
                else
                {
                    state.podRipeness = state.podRipeness + 1;
                    int growing = 0;
                    if (state.words[6] != 0)
                        growing = 1;
                    if (state.words[7] != 0)
                        growing++;
                    if (state.words[8] != 0)
                        growing++;
                    if (state.words[9] != 0)
                        growing++;
                    if (state.words[10] != 0)
                        growing++;
                    if (state.words[11] != 0)
                        growing++;
                    int word = 6;
                    do
                    {
                        if (FUN_1013f880() % 3 == 0)
                        {
                            if (growing < POD_RIPENING_MAX && FUN_1013f880() % 1000 < state.podRipeness * POD_NEW_CHANCE)
                            {
                                state.words[word] = 1;
                                state.podRipeness = 0;
                            }
                            break;
                        }
                        word++;
                    } while (word < 12);
                }
                DAT_10db6ee8[index].targetHeading = Atan2(toPlayer.x, toPlayer.y) + POD_PI;
                int step = state.podFrames + 1;
                state.podFrames = step;
                if (step <= POD_LOB_START_FRAMES)
                {
                    if (distance2D < POD_NEAR_RANGE && FUN_1013f880() % 1000 < POD_BURST_CHANCE)
                    {
                        state.podFrames = 0;
                        state.state = POD_STATE_BURST;
                    }
                }
                else
                {
                    int ripe = 0;
                    if (state.words[6] > POD_RIPE_TIMER)
                        ripe = 1;
                    if (state.words[7] > POD_RIPE_TIMER)
                        ripe++;
                    if (state.words[8] > POD_RIPE_TIMER)
                        ripe++;
                    if (state.words[9] > POD_RIPE_TIMER)
                        ripe++;
                    if (state.words[10] > POD_RIPE_TIMER)
                        ripe++;
                    if (state.words[11] > POD_RIPE_TIMER)
                        ripe++;
                    if (ripe > 0 && FUN_1013f880() % 1000 < ripe * 10)
                    {
                        state.podFrames = 0;
                        state.state = POD_STATE_LOB;
                    }
                }
            }
            else if (state.state == POD_STATE_LOB)
            {
                DAT_10db6ee8[index].targetHeading = Atan2(toPlayer.x, toPlayer.y) + POD_PI;
                int step = state.podFrames + 1;
                state.podFrames = step;
                state.podMotion = POD_MOTION_LOB;
                if (step > 6 && step < 13 && state.words[step] > POD_RIPE_TIMER &&
                    FUN_1013f880() % POD_LOB_PERIOD < POD_LOB_PICK)
                {
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0000 = 0;
                    options.m_0008 = POD_LOB_OPTION_SCALE;
                    float zero = 0.0f;
                    effectPosition.Fill(zero);
                    effectPosition.y = POD_LAUNCH_Y;
                    effectPosition.RotateX((float)(FUN_1013f880() % 60 - 20) / POD_HALF_TURN * POD_PI_PRECISE);
                    effectPosition.RotateZ((float)(FUN_1013f880() % 200 - 100) / 1800.0f * POD_PI_PRECISE -
                                           DAT_10db6ee8[index].heading);
                    Matrix4 &piece = DAT_10868838[index][step - 6];
                    piece.m[0] = effectPosition.x;
                    piece.m[2] = effectPosition.z * POD_LOB_SPEED_SCALE;
                    piece.m[1] = effectPosition.y;
                    piece.m[3] = POD_PIECE_BORN;
                    piece.m[6] = 0.0f;
                    piece.SetTranslation(DAT_1021bdf8[index][step - 6]->worldMatrix.GetTranslation());
                    piece.m[14] = piece.m[14] - POD_PIECE_LIFT;
                    float zeroAgain = 0.0f;
                    effectPosition.Fill(zeroAgain);
                    FUN_1001d8b0(index, POD_FLIGHT_EFFECT, effectPosition, -1, -1, &piece, 0, 0, 0, &options);
                    state.words[step] = 0;
                    FUN_10065b10(index, POD_LOB_SOUND, 0, 0);
                }
                if (state.podFrames == POD_LOB_END_FRAMES)
                {
                    state.podFrames = 0;
                    state.state = POD_STATE_IDLE;
                }
            }
            else if (state.state == POD_STATE_BURST)
            {
                int frame = DAT_10db6ee8[index].motionFrame;
                state.podFrames = state.podFrames + 1;
                state.podMotion = POD_MOTION_BURST;
                if (frame == POD_BURST_MOTION_FRAME)
                {
                    int thrown = 0;
                    for (int word = 6; word < 12; word++)
                    {
                        if (state.words[word] > POD_RIPE_TIMER && FUN_1013f880() % POD_BURST_PERIOD < POD_LOB_PICK)
                        {
                            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                            options.m_0000 = 0;
                            options.m_0008 = POD_LOB_OPTION_SCALE;
                            float zero = 0.0f;
                            effectPosition.Fill(zero);
                            effectPosition.y = POD_LAUNCH_Y;
                            effectPosition.RotateX((float)(FUN_1013f880() % 20 + 60) / POD_HALF_TURN * POD_PI_PRECISE);
                            effectPosition.RotateZ((float)(FUN_1013f880() % 2700 - 1350) / 1800.0f * POD_PI_PRECISE -
                                                   DAT_10db6ee8[index].heading);
                            Matrix4 &piece = DAT_10868838[index][word - 6];
                            piece.m[0] = effectPosition.x;
                            piece.m[2] = effectPosition.z * POD_BURST_SPEED_SCALE;
                            piece.m[1] = effectPosition.y;
                            piece.m[3] = POD_PIECE_BORN;
                            piece.m[6] = 0.0f;
                            piece.SetTranslation(DAT_1021bdf8[index][word - 6]->worldMatrix.GetTranslation());
                            piece.m[14] = piece.m[14] - POD_PIECE_LIFT;
                            float zeroAgain = 0.0f;
                            effectPosition.Fill(zeroAgain);
                            FUN_1001d8b0(index, POD_FLIGHT_EFFECT, effectPosition, -1, -1, &piece, 0, 0, 0, &options);
                            state.words[word] = 0;
                            if (!thrown)
                            {
                                thrown = 1;
                                FUN_10065b10(index, POD_LOB_SOUND, 0, 0);
                            }
                        }
                    }
                    pose.SetIdentity();
                    pose.SetTranslation(DAT_1021bdf8[index][POD_NODE_HEAD]->worldMatrix.GetTranslation());
                    float zero = 0.0f;
                    effectPosition.Fill(zero);
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0000 = 0;
                    options.m_0008 = POD_BURST_OPTION_SCALE;
                    options.owner = index;
                    DAT_10daaa80.Start(POD_BURST_HEAD_EFFECT, effectPosition, -1, -1,
                                       &DAT_1021bdf8[index][POD_NODE_HEAD]->worldMatrix, 0, 0, &pose, &options,
                                       &DAT_10219770[index]);
                    pose.m[14] = DAT_10db6ee8[index].shadowMatrix.translation.z + POD_BURST_SHADOW_LIFT;
                    DAT_10daaa80.Start(POD_BURST_RING_EFFECT_A, effectPosition, -1, -1, 0, 0, POD_BURST_RING_FRAME, &pose,
                                       &options, &DAT_10219770[index]);
                    DAT_10daaa80.Start(POD_BURST_RING_EFFECT_B, effectPosition, -1, -1, 0, 0, POD_BURST_RING_FRAME, &pose,
                                       &options, &DAT_10219770[index]);
                    DAT_10daaa80.Start(POD_BURST_RING_EFFECT_C, effectPosition, -1, -1, 0, 0, POD_BURST_RING_FRAME, &pose,
                                       &options, &DAT_10219770[index]);
                    pose.SetTranslation(DAT_1021bdf8[index][POD_NODE_BIP]->worldMatrix.GetTranslation());
                    DAT_10daaa80.Start(POD_BURST_HEAD_EFFECT, effectPosition, -1, -1,
                                       &DAT_1021bdf8[index][POD_NODE_BIP]->worldMatrix, 0, 0, &pose, &options,
                                       &DAT_10219770[index]);
                    pose.SetTranslation(DAT_1021bdf8[index][POD_NODE_BONE]->worldMatrix.GetTranslation());
                    DAT_10daaa80.Start(POD_BURST_HEAD_EFFECT, effectPosition, -1, -1,
                                       &DAT_1021bdf8[index][POD_NODE_BONE]->worldMatrix, 0, 0, &pose, &options,
                                       &DAT_10219770[index]);
                    FUN_1016b230(POD_BURST_CAPTION);
                    FUN_10055ee0();
                    float loudness = (DAT_10db6ee8[index].matrix.GetTranslation() - DAT_118e6cb8).LengthSquared3();
                    if (loudness < POD_BURST_LOUD_SQUARED || index < POD_PARTNER_FIRST)
                    {
                        Vec4 soundPosition;
                        soundPosition.Set(DAT_10db6ee8[PLAYER_SLOT].matrix.translation.x,
                                          DAT_10db6ee8[PLAYER_SLOT].matrix.translation.y,
                                          DAT_10db6ee8[PLAYER_SLOT].matrix.translation.z, 0.0f);
                        FUN_1003ad20(POD_BURST_SOUND, &soundPosition, 1.0f);
                    }
                }
                if (state.podFrames > POD_BURST_GLOW_FROM && state.podFrames < POD_BURST_GLOW_TO)
                {
                    DAT_10db6ee8[index].root->M_10194730(POD_NODE_KIND_GLOW, POD_NODE_MASK_A);
                    DAT_10db6ee8[index].root->M_10194730(POD_NODE_KIND_GLOW, POD_NODE_MASK_B);
                }
                else
                {
                    DAT_10db6ee8[index].root->M_10194730(POD_NODE_KIND_PLAIN, POD_NODE_MASK_A);
                    DAT_10db6ee8[index].root->M_10194730(POD_NODE_KIND_PLAIN, POD_NODE_MASK_B);
                }
                if (state.podFrames == POD_BURST_END)
                {
                    state.podFrames = 0;
                    state.state = POD_STATE_IDLE;
                }
            }
            else if (state.state == POD_STATE_FADE)
            {
                if (state.podGrowth > 0)
                    state.podGrowth = state.podGrowth - 1;
                else
                    state.state = POD_STATE_GROW;
                if (state.podGrowth > 0)
                    DAT_10db6ee8[index].alpha = (float)state.podGrowth / POD_GROWTH_ALPHA_FRAMES;
                else
                    DAT_10db6ee8[index].alpha = 0.0f;
                state.podMotion = POD_MOTION_GROW;
            }
        }
        {
            EffectOptions podOptions;
            podOptions.flags |= 2;
            podOptions.owner = index;
            for (int pod = 6; pod < 12; pod++)
            {
                int timer = DAT_1020b430[index].words[pod];
                if (timer == 1)
                {
                    pose.SetIdentity();
                    pose.SetTranslation(DAT_1021bdf8[index][pod - 6]->worldMatrix.GetTranslation());
                    float zero = 0.0f;
                    effectPosition.Fill(zero);
                    DAT_10daaa80.Start(POD_SPROUT_EFFECT, effectPosition, -1, -1, &DAT_1021bdf8[index][pod - 6]->worldMatrix,
                                       0, 0, &pose, &podOptions, &DAT_10219770[index]);
                }
                else if (timer > POD_RIPE_TIMER - 1)
                {
                    pose.SetIdentity();
                    pose.SetTranslation(DAT_1021bdf8[index][pod - 6]->worldMatrix.GetTranslation());
                    float zero = 0.0f;
                    effectPosition.Fill(zero);
                    DAT_10daaa80.Start(POD_RIPE_EFFECT, effectPosition, -1, -1, 0, 0, 0, &pose, &podOptions,
                                       &DAT_10219770[index]);
                }
                if (DAT_1020b430[index].words[pod] != 0)
                    DAT_1020b430[index].words[pod] = DAT_1020b430[index].words[pod] + 1;
            }
        }
        if (DAT_1020b430[index].podGrowth == 0)
        {
            EffectOptions groundOptions;
            groundOptions.flags |= 2;
            groundOptions.owner = index;
            pose.SetIdentity();
            pose = RotationZ(DAT_10db6ee8[index].heading);
            pose.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
            float zero = 0.0f;
            effectPosition.Fill(zero);
            if (DAT_10c655e0 != 0)
                DAT_10daaa80.Start(POD_GROUND_EFFECT_ALT, effectPosition, -1, -1, 0, 0, 0, &pose, &groundOptions,
                                   &DAT_10219770[index]);
            else
                DAT_10daaa80.Start(POD_GROUND_EFFECT, effectPosition, -1, -1, 0, 0, 0, &pose, &groundOptions,
                                   &DAT_10219770[index]);
        }
        if (DAT_102550e0[index] == 0)
        {
            DAT_10db6ee8[index].PlayMotion(DAT_1020b430[index].podMotion, 0, POD_MOTION_BLEND, 1.0f);
            FUN_100241e0(index, walk.x, walk.y, walk.z);
        }
        float heading = DAT_10db6ee8[index].heading;
        float target = DAT_10db6ee8[index].targetHeading;
        while (heading > POD_PI)
            heading -= POD_TWO_PI;
        while (target > POD_PI)
            target -= POD_TWO_PI;
        while (heading < -POD_PI)
            heading += POD_TWO_PI;
        while (target < -POD_PI)
            target += POD_TWO_PI;
        heading -= target;
        while (heading > POD_PI)
            heading -= POD_TWO_PI;
        while (heading < -POD_PI)
            heading += POD_TWO_PI;
        if (heading != 0.0f && DAT_10db6ee8[index].m_238c != 0 && DAT_1020b430[index].state == POD_STATE_IDLE)
            DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading - heading / POD_TURN_EASE;
    }
    return 0;
}
