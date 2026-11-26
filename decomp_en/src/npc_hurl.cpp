// An NPC that hurls a missile at the player (EN build; JP: decomp/src/npc_hurl.cpp, behaviour 0x26): it watches the player,
// winds up with its right hand and throws effect 0x36c8 along a randomly scattered arc.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_inline.h"
#include "companion_update.h"
#include "effect_manager.h"
#include "effect_options.h"
#include "engine.h"
#include "node.h"
#include "npc_hurl.h"
#include "vec4.h"
#include "visibility.h"
#include "weapon_attach.h"

// CharState::values indices the thrower keeps its state in.
const int HURL_VALUE_WAIT = 0;           // frames in the current step
const int HURL_VALUE_TALK = 1;           // talk id: three digits, reaction, retry and reach
const int HURL_VALUE_REACT = 2;          // talk % 10: a ready thrower throws with a chance of 2 * this + 2 percent per frame
const int HURL_VALUE_RETRY = 3;          // (talk / 10) % 10: after a throw the next one starts when a roll of 100 reaches (this + 1) * 3
const int HURL_VALUE_REACH = 4;          // (talk / 100) % 10: throwing range in hundreds of units
const int HURL_VALUE_AIMING = 5;         // 1 while the thrower holds the missile and aims (its charge effect shows)
const int HURL_VALUE_HIT = 6;            // 1 once the thrower was hit (it stops waiting and acts at once)
const int HURL_VALUE_WOUND = 7;          // 1 once the wind-up motion reached its held pose
const int HURL_VALUE_CARRY_HEIGHT = 8;   // height the carried character is held at (set by the carrier)
const int HURL_VALUE_CARRIED = 9;        // non-zero while a carrier holds the thrower

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
inline float WeightedLength(const Vec4 &offset)
{
    Vec4 weighted = offset;
    weighted.z = offset.z * HURL_HEIGHT_WEIGHT;
    return weighted.Length();
}

// Npc_HurlAtPlayer. talk holds three digits: reaction (talk % 10), retry ((talk / 10) % 10) and reach
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
    playerPosition.Copy(g_characterModels[0].matrix.GetTranslation());
    selfPosition.Copy(g_characterModels[index].matrix.GetTranslation());
    (playerPosition - selfPosition).Length();
    float range = WeightedLength(playerPosition - selfPosition);

    g_charTalkPending[index] = 0;
    if (g_charState[index].values[HURL_VALUE_HIT] == 0)
        g_charHurlMark[index] = (char)0xff;
    if (g_charHitFlag[index] != 0)
    {
        g_charState[index].values[HURL_VALUE_HIT] = 1;
        g_charHurlMark[index] = 0;
        if (g_charSetupCode[index] != 0)
            Char_DropEquipment(index, 0, 0);
    }
    if (g_charNodes[index][0] != 0 && g_charNodes[index][1] != 0)
    {
        Vec4 forearm;
        forearm.Copy(g_charNodes[index][1]->worldMatrix.GetTranslation() -
                     g_charNodes[index][0]->worldMatrix.GetTranslation());
        handPosition.Copy(g_charNodes[index][1]->worldMatrix.GetTranslation() + forearm);
        handPosition.z = handPosition.z + HURL_HAND_RISE;
    }
    else
        handPosition.Copy(selfPosition);
    if (g_charState[index].state == HURL_STATE_START)
    {
        g_charState[index].state = HURL_STATE_WATCH;
        g_charState[index].values[HURL_VALUE_REACT] = g_charState[index].values[HURL_VALUE_TALK] % HURL_DIGIT_BASE;
        g_charState[index].values[HURL_VALUE_RETRY] = g_charState[index].values[HURL_VALUE_TALK] / HURL_DIGIT_BASE % HURL_DIGIT_BASE;
        g_charState[index].values[HURL_VALUE_REACH] = g_charState[index].values[HURL_VALUE_TALK] / HURL_REACH_UNIT % HURL_DIGIT_BASE;
        g_charNodes[index][0] = g_characterModels[index].root->Find(HURL_HAND_NODE, 0);
        g_charNodes[index][1] = g_characterModels[index].root->Find(HURL_FOREARM_NODE, 0);
    }
    else if (g_charState[index].state == HURL_STATE_WATCH)
    {
        g_characterModels[index].PlayMotion(HURL_MOTION_IDLE, 0, HURL_MOTION_BLEND, 1.0f);
        float turn = Atan2(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + HURL_TURN_OFFSET;
        g_characterModels[index].targetHeading = turn;
        turn = turn - g_characterModels[index].heading;
        while (turn > HURL_TURN_OFFSET)
            turn = turn - HURL_FULL_TURN;
        while (turn < -HURL_TURN_OFFSET)
            turn = turn + HURL_FULL_TURN;
        if ((float)fabs(turn) > HURL_FACE_TOLERANCE)
            g_characterModels[index].PlayMotion(0, 0, HURL_MOTION_BLEND, 1.0f);
        if (range < (float)(g_charState[index].values[HURL_VALUE_REACH] * HURL_REACH_UNIT))
            g_charState[index].state = HURL_STATE_WIND_UP;
        if (g_charState[index].values[HURL_VALUE_HIT] != 0)
            Char_ResetState(index);
    }
    else if (g_charState[index].state == HURL_STATE_WIND_UP)
    {
        if (g_charState[index].values[HURL_VALUE_WOUND] != 0)
            g_characterModels[index].PlayMotion(HURL_MOTION_HELD, 0, HURL_MOTION_BLEND, 1.0f);
        else
            g_characterModels[index].PlayMotion(HURL_MOTION_WIND_UP, 0, HURL_MOTION_BLEND, 1.0f);

        if (g_characterModels[index].playback->layer == HURL_MOTION_HELD)
            g_charState[index].values[HURL_VALUE_WOUND] = 1;
        if (g_charState[index].values[HURL_VALUE_AIMING] != 0)
        {
            if ((g_characterModels[index].playback->layer == HURL_MOTION_HELD ||
                 g_characterModels[index].playback->layer == HURL_MOTION_HELD_LOOP) &&
                (((float)(g_charState[index].values[HURL_VALUE_REACH] * HURL_REACH_UNIT) > range &&
                  Random() % HURL_CHANCE_RANGE <
                      g_charState[index].values[HURL_VALUE_REACT] * HURL_REACT_STEP + HURL_REACT_BASE) ||
                 g_charState[index].values[HURL_VALUE_HIT] != 0))
                g_charState[index].state = HURL_STATE_THROW;
        }
        else if (g_characterModels[index].motionFrame >= HURL_AIM_FRAME)
            g_charState[index].values[HURL_VALUE_AIMING] = 1;
        float aim = Atan2(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + HURL_TURN_OFFSET;
        g_characterModels[index].targetHeading = aim;
        float turn = aim - g_characterModels[index].heading;
        while (turn > HURL_TURN_OFFSET)
            turn = turn - HURL_FULL_TURN;
        while (turn < -HURL_TURN_OFFSET)
            turn = turn + HURL_FULL_TURN;
        if (turn > HURL_TURN_STEP_LIMIT)
            turn = HURL_TURN_STEP_LIMIT;
        else if (turn < -HURL_TURN_STEP_LIMIT)
            turn = -HURL_TURN_STEP_LIMIT;
        g_characterModels[index].targetHeading = turn + g_characterModels[index].heading;
        if (g_charState[index].values[HURL_VALUE_AIMING] == 0 && g_charState[index].values[HURL_VALUE_HIT] != 0)
            Char_ResetState(index);
    }
    else if (g_charState[index].state == HURL_STATE_THROW)
    {
        g_charState[index].values[HURL_VALUE_WOUND] = 0;
        g_characterModels[index].PlayMotion(HURL_MOTION_THROW, 0, HURL_MOTION_BLEND, 1.0f);
        g_charState[index].values[HURL_VALUE_WAIT]++;
        int frame = g_characterModels[index].motionFrame;
        if (g_characterModels[index].motionFrame == HURL_RELEASE_FRAME && g_charState[index].values[HURL_VALUE_AIMING] != 0)
        {
            Character_PlaySound(index, HURL_RELEASE_SOUND, 0, 0);
            float reach = range / HURL_FULL_REACH;
            g_charState[index].values[HURL_VALUE_AIMING] = 0;
            float heading = g_characterModels[index].targetHeading;
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
                g_collisionFilter = g_characterModels[index].collisionFilter;
                if (g_mapRootNode != 0 && g_mapRootNode->Collide(0, HURL_PROBE_MASK, &from, &to, &hit))
                    pitch = HURL_FLAT_PITCH - (1.0f - reach * reach) * HURL_FLAT_PITCH;
                else
                    pitch = (1.0f - reach * reach) * HURL_FLAT_PITCH + HURL_FLAT_PITCH;
                g_collisionFilter = 0;
            }
            heading = heading + ((float)(Random() % HURL_SCATTER_STEPS) - HURL_SCATTER_CENTRE) * HURL_TURN_OFFSET /
                                    HURL_SCATTER_YAW_DIVISOR;
            pitch = pitch + ((float)(Random() % HURL_SCATTER_STEPS) - HURL_SCATTER_CENTRE) * HURL_TURN_OFFSET /
                                HURL_SCATTER_PITCH_DIVISOR;
            Matrix4 matrix;
            EffectOptions options;
            Vec4 origin;
            matrix = RotationX(pitch) * RotationZ(heading);
            matrix.SetTranslation(handPosition);
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.m_0000 = 0;
            options.scale = HURL_SHOT_SPEED;
            options.m_000e = HURL_SHOT_BLEND;
            options.m_0020 = 1;
            Char_StartEffect(index, HURL_SHOT_EFFECT, origin, -1, -1, 0, 0, 0, &matrix, &options);
        }
        else if (frame == HURL_RETRY_FRAME)
        {
            if (g_charState[index].values[HURL_VALUE_HIT] == 0 &&
                Random() % HURL_CHANCE_RANGE >= (g_charState[index].values[HURL_VALUE_RETRY] + 1) * HURL_RETRY_STEP)
            {
                g_characterModels[index].targetHeading =
                    atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + HURL_TURN_OFFSET;
                g_charState[index].values[HURL_VALUE_AIMING] = 1;
                g_characterModels[index].PlayMotion(HURL_MOTION_THROW, 1, HURL_MOTION_BLEND, 1.0f);
                g_charState[index].values[HURL_VALUE_WAIT] = 0;
            }
            else
                g_charState[index].state = HURL_STATE_WATCH;
        }
    }
    if (g_charState[index].values[HURL_VALUE_CARRIED] != 0 && g_charNodes[index][8] != 0)
    {
        Vec4 position;
        position.Copy(g_charNodes[index][8]->worldMatrix.GetTranslation());
        position.z = (float)g_charState[index].values[HURL_VALUE_CARRY_HEIGHT];
        position.Copy(position - selfPosition);
        if (position.Length() > HURL_CARRY_REACH)
        {
            position.Normalized3();
            position.Copy(position * HURL_CARRY_REACH);
        }
        position.Copy(position + selfPosition);
        g_characterModels[index].SetPosition(position.x, position.y, position.z);
    }
    if (g_charState[index].values[HURL_VALUE_AIMING] != 0)
    {
        Matrix4 matrix;
        EffectOptions options;
        Vec4 origin;
        matrix.SetIdentity();
        matrix.SetTranslation(handPosition);
        options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
        g_effectManager.Start(HURL_CHARGE_EFFECT, origin, -1, -1, 0, 0, 0, &matrix, &options, &g_charEffectHandle[index]);
    }
    return 0;
}
