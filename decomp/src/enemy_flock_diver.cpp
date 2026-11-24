// The flock diver (0x10086380, per-kind behaviour 0x2c of 0x100f9670): five fliers in the enemy slots 0x3c..0x40 that
// circle the arena, rise out of the pit, swoop at the player with streaks and rings, fall back and follow each other in
// chains. The slots before them hold the flock's director: slot 0x3a's words say which attack pattern (flockPattern) and
// round (flockRound) the flock is in and slot 0x3b's state says how it forms up (formation) and carries the clock of the
// formation (flockClock). C++ rather than extern "C": the function keeps an exception frame for its Matrix4 locals.

#include <math.h>
#include <stdlib.h>

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
#include "last_position.h"
#include "map_vars.h"
#include "world.h"

#include "char_ground.h"
#include "doppler_loops.h"
#include "hop_spit_enemy.h"

#include "flock_diver.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40
Matrix4 RotationX(float angle);   // 0x101b1ad0
Matrix4 Scaling(float x, float y, float z);   // 0x101b13c0

// Slots: 0x3a holds the director's pattern and round, 0x3b its formation state and clock, 0x3c..0x40 are the fliers.
#define FLOCK_DIRECTOR_SLOT 0x3a
#define FLOCK_FORMATION_SLOT 0x3b
#define FLOCK_FIRST 0x3c
#define FLOCK_MIDDLE 0x3e
#define FLOCK_LAST 0x40
#define FLOCK_LATE_ROUND 2

// CharState::state of a flier.
#define FLOCK_STATE_START 0
#define FLOCK_STATE_CIRCLE 1
#define FLOCK_STATE_RISE_A 2
#define FLOCK_STATE_RISE_B 3
#define FLOCK_STATE_SWOOP_A 4
#define FLOCK_STATE_SWOOP_B 5
#define FLOCK_STATE_FALL 6
#define FLOCK_STATE_SWOOP_C 8
#define FLOCK_STATE_SWOOP_D 9
#define FLOCK_STATE_CHAIN_A 10
#define FLOCK_STATE_CHAIN_B 11
#define FLOCK_STATE_CHASE 12

// Formations (the state of slot 0x3b): 1, 2 and 4 hold the flock on a ring, 3 and 5..9 pull it into the player's path.
#define FORMATION_BAND_LAST 4
#define FORMATION_PULL 3
#define FORMATION_WIDE_FIRST 5
#define FORMATION_WIDE_END 10

// moveMode: how the move vector is applied at the end of the frame.
#define MOVE_NONE 0
#define MOVE_STEP 1            // the move vector is a step added to the position
#define MOVE_FOLLOW 2          // the move vector is the place to be, the step is limited

#define FLOCK_MOTION_NONE -1
#define FLOCK_MOTION_FLY 0
#define FLOCK_MOTION_GLIDE 3
#define FLOCK_MOTION_STRIKE 5
#define FLOCK_MOTION_LAND 0xb
#define FLOCK_MOTION_FALL 9
#define FLOCK_MOTION_DROP 10
#define FLOCK_MOTION_ASCEND 8
#define FLOCK_MOTION_END_FRAME 0xb3
#define FLOCK_BLEND_FRAMES 5

#define FLOCK_SCALE 0.1f
#define FLOCK_GROUND_PERIOD 30
#define SPEED_STEP_LIMIT 30.0f
#define FLOCK_PI 3.1415f
#define FLOCK_TWO_PI 6.283f
#define FLOCK_HEAD_NODE "Bip01 Head"
#define FLOCK_ALPHA 2
#define FLOCK_PLAYER_SPOT_SQUARED 6250000.0f

// Rising (states 2 and 3) and swooping (states 4, 5, 8, 9): `phase` is the step minus the flier's start delay.
#define RISE_START_DELAY 0xf
#define RISE_START_DELAY_MIDDLE 0x1e
#define SWOOP_DELAY_STEP 7
#define RISE_PHASE_FAST 0x1e
#define RISE_PHASE_SLOW 0x23
#define RISE_PHASE_TURN 0x28
#define RISE_PHASE_STRIKE 0x32
#define RISE_PHASE_LAND 0x41
#define RISE_PHASE_FLASH 0x26
#define RISE_PHASE_SLIDE 0x39
#define SWOOP_THROW_PHASE 0x23
#define SWOOP_WINDOW 10000
#define SWOOP_END 20000
#define SWOOP_FALL_END 0x4e20
#define SWOOP_PROBE_END 0x2710
#define RISE_SPEED_Y -2.0f
#define GLIDE_SPEED_Y -1.5f
#define RISE_LIFT 500.0f
#define RISE_FAST_DIVISOR 55.0f
#define RISE_SLOW_DIVISOR 35.0f
#define CEILING_MARGIN 150.0f
#define RISE_B_LIFT 100.0f
#define RISE_A_LIFT 75.0f
#define RISE_FLASH_LIFT 280.0f
#define RISE_FLASH_SPREAD 120.0f
#define RISE_LIFT_SPREAD 25.0f
#define SWOOP_LIFT_SPREAD 6.0f
#define STRIKE_FALL_SCALE 0.8f
#define LAND_SPEED 15.0f
#define LAND_SLIDE_SPEED -10.0f
#define LAND_SCALE 1.15f
#define LAND_FLOOR -450.0f
#define LAND_SPEED_DIVISOR 1.5f
#define STRIKE_SPEED 32.0f
#define STRIKE_SPEED_FAST 35.0f
#define STRIKE_DRAG 3.0f
#define STRIKE_RANGE_FLOOR -37.5f
#define STRIKE_RANGE 550.0f
#define STRIKE_BOUNCE -0.95f
#define STRIKE_BOUNCE_LIMIT 25.0f
#define STRIKE_MIN_FRAMES 100
#define RISE_GAIN 1.1f
#define STRIKE_PITCH -1.5708f      // 0xbfc90fb6 as a float
#define STREAK_PERIOD 3
#define FLASH_SOUND 0x2ad
#define HURL_SOUND 0x2ae
#define LAND_SOUND 0x2ab
#define STREAK_EFFECT 0x463
#define SWOOP_EFFECT 0x60c
#define SWOOP_TRAIL_EFFECT 0x657
#define HEAD_EFFECT 5
#define HEAD_EFFECT_PERIOD 10
#define SPLASH_EFFECT 0xab
#define SPLASH_SOUND 0x2ac
#define SPLASH_LIFT 20.0f
#define SPLASH_NEAR_SQUARED 6250000.0f
#define FIRST_ENEMY_SLOT 0x3a
#define SPLASH_ALWAYS_BELOW 0x3a

// Chains (states 10 and 11): each flier follows the slot two before it (the first one the player).
#define CHAIN_FRAMES 1000
#define CHAIN_END 2000
#define CHAIN_FIRST_STAGE 0x5a
#define CHAIN_FOLLOW_RANGE 400.0f
#define CHAIN_SHORT_RANGE 150.0f
#define CHAIN_FAR_RANGE 1300.0f
#define CHAIN_SPEED_LIMIT 30.0f
#define CHAIN_STEP_DOWN 75.0f
#define CHAIN_CEILING 170.0f
#define CHAIN_OFFSET_ANGLE 2.0944f
#define CHAIN_STAGE_ONE 0x3e9
#define CHAIN_STAGE_END 0x424
#define CHAIN_BURST_ANGLE 0.5236f
#define CHAIN_BURST_SCALE 0.9f
#define CHAIN_BURST_EFFECT_A 0x452
#define CHAIN_BURST_EFFECT_B 0x463
#define CHAIN_BURST_TURN 1.0472f
#define CHAIN_SPIRAL_FRAMES 0x441
#define CHAIN_SPIRAL_LOW -350.0f
#define CHAIN_SPIRAL_FLOOR -700.0f
#define CHAIN_SPIRAL_DRIFT 30.0f
#define CHAIN_SPIRAL_ANGLE 0.0005f
#define CHAIN_SPIRAL_ANGLE_SLOW 0.001f
#define CHAIN_ANGLE_BASE 0.15f
#define CHAIN_SPIRAL_START 2000
#define CHAIN_SPIRAL_END 3000
#define CHAIN_SPIRAL_PHASE_COUNT 0x7d1
#define CHAIN_SPIRAL_STAGE 0x7d0
#define CHAIN_SPIRAL_ONE_TURN 0xbb8
#define CHAIN_SPIN_END 4000
#define CHAIN_SPIN_START 0xbc7
#define CHAIN_SPIN_MID 0xbd6
#define CHAIN_SPIN_SCALE 3.0f
#define CHAIN_SPIN_TURN 15.0f
#define CHAIN_SPIN_FLOOR -690.0f
#define CHAIN_HALF_TURN 1.5708f
#define CHAIN_SPIN_RISE -30.0f
#define CHAIN_ORBIT_RADIUS 150.0f
#define CHAIN_REACH_RADIUS 400.0f
#define CHAIN_ORBIT_STEP 0.0005f
#define CHAIN_WIDTH 0.001f
#define CHAIN_SLIDE_HEIGHT 70.0f
#define CHAIN_SLIDE_LIMIT 10.0f
#define CHAIN_LEAVE_RANGE 1200.0f
#define CHAIN_CHASE_FRAMES 0x5a
#define CHAIN_CHASE_ROAR 0x5b
#define CHAIN_CHASE_DIVISOR 30.0f
#define CHAIN_CHASE_FALL -30.0f
#define CHAIN_CHASE_TURN_SOUND 0x2ab

// Circling the arena (state 1): the move vector is a point on a ring about the arena centre.
#define RING_ANGLE_STEP 0.1f
#define RING_ANGLE_STEP_SMALL 0.025f
#define RING_RADIUS_FREE 1000.0f
#define RING_RADIUS_TURN 450.0f
#define RING_RADIUS_NEAR 1550.0f
#define RING_RADIUS_FAR 1300.0f
#define RING_FLOOR -300.0f
#define RING_FLOOR_NEAR -500.0f
#define RING_FLOOR_FAR -600.0f
#define RING_FLIP_PERIOD 100
#define RING_LEADER_FLIP_PERIOD 500
#define RING_FLIP_BELOW 2
#define RING_FAR_ANGLE 1.3f
#define RING_NEAR_ANGLE 0.7f
#define RING_FAR_SIDE 0.15f
#define RING_NEAR_SIDE 0.25f
#define RING_SLOT_SPACING 200
#define RING_CLOCK_SCALE 1000.0f
#define RING_SPEED_FACTOR 0.9f
#define RING_SPEED_LIMIT 35.0f
#define RING_SPEED_LIMIT_NEAR 30.0f
#define RING_SPEED_SLOW 10.0f
#define RING_TURN_RANGE 2.0f
#define FORMATION_SPOT 200.0f
#define FORMATION_SPOT_MIDDLE 50.0f
#define FORMATION_SPOT_Y -150.0f
#define FORMATION_SPOT_BACK -500.0f
#define FORMATION_SPOT_STEP 150.0f
#define FORMATION_SPOT_DEPTH -150.0f
#define FORMATION_SPOT_SLOPE 0xc3160000

// 0x10086380 (Enemy_FlockDiver; C++ linkage: the original has an EH frame). One frame of flier `index`. The flier is
// scaled to 0.1, set down on its slot's ground every 30th frame and kept busy. Then its state (CharState::state) runs:
//  0: finds its head node and becomes state 6.
//  1: circles the arena on a ring (the formation of slot 0x3b says which ring and how far) and, while the formation
//     leaves it free, picks its next state from slot 0x3a's pattern: 2..4 start a rise or a swoop, 5..7 let only one
//     pair of fliers go, and in later rounds (flockRound 2 and above) the swoop is state 8 or 9.
//  2, 3: rise out of the pit towards the ceiling (150 below it), turning to the player; step 0x26 flashes a streak at
//     the head and starts the sound; state 3 starts the fliers one after the other.
//  4, 5, 8, 9: rise, glide and swoop at the player: from step 0x23 a straight dive of 32 or 35 units a frame with trails
//     (probing the ground ahead every frame), then climb back or fall; at the end of the motion it returns to state 1.
//  6: falls (z speed -30, 0.95 of the last speed once under the pit's ceiling) and returns to state 1 when it landed.
//  10, 11: follow the slot two before it (the first one the player), the player or the place it left, in stages of 1000
//     frames; state 11 later spirals down over 4000 steps.
//  12: slowly closes in on the player and goes back to state 1 when it leaves 1200 units around the arena.
// At the end the head's crossing of the pit's ceiling starts a splash, the flier is moved by the move vector and its
// motion is played. Returns 0.
int Enemy_FlockDiver(int index)
{
    int formation = DAT_1020b430[FLOCK_FORMATION_SLOT].state;
    int motion = FLOCK_MOTION_NONE;
    int motionLoop = 0;
    int motionBlend = FLOCK_BLEND_FRAMES;
    int holdFlag = 0;
    Vec4 playerPosition;
    Vec4 selfPosition;
    Vec4 centre;
    Vec4 move;
    playerPosition.Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
    selfPosition.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    centre.Copy(DAT_10e40204.GetTranslation());
    float speed = (selfPosition - centre).Length2D();
    int moveMode = MOVE_NONE;
    float polarAngle = (float)atan2(selfPosition.y, selfPosition.x);
    DAT_1021f6c0[index] = 0;
    if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
        DAT_10db6ee8[index].mode = 0;
    DAT_10daaae8[index].stats[STAT_BUSY] = 1;
    DAT_10db6ee8[index].uniformScale = FLOCK_SCALE;
    if (DAT_104c0380 % FLOCK_GROUND_PERIOD == index % FLOCK_GROUND_PERIOD)
    {
        DAT_10db6ee8[index].m_0012 = FLOCK_ALPHA;
        FUN_1001c090(index, 1);
    }
    CharState &state = DAT_1020b430[index];
    if (state.state == FLOCK_STATE_START)
    {
        state.state = FLOCK_STATE_FALL;
        state.flockFrames = 0;
        state.flockMotion = FLOCK_MOTION_NONE;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(FLOCK_HEAD_NODE, 0);
    }
    else if (state.state == FLOCK_STATE_CIRCLE)
    {
        if (DAT_1020b430[FLOCK_DIRECTOR_SLOT].flockRound < FLOCK_LATE_ROUND)
        {
            if (formation >= 1 && formation <= FORMATION_BAND_LAST && formation != FORMATION_PULL)
            {
                motion = FLOCK_MOTION_FLY;
                float clock = (float)DAT_1020b430[FLOCK_FORMATION_SLOT].flockClock;
                float angle = clock / RING_CLOCK_SCALE - polarAngle;
                while (angle > FLOCK_PI)
                    angle -= FLOCK_TWO_PI;
                while (angle < -FLOCK_PI)
                    angle += FLOCK_TWO_PI;
                if (floorf(angle) >= RING_FAR_ANGLE)
                {
                    float ringAngle;
                    if (angle > 0.0f)
                        ringAngle = atan2f(selfPosition.y, selfPosition.x) + RING_FAR_SIDE;
                    else
                        ringAngle = atan2f(selfPosition.y, selfPosition.x) - RING_FAR_SIDE;
                    move.Set(cosf(ringAngle) * RING_RADIUS_FREE, sinf(ringAngle) * RING_RADIUS_FREE, centre.z, 1.0f);
                    moveMode = MOVE_FOLLOW;
                }
                else
                {
                    switch (index)
                    {
                    case 0x3c:
                        move.x = -FORMATION_SPOT;
                        move.RotateZ(FLOCK_PI - DAT_10db6ee8[index].heading);
                        break;
                    case 0x3d:
                        move.x = FORMATION_SPOT;
                        move.RotateZ(FLOCK_PI - DAT_10db6ee8[index].heading);
                        break;
                    case 0x3e:
                        move.y = FORMATION_SPOT_Y;
                        move.x = -FORMATION_SPOT_MIDDLE;
                        break;
                    case 0x3f:
                        move.z = FORMATION_SPOT_DEPTH;
                        break;
                    case 0x40:
                        move.y = FORMATION_SPOT_BACK;
                        move.x = FORMATION_SPOT_MIDDLE;
                        move.RotateZ(-DAT_10db6ee8[index].heading);
                        break;
                    }
                    move.Copy(move + centre);
                    move.Copy(move - selfPosition);
                    speed = move.Length() * RING_SPEED_FACTOR;
                    move.Normalized3();
                    if (speed > RING_SPEED_LIMIT)
                        speed = RING_SPEED_LIMIT;
                    move.Copy(move * speed);
                    moveMode = MOVE_STEP;
                    if (move.Length2D() > RING_TURN_RANGE)
                        DAT_10db6ee8[index].targetHeading = atan2f(move.x, move.y) + FLOCK_PI;
                }
            }
            else if (formation == FORMATION_PULL || (formation > FORMATION_BAND_LAST && formation < FORMATION_WIDE_END))
            {
                holdFlag = 1;
                motion = FLOCK_MOTION_FLY;
                float clock = (float)DAT_1020b430[FLOCK_FORMATION_SLOT].flockClock;
                float angle = clock / RING_CLOCK_SCALE - polarAngle;
                while (angle > FLOCK_PI)
                    angle -= FLOCK_TWO_PI;
                while (angle < -FLOCK_PI)
                    angle += FLOCK_TWO_PI;
                if ((float)fabs(angle) >= RING_NEAR_ANGLE)
                {
                    float ringAngle;
                    if (angle > 0.0f)
                        ringAngle = atan2f(selfPosition.y, selfPosition.x) + RING_NEAR_SIDE;
                    else
                        ringAngle = atan2f(selfPosition.y, selfPosition.x) - RING_NEAR_SIDE;
                    move.Set(cosf(ringAngle) * RING_RADIUS_FAR, sinf(ringAngle) * RING_RADIUS_FAR, FORMATION_SPOT_Y, 1.0f);
                    moveMode = MOVE_FOLLOW;
                }
                else
                {
                    float spread =
                        (float)((index - FLOCK_MIDDLE) * RING_SLOT_SPACING + DAT_1020b430[FLOCK_FORMATION_SLOT].flockClock);
                    float slotAngle = spread / RING_CLOCK_SCALE;
                    move.Set(cosf(slotAngle) * RING_RADIUS_NEAR, sinf(slotAngle) * RING_RADIUS_NEAR, FORMATION_SPOT_Y, 1.0f);
                    move.Copy(move - selfPosition);
                    speed = move.Length() * RING_SPEED_FACTOR;
                    if (speed > RING_SPEED_LIMIT_NEAR)
                        speed = RING_SPEED_LIMIT_NEAR;
                    move.Normalized3();
                    move.Copy(move * speed);
                    moveMode = MOVE_STEP;
                    holdFlag = 0;
                    if (speed >= RING_SPEED_SLOW)
                        DAT_10db6ee8[index].targetHeading = atan2f(move.x, move.y) + FLOCK_PI;
                    else
                        DAT_10db6ee8[index].targetHeading =
                            atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + FLOCK_PI;
                }
            }
            else
            {
                float ringAngle = atan2f(selfPosition.y, selfPosition.x);
                if (state.flockSide == 0)
                    ringAngle -= RING_ANGLE_STEP;
                else
                    ringAngle += RING_ANGLE_STEP;
                if (FUN_1013f880() % RING_FLIP_PERIOD < RING_FLIP_BELOW)
                    state.flockSide = (state.flockSide == 0);
                move.Set(cosf(ringAngle) * RING_RADIUS_FREE, sinf(ringAngle) * RING_RADIUS_FREE, RING_FLOOR, 1.0f);
                moveMode = MOVE_FOLLOW;
            }
        }
        else if (DAT_10db6ee8[PLAYER_SLOT].mode == 2)
        {
            holdFlag = 1;
            if ((index - FLOCK_FORMATION_SLOT) % 2 == 0 ||
                (DAT_1020b430[FLOCK_FIRST].state != FLOCK_STATE_CHAIN_A &&
                 (DAT_1020b430[FLOCK_FIRST].state != 1 || DAT_1020b430[FLOCK_FIRST + 2].state != 1 ||
                  DAT_1020b430[FLOCK_LAST].state != 1)))
            {
                if ((index - FIRST_ENEMY_SLOT) % 2 == 0 ||
                    (DAT_1020b430[FLOCK_FIRST + 1].state != FLOCK_STATE_CHAIN_B &&
                     (DAT_1020b430[FLOCK_FIRST + 1].state != 1 || DAT_1020b430[FLOCK_MIDDLE + 1].state != 1)))
                {
                    state.state = FLOCK_STATE_CHASE;
                    state.flockFrames = 0;
                }
                else
                {
                    state.state = FLOCK_STATE_CHAIN_B;
                    state.flockFrames = 0;
                }
            }
            else
            {
                state.state = FLOCK_STATE_CHAIN_A;
                state.flockFrames = 0;
            }
        }
        else if (index != FLOCK_FIRST)
        {
            float ringAngle = atan2f(DAT_10db6ee8[index - 1].matrix.translation.y,
                                     DAT_10db6ee8[index - 1].matrix.translation.x);
            state.flockSide = DAT_1020b430[index - 1].flockSide;
            if (state.flockSide == 0)
                ringAngle += 1.256637f;
            else
                ringAngle -= 1.256637f;
            move.Set(cosf(ringAngle) * RING_RADIUS_TURN, sinf(ringAngle) * RING_RADIUS_TURN, RING_FLOOR, 1.0f);
            moveMode = MOVE_FOLLOW;
        }
        else
        {
            float ringAngle = atan2f(selfPosition.y, selfPosition.x);
            if (DAT_1020b430[FLOCK_FIRST].flockSide == 0)
                ringAngle -= RING_ANGLE_STEP_SMALL;
            else
                ringAngle += RING_ANGLE_STEP_SMALL;
            if (FUN_1013f880() % RING_LEADER_FLIP_PERIOD < RING_FLIP_BELOW)
                DAT_1020b430[FLOCK_FIRST].flockSide = (DAT_1020b430[FLOCK_FIRST].flockSide == 0);
            move.Set(cosf(ringAngle) * RING_RADIUS_TURN, sinf(ringAngle) * RING_RADIUS_TURN, RING_FLOOR, 1.0f);
            moveMode = MOVE_FOLLOW;
        }
    }
    else if (state.state == FLOCK_STATE_RISE_A || state.state == FLOCK_STATE_RISE_B)
    {
        int lead = 0;
        if (state.state == FLOCK_STATE_RISE_B)
        {
            switch (index)
            {
            case 0x3d:
            case 0x3f:
                lead = RISE_START_DELAY;
                break;
            case 0x3e:
                lead = RISE_START_DELAY_MIDDLE;
                break;
            }
        }
        int step = ++state.flockFrames;
        moveMode = MOVE_STEP;
        int phase = step - lead;
        if (phase >= 0)
        {
            if (phase < RISE_PHASE_FAST)
            {
                motion = FLOCK_MOTION_FLY;
                move.y = RISE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = (DAT_104bfe24 + RISE_LIFT) / RISE_FAST_DIVISOR;
                DAT_10db6ee8[index].targetHeading =
                    atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + FLOCK_PI;
                float ceiling = DAT_104bfe24 - CEILING_MARGIN;
                if (selfPosition.z + move.z > ceiling)
                    move.z = ceiling - selfPosition.z;
            }
            else if (phase < RISE_PHASE_SLOW)
            {
                motion = FLOCK_MOTION_GLIDE;
                move.y = RISE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = (DAT_104bfe24 + RISE_LIFT) / RISE_SLOW_DIVISOR;
                DAT_10db6ee8[index].targetHeading =
                    atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + FLOCK_PI;
                float ceiling = DAT_104bfe24 - CEILING_MARGIN;
                if (selfPosition.z + move.z > ceiling)
                    move.z = ceiling - selfPosition.z;
            }
            else if (phase == RISE_PHASE_SLOW)
            {
                motion = FLOCK_MOTION_GLIDE;
                move.y = GLIDE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = RISE_B_LIFT - (float)abs(FLOCK_MIDDLE - index) * RISE_LIFT_SPREAD;
            }
            else if (phase < RISE_PHASE_TURN)
            {
                motion = FLOCK_MOTION_GLIDE;
                move.y = GLIDE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = DAT_10802bf8[index].position.z * STRIKE_FALL_SCALE;
            }
            else if (phase == RISE_PHASE_TURN)
            {
                motion = FLOCK_MOTION_STRIKE;
                motionBlend = 0;
                motionLoop = 1;
                move.y = GLIDE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = DAT_10802bf8[index].position.z * STRIKE_FALL_SCALE;
            }
            else if (phase < RISE_PHASE_STRIKE)
            {
                motion = FLOCK_MOTION_STRIKE;
                move.y = GLIDE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = DAT_10802bf8[index].position.z * STRIKE_FALL_SCALE;
            }
            else if (phase == RISE_PHASE_STRIKE)
            {
                motion = FLOCK_MOTION_LAND;
                motionBlend = 0;
                motionLoop = 1;
                move.y = LAND_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading);
            }
            else if (phase < RISE_PHASE_LAND)
            {
                move.y = (float)(RISE_PHASE_LAND - step);
                motion = FLOCK_MOTION_LAND;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                if (state.flockFrames - lead > RISE_PHASE_SLIDE)
                    move.z = DAT_10802bf8[index].position.z * LAND_SCALE;
                else if (state.flockFrames - lead == RISE_PHASE_SLIDE)
                    move.z = LAND_SLIDE_SPEED;
            }
            else if (selfPosition.z > LAND_FLOOR)
            {
                motion = FLOCK_MOTION_LAND;
                move.z = DAT_10802bf8[index].position.z * LAND_SCALE;
            }
            else if (DAT_10db6ee8[index].motionFrame == FLOCK_MOTION_END_FRAME)
            {
                state.state = FLOCK_STATE_CIRCLE;
                state.flockFrames = 0;
            }
            else
            {
                motion = FLOCK_MOTION_DROP;
                move.z = DAT_10802bf8[index].position.z / LAND_SPEED_DIVISOR;
            }
        }
        if (state.flockFrames - lead == RISE_PHASE_FLASH && selfPosition.z > DAT_104bfe24 - CEILING_MARGIN)
        {
            Vec4 effectPosition;
            Matrix4 pose;
            EffectOptions options;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            effectPosition.Copy(selfPosition);
            effectPosition.z = RISE_FLASH_LIFT - (float)abs(FLOCK_MIDDLE - index) * RISE_FLASH_SPREAD;
            float pitch;
            if ((effectPosition - playerPosition).Length() > 1.0f)
                pitch = acosf((effectPosition - playerPosition).Length2D() / (effectPosition - playerPosition).Length());
            else
                pitch = 0.0f;
            if (playerPosition.z < effectPosition.z)
                pitch = pitch * -1.0f;
            float zero = 0.0f;
            effectPosition.Fill(zero);
            pose = RotationX(pitch) * RotationZ(DAT_10db6ee8[index].heading);
            FUN_1001d8b0(index, STREAK_EFFECT, effectPosition, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0, &pose,
                         &options);
            FUN_10065b10(index, FLASH_SOUND, 0, 0);
        }
    }
    else if (state.state == FLOCK_STATE_SWOOP_A || state.state == FLOCK_STATE_SWOOP_B)
    {
        int lead = 0;
        if (state.state == FLOCK_STATE_SWOOP_A)
        {
            switch (index)
            {
            case 0x3d:
            case 0x3f:
                lead = RISE_START_DELAY;
                break;
            case 0x3e:
                lead = RISE_START_DELAY_MIDDLE;
                break;
            }
        }
        else
        {
            switch (index)
            {
            case 0x3c:
                lead = SWOOP_DELAY_STEP;
                break;
            case 0x3d:
                lead = SWOOP_DELAY_STEP * 2;
                break;
            case 0x3e:
                lead = SWOOP_DELAY_STEP * 3;
                break;
            case 0x3f:
                lead = SWOOP_DELAY_STEP * 4;
                break;
            case 0x40:
                lead = SWOOP_DELAY_STEP * 5;
                break;
            }
        }
        int step = ++state.flockFrames;
        int phase = step - lead;
        moveMode = MOVE_STEP;
        if (phase >= 0)
        {
            if (phase < RISE_PHASE_SLOW)
            {
                if (phase < RISE_PHASE_FAST)
                {
                    motion = FLOCK_MOTION_FLY;
                    move.y = RISE_SPEED_Y;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    move.z = (DAT_104bfe24 + RISE_LIFT) / RISE_FAST_DIVISOR;
                }
                else
                {
                    motion = FLOCK_MOTION_GLIDE;
                    move.y = RISE_SPEED_Y;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    move.z = (DAT_104bfe24 + RISE_LIFT) / RISE_SLOW_DIVISOR;
                }
                DAT_10db6ee8[index].targetHeading =
                    atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + FLOCK_PI;
                float ceiling = DAT_104bfe24 - CEILING_MARGIN;
                if (selfPosition.z + move.z > ceiling)
                    move.z = ceiling - selfPosition.z;
            }
            else if (phase == SWOOP_THROW_PHASE)
            {
                motion = FLOCK_MOTION_GLIDE;
                move.y = GLIDE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = RISE_A_LIFT - (float)abs(FLOCK_MIDDLE - index) * SWOOP_LIFT_SPREAD;
                DAT_10802bf8[index].step.Copy(playerPosition - selfPosition);
                DAT_10802bf8[index].step.z = 0.0f;
                DAT_10802bf8[index].step.Normalized3();
                DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].step * STRIKE_SPEED);
                DAT_10802bf8[index].step.z = move.z * 0.7f;
                FUN_10065b10(index, HURL_SOUND, 0, 0);
            }
            else if (phase < SWOOP_WINDOW)
            {
                DAT_10802bf8[index].step.z = DAT_10802bf8[index].step.z - STRIKE_DRAG;
                motion = FLOCK_MOTION_STRIKE;
                if (FUN_100242c0(index, 0, DAT_10802bf8[index].step.Length2D(), 1, 1, 0) == 0)
                {
                    if (selfPosition.z < STRIKE_RANGE_FLOOR && selfPosition.Length2D() < STRIKE_RANGE)
                    {
                        float bounce = DAT_10802bf8[index].step.z * STRIKE_BOUNCE;
                        DAT_10802bf8[index].step.z = bounce;
                        if (bounce > STRIKE_BOUNCE_LIMIT)
                            DAT_10802bf8[index].step.z = STRIKE_BOUNCE_LIMIT;
                        move.Copy(DAT_10802bf8[index].step);
                        if (selfPosition.z + move.z < STRIKE_RANGE_FLOOR)
                            move.z = STRIKE_RANGE_FLOOR - selfPosition.z;
                    }
                    else if (selfPosition.z < DAT_104bfe24 && state.flockFrames - lead > STRIKE_MIN_FRAMES)
                        state.flockFrames = lead + SWOOP_WINDOW;
                    else
                        move.Copy(DAT_10802bf8[index].step);
                }
                else
                    state.flockFrames = lead + SWOOP_WINDOW;
                Matrix4 pose;
                EffectOptions options;
                Vec4 effectPosition;
                options.flags |= 2;
                pose = Scaling(0.35f, 0.35f, 0.8f) * RotationX(STRIKE_PITCH) * RotationZ(DAT_10db6ee8[index].heading);
                pose.SetTranslation(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() +
                                    DAT_10802bf8[index].step * 5.0f);
                if (state.flockFrames % STREAK_PERIOD == 0)
                    FUN_1001d8b0(index, SWOOP_EFFECT, effectPosition, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0,
                                 &pose, &options);
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0000 = 0;
                options.m_000e = 5;
                FUN_1001d8b0(index, SWOOP_TRAIL_EFFECT, effectPosition, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0,
                             0, &options);
            }
            else if (lead + state.flockFrames < SWOOP_END)
            {
                float rise = DAT_10802bf8[index].step.z * RISE_GAIN;
                DAT_10802bf8[index].step.z = rise;
                if (rise > 0.0f)
                    DAT_10802bf8[index].step.z = LAND_SLIDE_SPEED;
                move.z = DAT_10802bf8[index].step.z;
                motion = FLOCK_MOTION_ASCEND;
                if (selfPosition.z < DAT_104bfe24)
                    state.flockFrames = lead + SWOOP_END;
            }
            else
            {
                float fall = DAT_10802bf8[index].step.z / LAND_SCALE;
                motion = FLOCK_MOTION_DROP;
                DAT_10802bf8[index].step.z = fall;
                move.z = fall;
                if (DAT_10db6ee8[index].motionFrame == FLOCK_MOTION_END_FRAME)
                {
                    state.state = FLOCK_STATE_CIRCLE;
                    state.flockFrames = 0;
                }
            }
        }
    }
    else if (state.state == FLOCK_STATE_SWOOP_C || state.state == FLOCK_STATE_SWOOP_D)
    {
        int lead = 0;
        if (state.state == FLOCK_STATE_SWOOP_C)
        {
            switch (index)
            {
            case 0x3d:
            case 0x3f:
                lead = RISE_START_DELAY;
                break;
            case 0x3e:
                lead = RISE_START_DELAY_MIDDLE;
                break;
            }
        }
        else
        {
            switch (index)
            {
            case 0x3c:
                lead = SWOOP_DELAY_STEP;
                break;
            case 0x3d:
                lead = SWOOP_DELAY_STEP * 2;
                break;
            case 0x3e:
                lead = SWOOP_DELAY_STEP * 3;
                break;
            case 0x3f:
                lead = SWOOP_DELAY_STEP * 4;
                break;
            case 0x40:
                lead = SWOOP_DELAY_STEP * 5;
                break;
            }
        }
        int step = ++state.flockFrames;
        int phase = step - lead;
        moveMode = MOVE_STEP;
        if (phase >= 0)
        {
            if (phase < RISE_PHASE_SLOW)
            {
                if (phase < RISE_PHASE_FAST)
                {
                    motion = FLOCK_MOTION_FLY;
                    move.y = RISE_SPEED_Y;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    move.z = (DAT_104bfe24 + RISE_LIFT) / RISE_FAST_DIVISOR;
                }
                else
                {
                    motion = FLOCK_MOTION_GLIDE;
                    move.y = RISE_SPEED_Y;
                    move.RotateZ(-DAT_10db6ee8[index].heading);
                    move.z = (DAT_104bfe24 + RISE_LIFT) / RISE_SLOW_DIVISOR;
                }
                DAT_10db6ee8[index].targetHeading =
                    atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + FLOCK_PI;
                float ceiling = DAT_104bfe24 - CEILING_MARGIN;
                if (selfPosition.z + move.z > ceiling)
                    move.z = ceiling - selfPosition.z;
            }
            else if (phase == SWOOP_THROW_PHASE)
            {
                motion = FLOCK_MOTION_GLIDE;
                move.y = GLIDE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = RISE_B_LIFT - (float)abs(FLOCK_MIDDLE - index) * RISE_LIFT_SPREAD;
            }
            else if (phase < RISE_PHASE_TURN)
            {
                motion = FLOCK_MOTION_GLIDE;
                move.y = GLIDE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = DAT_10802bf8[index].position.z * STRIKE_FALL_SCALE;
            }
            else if (phase == RISE_PHASE_TURN)
            {
                motion = FLOCK_MOTION_STRIKE;
                motionBlend = 0;
                motionLoop = 1;
                move.y = GLIDE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = DAT_10802bf8[index].position.z * STRIKE_FALL_SCALE;
            }
            else if (phase < RISE_PHASE_STRIKE)
            {
                motion = FLOCK_MOTION_STRIKE;
                move.y = GLIDE_SPEED_Y;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = DAT_10802bf8[index].position.z * STRIKE_FALL_SCALE;
            }
            else if (phase == RISE_PHASE_STRIKE)
            {
                DAT_10802bf8[index].step.Copy(playerPosition - selfPosition);
                DAT_10802bf8[index].step.Normalized3();
                DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].step * STRIKE_SPEED_FAST);
                FUN_10065b10(index, HURL_SOUND, 0, 0);
            }
            else if (phase < SWOOP_PROBE_END)
            {
                motion = FLOCK_MOTION_STRIKE;
                if (FUN_100242c0(index, 0, DAT_10802bf8[index].step.Length2D(), 1, 1, 0) != 0 ||
                    (selfPosition.z < DAT_104bfe24 && state.flockFrames - lead > STRIKE_MIN_FRAMES))
                {
                    float last = DAT_10802bf8[index].step.z;
                    state.flockFrames = lead + SWOOP_WINDOW;
                    if (last > -1.0f)
                        DAT_10802bf8[index].step.z = -1.0f;
                }
                else
                    move.Copy(DAT_10802bf8[index].step);
                Matrix4 pose;
                EffectOptions options;
                Vec4 effectPosition;
                options.flags |= 2;
                pose = Scaling(0.35f, 0.35f, 0.8f) * RotationX(STRIKE_PITCH) * RotationZ(DAT_10db6ee8[index].heading);
                pose.SetTranslation(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() +
                                    DAT_10802bf8[index].step * 5.0f);
                if (state.flockFrames % STREAK_PERIOD == 0)
                    FUN_1001d8b0(index, SWOOP_EFFECT, effectPosition, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0,
                                 &pose, &options);
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0000 = 0;
                options.m_000e = 5;
                FUN_1001d8b0(index, SWOOP_TRAIL_EFFECT, effectPosition, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0,
                             0, &options);
            }
            else if (lead + state.flockFrames < SWOOP_END)
            {
                float rise = DAT_10802bf8[index].step.z * RISE_GAIN;
                DAT_10802bf8[index].step.z = rise;
                if (rise > 0.0f)
                    DAT_10802bf8[index].step.z = LAND_SLIDE_SPEED;
                move.z = DAT_10802bf8[index].step.z;
                motion = FLOCK_MOTION_ASCEND;
                if (selfPosition.z < DAT_104bfe24)
                    state.flockFrames = lead + SWOOP_END;
            }
            else
            {
                float fall = DAT_10802bf8[index].step.z / LAND_SCALE;
                motion = FLOCK_MOTION_DROP;
                DAT_10802bf8[index].step.z = fall;
                move.z = fall;
                if (DAT_10db6ee8[index].motionFrame == FLOCK_MOTION_END_FRAME)
                {
                    state.state = FLOCK_STATE_CIRCLE;
                    state.flockFrames = 0;
                }
            }
        }
    }
    else if (state.state == FLOCK_STATE_FALL)
    {
        motion = FLOCK_MOTION_FALL;
        move.z = CHAIN_CHASE_FALL;
        moveMode = MOVE_STEP;
        if (selfPosition.z < DAT_104bfe24)
        {
            move.z = DAT_10802bf8[index].position.z * 0.95f;
            motion = FLOCK_MOTION_DROP;
            if (DAT_10db6ee8[index].motionFrame == FLOCK_MOTION_END_FRAME)
                state.state = FLOCK_STATE_CIRCLE;
        }
    }
    else if (state.state == FLOCK_STATE_CHAIN_A)
    {
        int step = ++state.flockFrames;
        if (step < CHAIN_FRAMES)
        {
            float angle;
            if (index == FLOCK_FIRST)
                angle = atan2f(selfPosition.y - playerPosition.y, selfPosition.x - playerPosition.x);
            else
                angle = atan2f(DAT_10db6ee8[index - 2].matrix.translation.y - playerPosition.y,
                               DAT_10db6ee8[index - 2].matrix.translation.x - playerPosition.x) +
                        CHAIN_OFFSET_ANGLE;
            move.Set(cosf(angle) * CHAIN_REACH_RADIUS, sinf(angle) * CHAIN_REACH_RADIUS, 0.0f, 0.0f);
            move.Copy(move + playerPosition);
            move.z = move.z - CHAIN_STEP_DOWN;
            float ceiling = DAT_104bfe24 - CHAIN_CEILING;
            if (move.z > ceiling)
                move.z = ceiling;
            moveMode = MOVE_STEP;
            DAT_10db6ee8[index].targetHeading =
                atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + FLOCK_PI;
            if (move.Length2D() > CHAIN_FAR_RANGE)
            {
                float height = move.z;
                move.z = 0.0f;
                move.Normalized3();
                move.Copy(move * CHAIN_FAR_RANGE);
                move.z = height;
            }
            move.Copy(move - selfPosition);
            if (move.Length() > CHAIN_SPEED_LIMIT)
            {
                move.Normalized3();
                move.Copy(move * CHAIN_SPEED_LIMIT);
            }
            if (state.flockFrames == CHAIN_FIRST_STAGE)
                state.flockFrames = CHAIN_FRAMES;
        }
        else if (step < CHAIN_END)
        {
            if (step == CHAIN_STAGE_ONE)
            {
                Matrix4 pose;
                EffectOptions options;
                Vec4 effectPosition;
                pose = RotationZ(DAT_10db6ee8[index].heading);
                pose.SetTranslation(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.list = 1;
                options.m_0008 = 0.9f;
                FUN_1001d8b0(index, STREAK_EFFECT, effectPosition, -1, -1, 0, 0, 0, &pose, &options);
                pose = RotationZ(DAT_10db6ee8[index].heading + CHAIN_BURST_ANGLE);
                pose.SetTranslation(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
                FUN_1001d8b0(index, CHAIN_BURST_EFFECT_A, effectPosition, -1, -1, 0, 0, 0, &pose, &options);
                pose = RotationX(CHAIN_BURST_TURN) * RotationZ(DAT_10db6ee8[index].heading);
                pose.SetTranslation(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
                FUN_1001d8b0(index, CHAIN_BURST_EFFECT_A, effectPosition, -1, -1, 0, 0, 0, &pose, &options);
                FUN_10065b10(index, FLASH_SOUND, 0, 0);
            }
            if (state.flockFrames == CHAIN_STAGE_END)
            {
                state.state = FLOCK_STATE_CIRCLE;
                state.flockFrames = 0;
            }
        }
    }
    else if (state.state == FLOCK_STATE_CHAIN_B)
    {
        int step = ++state.flockFrames;
        if (step < CHAIN_FRAMES)
        {
            float angle;
            if (index == 0x3d)
            {
                angle = atan2f(selfPosition.y - playerPosition.y, selfPosition.x - playerPosition.x);
                if (DAT_1020b430[0x3d].flockSide == 0)
                    angle = angle - CHAIN_ANGLE_BASE - (float)DAT_1020b430[0x3d].flockFrames * CHAIN_WIDTH;
                else
                    angle = (float)DAT_1020b430[0x3d].flockFrames * CHAIN_WIDTH + angle + CHAIN_ANGLE_BASE;
            }
            else
            {
                angle = atan2f(DAT_10db6ee8[index - 2].matrix.translation.y - playerPosition.y,
                               DAT_10db6ee8[index - 2].matrix.translation.x - playerPosition.x);
                state.flockSide = DAT_1020b430[index - 1].flockSide;
                if (state.flockSide == 0)
                    angle = angle + FLOCK_PI;
                else
                    angle = angle - FLOCK_PI;
            }
            move.Set(cosf(angle) * CHAIN_ORBIT_RADIUS, sinf(angle) * CHAIN_ORBIT_RADIUS, 0.0f, 0.0f);
            move.Copy(move + playerPosition);
            move.z = move.z - 50.0f;
            moveMode = MOVE_FOLLOW;
            float ceiling = DAT_104bfe24 - CEILING_MARGIN;
            if (move.z > ceiling)
                move.z = ceiling;
            if (move.Length2D() > CHAIN_FAR_RANGE)
            {
                float height = move.z;
                move.z = 0.0f;
                move.Normalized3();
                move.Copy(move * CHAIN_FAR_RANGE);
                move.z = height;
            }
            if (state.flockFrames == CHAIN_FIRST_STAGE)
                state.flockFrames = CHAIN_FRAMES;
            Vec4 aim;
            aim.Copy(move - selfPosition);
            DAT_10db6ee8[index].heading = atan2f(aim.x, aim.y) + FLOCK_PI;
            DAT_10db6ee8[index].targetHeading = atan2f(aim.x, aim.y) + FLOCK_PI;
        }
        else if (step < CHAIN_END)
        {
            float angle = atan2f(selfPosition.y - playerPosition.y, selfPosition.x - playerPosition.x);
            if (state.flockSide == 0)
                angle = angle - CHAIN_ANGLE_BASE - (float)(state.flockFrames - CHAIN_FRAMES) * CHAIN_SPIRAL_ANGLE;
            else
                angle = (float)(state.flockFrames - CHAIN_FRAMES) * CHAIN_SPIRAL_ANGLE + angle + CHAIN_ANGLE_BASE;
            move.Set(cosf(angle) * CHAIN_ORBIT_RADIUS, sinf(angle) * CHAIN_ORBIT_RADIUS, 0.0f, 0.0f);
            move.Copy(move + playerPosition);
            moveMode = MOVE_FOLLOW;
            if (index == 0x3d)
            {
                move.z = selfPosition.z - CHAIN_SPIRAL_DRIFT;
                if (move.z < CHAIN_SPIRAL_FLOOR)
                    move.z = CHAIN_SPIRAL_FLOOR;
            }
            else
            {
                move.z = selfPosition.z + CHAIN_SPIRAL_DRIFT;
                float ceiling = DAT_104bfe24 - CEILING_MARGIN;
                if (move.z > ceiling)
                    move.z = ceiling;
            }
            if (state.flockFrames == CHAIN_SPIRAL_FRAMES)
            {
                if (move.z >= CHAIN_SPIRAL_LOW)
                    state.flockFrames = CHAIN_SPIRAL_END;
                else
                    state.flockFrames = CHAIN_SPIRAL_START;
                DAT_10802bf8[index].step.Copy(playerPosition);
            }
        }
        else if (step < CHAIN_SPIRAL_END)
        {
            float angle = atan2f(selfPosition.y - DAT_10802bf8[index].position.y,
                                 selfPosition.x - DAT_10802bf8[index].position.x);
            if (state.flockSide == 0)
                angle = angle - CHAIN_ANGLE_BASE - (float)(state.flockFrames - 0x776) * CHAIN_SPIRAL_ANGLE;
            else
                angle = (float)(state.flockFrames - 0x776) * CHAIN_SPIRAL_ANGLE + angle + CHAIN_ANGLE_BASE;
            if (state.flockFrames == CHAIN_SPIRAL_PHASE_COUNT)
                FUN_10065b10(index, LAND_SOUND, 0, 0);
            move.Set(cosf(angle) * CHAIN_ORBIT_RADIUS, sinf(angle) * CHAIN_ORBIT_RADIUS, 0.0f, 0.0f);
            move.Copy(move + playerPosition);
            move.z = selfPosition.z + SPLASH_LIFT;
            moveMode = MOVE_FOLLOW;
            if (index == 0x3f)
                floorf(selfPosition.z - DAT_10e44b74);
            if (DAT_104bfe24 - CEILING_MARGIN < move.z)
            {
                state.state = FLOCK_STATE_CIRCLE;
                state.flockFrames = 0;
            }
            if (state.flockFrames % HEAD_EFFECT_PERIOD == 0)
            {
                Vec4 effectPosition;
                EffectOptions options;
                options.list = 1;
                options.flags |= 2;
                FUN_1001d8b0(index, HEAD_EFFECT, effectPosition, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0, 0,
                             &options);
            }
            DAT_1021f6c0[index] = 1;
        }
        else if (step < CHAIN_SPIN_END)
        {
            move.Copy(selfPosition);
            int spin = state.flockFrames;
            moveMode = MOVE_FOLLOW;
            if (spin < CHAIN_SPIN_MID)
            {
                if (spin < CHAIN_SPIN_START)
                    move.z = (float)(CHAIN_SPIN_START - spin) * CHAIN_SPIN_SCALE + move.z;
                else
                    move.z = move.z - (float)(spin - CHAIN_SPIN_START) * 2.0f;
                DAT_10db6ee8[index].root->matrix =
                    RotationX((float)(CHAIN_SPIN_START - spin) * CHAIN_HALF_TURN / CHAIN_SPIN_TURN);
            }
            else
            {
                move.z = move.z - CHAIN_SPIRAL_DRIFT;
                if (selfPosition.z < CHAIN_SPIN_FLOOR)
                {
                    state.state = FLOCK_STATE_CIRCLE;
                    state.flockFrames = 0;
                    DAT_10db6ee8[index].root->matrix.SetIdentity();
                }
                if (state.flockFrames % HEAD_EFFECT_PERIOD == 0)
                {
                    Vec4 effectPosition;
                    EffectOptions options;
                    options.list = 1;
                    options.flags |= 2;
                    FUN_1001d8b0(index, HEAD_EFFECT, effectPosition, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0,
                                 0, &options);
                }
                DAT_1021f6c0[index] = 1;
            }
        }
    }
    else if (state.state == FLOCK_STATE_CHASE)
    {
        int step = state.flockFrames;
        if (step == 0)
        {
            DAT_10db6ee8[index].targetHeading =
                atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + FLOCK_PI;
            state.flockFrames = step + 1;
        }
        else if (step < CHAIN_CHASE_FRAMES)
        {
            float zero = 0.0f;
            move.Fill(zero);
            move.z = (playerPosition.z - selfPosition.z) / CHAIN_CHASE_DIVISOR;
            float ceiling = DAT_104bfe24 - CHAIN_CEILING;
            if (selfPosition.z + move.z > ceiling)
                move.z = ceiling - selfPosition.z;
            moveMode = MOVE_STEP;
            state.flockFrames = state.flockFrames + 1;
        }
        else
        {
            if (step == CHAIN_CHASE_ROAR)
                FUN_10065b10(index, CHAIN_CHASE_TURN_SOUND, 0, 0);
            move.y = CHAIN_CHASE_FALL;
            move.RotateZ(-DAT_10db6ee8[index].heading);
            move.z = (playerPosition.z - selfPosition.z) - CHAIN_SLIDE_HEIGHT;
            if (move.z > CHAIN_SLIDE_LIMIT)
                move.z = CHAIN_SLIDE_LIMIT;
            else if (move.z < -CHAIN_SLIDE_LIMIT)
                move.z = -CHAIN_SLIDE_LIMIT;
            float ceiling = DAT_104bfe24 - CHAIN_CEILING;
            if (selfPosition.z + move.z > ceiling)
                move.z = ceiling - selfPosition.z;
            moveMode = MOVE_STEP;
            DAT_1021f6c0[index] = 1;
            if (state.flockFrames % HEAD_EFFECT_PERIOD == 0)
            {
                Vec4 effectPosition;
                EffectOptions options;
                options.list = 1;
                options.flags |= 2;
                FUN_1001d8b0(index, HEAD_EFFECT, effectPosition, -1, -1, &DAT_1021bdf8[index][0]->worldMatrix, 0, 0, 0,
                             &options);
            }
            if (selfPosition.Length2D() > CHAIN_LEAVE_RANGE)
            {
                float outward = selfPosition.Length2D();
                if (outward < (selfPosition + move).Length2D())
                    state.state = FLOCK_STATE_CIRCLE;
            }
            state.flockFrames = state.flockFrames + 1;
        }
    }
    if (state.state == FLOCK_STATE_CIRCLE && holdFlag == 0)
    {
        int pattern = DAT_1020b430[FLOCK_DIRECTOR_SLOT].flockPattern;
        switch (pattern)
        {
        case 2:
            state.state = FLOCK_STATE_RISE_B;
            state.flockFrames = -1;
            break;
        case 3:
            state.state = FLOCK_STATE_SWOOP_A;
            if (DAT_1020b430[FLOCK_DIRECTOR_SLOT].flockRound > 1)
                state.state = FLOCK_STATE_SWOOP_C;
            state.flockFrames = -1;
            break;
        case 4:
            state.state = FLOCK_STATE_SWOOP_B;
            if (DAT_1020b430[FLOCK_DIRECTOR_SLOT].flockRound > 1)
                state.state = FLOCK_STATE_SWOOP_D;
            state.flockFrames = -1;
            break;
        case 5:
        case 6:
        case 7:
            if (abs(index - FLOCK_MIDDLE) != pattern - 5)
                break;
        case 1:
            state.state = FLOCK_STATE_RISE_A;
            state.flockFrames = -1;
            break;
        }
    }
    float headHeight = DAT_1021bdf8[index][0]->worldMatrix.translation.z;
    if ((headHeight > DAT_104bfe24 && DAT_10255160[index][0] <= DAT_104bfe24) ||
        (headHeight <= DAT_104bfe24 && DAT_10255160[index][0] > DAT_104bfe24))
    {
        Vec4 effectPosition;
        Matrix4 pose;
        pose.SetIdentity();
        RotationZ(DAT_10db6ee8[index].heading);
        pose.SetTranslation(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
        pose.m[14] = DAT_104bfe24 + SPLASH_LIFT;
        DAT_10daaa80.Start(SPLASH_EFFECT, effectPosition, -1, -1, 0, 0, 0, &pose, 0, &DAT_10219770[index]);
        float loudness = (DAT_10db6ee8[index].matrix.GetTranslation() - DAT_118e6cb8).LengthSquared3();
        if (loudness < SPLASH_NEAR_SQUARED || index < SPLASH_ALWAYS_BELOW)
        {
            Vec4 soundPosition;
            soundPosition.Set(DAT_10db6ee8[index].matrix.translation.x, DAT_10db6ee8[index].matrix.translation.y,
                              DAT_10db6ee8[index].matrix.translation.z, 0.0f);
            FUN_1003ad20(SPLASH_SOUND, &soundPosition, 1.0f);
        }
    }
    DAT_10255160[index][0] = DAT_1021bdf8[index][0]->worldMatrix.translation.z;
    if (moveMode == MOVE_STEP)
    {
        DAT_10db6ee8[index].SetPosition(selfPosition.x + move.x, selfPosition.y + move.y, selfPosition.z + move.z);
        DAT_10802bf8[index].position.Copy(move);
    }
    else if (moveMode == MOVE_FOLLOW)
    {
        Vec4 delta;
        delta.Copy(move - selfPosition);
        DAT_10802bf8[index].position.Copy(delta);
        float length = delta.Length();
        if (length > SPEED_STEP_LIMIT)
        {
            delta.Normalized3();
            delta.Copy(delta * SPEED_STEP_LIMIT);
        }
        move.Copy(selfPosition + delta);
        DAT_10db6ee8[index].SetPosition(move.x, move.y, move.z);
        if (length > 1.0f)
        {
            float aim = (float)atan2(delta.x, delta.y);
            DAT_10db6ee8[index].targetHeading = aim + FLOCK_PI;
        }
    }
    if (motion > -1 && state.flockMotion != motion)
        DAT_10db6ee8[index].PlayMotion(motion, motionLoop, motionBlend, 1.0f);
    return 0;
}
