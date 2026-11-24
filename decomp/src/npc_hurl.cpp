// An NPC that hurls a missile at the player (0x100b6740, behaviour 0x26 of Char_RunBehaviour 0x100dfcf0): it watches
// the player, winds up with its right hand and throws effect 0x36c8 along a randomly scattered arc.

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
#include "world.h"
#include "map_vars.h"

#include "npc_hurl.h"

#define HURL_STATE_START 0
#define HURL_STATE_WATCH 1
#define HURL_STATE_WIND_UP 2
#define HURL_STATE_THROW 3
#define HURL_HAND_NODE "Bip01 R Finger0"
#define HURL_FOREARM_NODE "Dummy01"
#define HURL_DIGIT_BASE 10
#define HURL_REACH_UNIT 100
#define HURL_HEIGHT_WEIGHT 3.0f
#define HURL_HAND_RISE 30.0f
#define HURL_TURN_OFFSET 3.1415f
#define HURL_FULL_TURN 6.283f
#define HURL_FACE_TOLERANCE 0.1f
#define HURL_TURN_STEP_LIMIT 0.1f
#define HURL_MOTION_IDLE 2
#define HURL_MOTION_WIND_UP 0x1a
#define HURL_MOTION_HELD 0x1b
#define HURL_MOTION_HELD_LOOP 0x1c
#define HURL_MOTION_THROW 0x1d
#define HURL_MOTION_BLEND 5
#define HURL_AIM_FRAME 0xea
#define HURL_RELEASE_FRAME 0xf8
#define HURL_RETRY_FRAME 0x108
#define HURL_CHANCE_RANGE 100
#define HURL_REACT_BASE 2
#define HURL_REACT_STEP 2
#define HURL_RETRY_STEP 3
#define HURL_RELEASE_SOUND 0x1c7
#define HURL_FULL_REACH 950.0f
#define HURL_FLAT_PITCH 0.7854f
#define HURL_SCATTER_STEPS 30
#define HURL_SCATTER_CENTRE 15.0f
#define HURL_SCATTER_YAW_DIVISOR 360.0f
#define HURL_SCATTER_PITCH_DIVISOR 720.0f
#define HURL_PROBE_MASK 4
#define HURL_PROBE_RISE 650.0f
#define HURL_SHOT_RISE 50.0f
#define HURL_SHOT_EFFECT 0x36c8
#define HURL_SHOT_SPEED 0.7f
#define HURL_SHOT_BLEND 5
#define HURL_CHARGE_EFFECT 0x683
#define HURL_CARRY_REACH 144.0f

// The game's atan2 wrapper (float arguments, so each difference is rounded to a float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Length of an offset with its height counted three times.
inline float WeightedLength(Vec4 offset)
{
    offset.z = offset.z * HURL_HEIGHT_WEIGHT;
    return offset.Length();
}

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_10043cc0(int index);                                       // Char_ResetState
#include "char_equipment.h"
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

// 0x100b6740 (Npc_HurlAtPlayer). talk holds three digits: reaction (talk % 10), retry ((talk / 10) % 10) and reach
// ((talk / 100) % 10). State 0 reads them and finds the "Bip01 R Finger0" and "Dummy01" nodes. State 1 turns to the
// player and starts the wind-up once the player is closer than reach * 100 (distance with the height tripled). State 2
// plays the wind-up and, once it holds, aims (turning at most 0.1 per frame) and goes on to throwing when a roll of
// 2 * reaction + 2 percent succeeds or the thrower was hit. State 3 releases at motion frame 0xf8: effect 0x36c8 from
// the hand, tilted by the distance (a wall in the way flips the arc) and scattered by up to 15 degrees, then at frame
// 0x108 either aims again or goes back to state 1. A carried thrower is moved to the carrier's node, and while aiming a
// charge effect (0x683) follows the hand. Returns 0.
int Npc_HurlAtPlayer(int index)
{
    Vec4 playerPosition;
    Vec4 selfPosition;
    Vec4 handPosition;
    playerPosition.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    selfPosition.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    (playerPosition - selfPosition).Length();
    float range = WeightedLength(playerPosition - selfPosition);

    DAT_10255a88[index] = 0;
    if (DAT_1020b430[index].hurlHit == 0)
        DAT_10219e40[index] = (char)0xff;
    if (DAT_1042d9e8[index] != 0)
    {
        DAT_1020b430[index].hurlHit = 1;
        DAT_10219e40[index] = 0;
        if (DAT_102548c0[index] != 0)
            FUN_100615f0(index, 0, 0);
    }
    if (DAT_1021bdf8[index][0] != 0 && DAT_1021bdf8[index][1] != 0)
    {
        Vec4 forearm;
        forearm.Copy(DAT_1021bdf8[index][1]->worldMatrix.GetTranslation() -
                     DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
        handPosition.Copy(DAT_1021bdf8[index][1]->worldMatrix.GetTranslation() + forearm);
        handPosition.z = handPosition.z + HURL_HAND_RISE;
    }
    else
        handPosition.Copy(selfPosition);
    if (DAT_1020b430[index].state == HURL_STATE_START)
    {
        DAT_1020b430[index].state = HURL_STATE_WATCH;
        DAT_1020b430[index].hurlReactDigit = DAT_1020b430[index].talk % HURL_DIGIT_BASE;
        DAT_1020b430[index].hurlRetryDigit = DAT_1020b430[index].talk / HURL_DIGIT_BASE % HURL_DIGIT_BASE;
        DAT_1020b430[index].hurlReach = DAT_1020b430[index].talk / HURL_REACH_UNIT % HURL_DIGIT_BASE;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(HURL_HAND_NODE, 0);
        DAT_1021bdf8[index][1] = DAT_10db6ee8[index].root->Find(HURL_FOREARM_NODE, 0);
    }
    else if (DAT_1020b430[index].state == HURL_STATE_WATCH)
    {
        DAT_10db6ee8[index].PlayMotion(HURL_MOTION_IDLE, 0, HURL_MOTION_BLEND, 1.0f);
        float turn = Atan2(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + HURL_TURN_OFFSET;
        DAT_10db6ee8[index].targetHeading = turn;
        turn = turn - DAT_10db6ee8[index].heading;
        while (turn > HURL_TURN_OFFSET)
            turn = turn - HURL_FULL_TURN;
        while (turn < -HURL_TURN_OFFSET)
            turn = turn + HURL_FULL_TURN;
        if ((float)fabs(turn) > HURL_FACE_TOLERANCE)
            DAT_10db6ee8[index].PlayMotion(0, 0, HURL_MOTION_BLEND, 1.0f);
        if (range < (float)(DAT_1020b430[index].hurlReach * HURL_REACH_UNIT))
            DAT_1020b430[index].state = HURL_STATE_WIND_UP;
        if (DAT_1020b430[index].hurlHit != 0)
            FUN_10043cc0(index);
    }
    else if (DAT_1020b430[index].state == HURL_STATE_WIND_UP)
    {
        if (DAT_1020b430[index].hurlWound != 0)
            DAT_10db6ee8[index].PlayMotion(HURL_MOTION_HELD, 0, HURL_MOTION_BLEND, 1.0f);
        else
            DAT_10db6ee8[index].PlayMotion(HURL_MOTION_WIND_UP, 0, HURL_MOTION_BLEND, 1.0f);

        if (DAT_10db6ee8[index].motion->layer == HURL_MOTION_HELD)
            DAT_1020b430[index].hurlWound = 1;
        if (DAT_1020b430[index].hurlAiming != 0)
        {
            if ((DAT_10db6ee8[index].motion->layer == HURL_MOTION_HELD ||
                 DAT_10db6ee8[index].motion->layer == HURL_MOTION_HELD_LOOP) &&
                (((float)(DAT_1020b430[index].hurlReach * HURL_REACH_UNIT) > range &&
                  FUN_1013f880() % HURL_CHANCE_RANGE <
                      DAT_1020b430[index].hurlReactDigit * HURL_REACT_STEP + HURL_REACT_BASE) ||
                 DAT_1020b430[index].hurlHit != 0))
                DAT_1020b430[index].state = HURL_STATE_THROW;
        }
        else if (DAT_10db6ee8[index].motionFrame >= HURL_AIM_FRAME)
            DAT_1020b430[index].hurlAiming = 1;
        float aim = Atan2(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + HURL_TURN_OFFSET;
        DAT_10db6ee8[index].targetHeading = aim;
        float turn = aim - DAT_10db6ee8[index].heading;
        while (turn > HURL_TURN_OFFSET)
            turn = turn - HURL_FULL_TURN;
        while (turn < -HURL_TURN_OFFSET)
            turn = turn + HURL_FULL_TURN;
        if (turn > HURL_TURN_STEP_LIMIT)
            turn = HURL_TURN_STEP_LIMIT;
        else if (turn < -HURL_TURN_STEP_LIMIT)
            turn = -HURL_TURN_STEP_LIMIT;
        DAT_10db6ee8[index].targetHeading = turn + DAT_10db6ee8[index].heading;
        if (DAT_1020b430[index].hurlAiming == 0 && DAT_1020b430[index].hurlHit != 0)
            FUN_10043cc0(index);
    }
    else if (DAT_1020b430[index].state == HURL_STATE_THROW)
    {
        DAT_1020b430[index].hurlWound = 0;
        DAT_10db6ee8[index].PlayMotion(HURL_MOTION_THROW, 0, HURL_MOTION_BLEND, 1.0f);
        int frame = DAT_10db6ee8[index].motionFrame;
        DAT_1020b430[index].wait++;
        if (DAT_10db6ee8[index].motionFrame == HURL_RELEASE_FRAME && DAT_1020b430[index].hurlAiming != 0)
        {
            FUN_10065b10(index, HURL_RELEASE_SOUND, 0, 0);
            float reach = range / HURL_FULL_REACH;
            DAT_1020b430[index].hurlAiming = 0;
            float heading = DAT_10db6ee8[index].targetHeading;
            float pitch;
            if (reach > 1.0f)
                pitch = HURL_FLAT_PITCH;
            else
            {
                Vec4 from;
                Vec4 to;
                Vec4 hit;
                from.Copy(selfPosition);
                from.z = from.z + HURL_SHOT_RISE;
                to.Copy(from);
                to.z = to.z + HURL_PROBE_RISE;
                DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
                if (DAT_10255f08 != 0 && DAT_10255f08->Collide(0, HURL_PROBE_MASK, &from, &to, &hit))
                    pitch = HURL_FLAT_PITCH - (1.0f - reach * reach) * HURL_FLAT_PITCH;
                else
                    pitch = ((1.0f - reach * reach) + 1.0f) * HURL_FLAT_PITCH;
                DAT_11b002c0 = 0;
            }
            heading = heading + ((float)(FUN_1013f880() % HURL_SCATTER_STEPS) - HURL_SCATTER_CENTRE) * HURL_TURN_OFFSET /
                                    HURL_SCATTER_YAW_DIVISOR;
            pitch = pitch + ((float)(FUN_1013f880() % HURL_SCATTER_STEPS) - HURL_SCATTER_CENTRE) * HURL_TURN_OFFSET /
                                HURL_SCATTER_PITCH_DIVISOR;
            Matrix4 matrix;
            EffectOptions options;
            Vec4 origin;
            matrix = RotationX(pitch) * RotationZ(heading);
            matrix.SetTranslation(handPosition);
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0000 = 0;
            options.m_0008 = HURL_SHOT_SPEED;
            options.m_000e = HURL_SHOT_BLEND;
            options.m_0020 = 1;
            FUN_1001d8b0(index, HURL_SHOT_EFFECT, origin, -1, -1, 0, 0, 0, &matrix, &options);
        }
        else if (frame == HURL_RETRY_FRAME)
        {
            if (DAT_1020b430[index].hurlHit == 0 &&
                FUN_1013f880() % HURL_CHANCE_RANGE >= (DAT_1020b430[index].hurlRetryDigit + 1) * HURL_RETRY_STEP)
            {
                DAT_10db6ee8[index].targetHeading =
                    atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + HURL_TURN_OFFSET;
                DAT_1020b430[index].hurlAiming = 1;
                DAT_10db6ee8[index].PlayMotion(HURL_MOTION_THROW, 1, HURL_MOTION_BLEND, 1.0f);
                DAT_1020b430[index].wait = 0;
            }
            else
                DAT_1020b430[index].state = HURL_STATE_WATCH;
        }
    }
    if (DAT_1020b430[index].hurlCarried != 0 && DAT_1021bdf8[index][8] != 0)
    {
        Vec4 position;
        position.Copy(DAT_1021bdf8[index][8]->worldMatrix.GetTranslation());
        position.z = (float)DAT_1020b430[index].hurlCarryHeight;
        position.Copy(position - selfPosition);
        if (position.Length() > HURL_CARRY_REACH)
        {
            position.Normalized3();
            position.Copy(position * HURL_CARRY_REACH);
        }
        position.Copy(position + selfPosition);
        DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
    }
    if (DAT_1020b430[index].hurlAiming != 0)
    {
        Matrix4 matrix;
        EffectOptions options;
        Vec4 origin;
        matrix.SetIdentity();
        matrix.SetTranslation(handPosition);
        options.followOwner = 1;
        DAT_10daaa80.Start(HURL_CHARGE_EFFECT, origin, -1, -1, 0, 0, 0, &matrix, &options, &DAT_10219770[index]);
    }
    return 0;
}
