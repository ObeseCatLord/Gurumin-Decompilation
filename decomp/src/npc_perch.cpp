// An NPC that perches on a neighbour (0x100b3770, behaviour 10 of Char_RunBehaviour 0x100dfcf0): a rider that sits on
// the head of the character `talk / 100` slots before it, copies that character's heading and, in two modes, turns to
// the player and now and then spits an effect at them.

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

#include "npc_perch.h"

#define PERCH_MODE_FOLLOW 0
#define PERCH_MODE_SPIT 1
#define PERCH_MODE_SPIT_BURSTS 2
#define PERCH_MODE_CARRIED 3
#define PERCH_MODE_DIVISOR 100
#define PERCH_MOVE_MODE_NORMAL 1
#define PERCH_HEAD_NODE "Bip01 Head"
#define PERCH_HEAD_REACH_SQUARED 10000.0f
#define PERCH_UNPLACED_X (-20000.0f)
#define PERCH_SEAT_HEIGHT 45.0f
#define PERCH_TURN_OFFSET 3.1415f
#define PERCH_FIRST_TURN_FRAMES 60
#define PERCH_SPIT_CHANCE_RANGE 10000
#define PERCH_SPIT_CHANCE 500
#define PERCH_SPIT_RANGE 1500.0f
#define PERCH_MOUTH_FRONT 50.0f
#define PERCH_TARGET_HEIGHT 50.0f
#define PERCH_AIM_MIN_LENGTH 10.0f
#define PERCH_MAX_SPITS 3
#define PERCH_BURST_PAUSE_ONE (-120)
#define PERCH_BURST_PAUSE_TWO (-60)
#define PERCH_BURST_START 1000
#define PERCH_BURST_STEP 15
#define PERCH_SPIT_EFFECT 600
#define PERCH_SPIT_SOUND 0x2cd

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_10043cc0(int index);                                       // Char_ResetState
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

// 0x100b3770 (Npc_PerchOnNeighbour). `range` is the distance to the player, then a scratch float of the spit. The neighbour is the slot `talk / 100` below its own. While the character is
// flagged in DAT_102550e0 it only follows the neighbour (sits 45 units above its head, or above its feet when the
// head is 100 or more units away or the neighbour is unplaced). Otherwise state 0 picks the neighbour, finds the
// "Bip01 Head" node and stores the mode (talk % 100); mode 0 copies the neighbour's heading, modes 1 and 2 turn to
// face the player. In mode 1 it spits every so often (a 5 % chance per frame once 60 frames passed; after four
// spits a long pause), mode 2 also spits every 15 frames of a burst. A spit is effect 600 from a point 50 units in
// front of the head, tilted towards the player's chest (acos of flat over full distance), with sound 0x2cd, only
// when the player is within 1500 units. A neighbour that is gone or dead resets the state. Returns 0.
int Npc_PerchOnNeighbour(int index)
{
    Vec4 playerPosition;
    Vec4 selfPosition;
    playerPosition.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    selfPosition.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float range = (playerPosition - selfPosition).Length();
    DAT_10db6ee8[index].mode = 0;
    DAT_10255a88[index] = 0;
    if (DAT_102550e0[index] != 0)
    {
        if (DAT_1020b430[index].neighbour <= 0 ||
            DAT_10daaae8[DAT_1020b430[index].neighbour].stats[STAT_HEALTH] <= 0 ||
            DAT_10210d30[DAT_1020b430[index].neighbour] != 0)
        {
            DAT_10db6ee8[index].mode = PERCH_MOVE_MODE_NORMAL;
            FUN_10043cc0(index);
            return 0;
        }
        if (DAT_10210d30[index] != 0)
            return 0;
        if (DAT_10daaae8[index].stats[STAT_HEALTH] <= 0)
            return 0;
        Vec4 seat;
        if (DAT_1021bdf8[DAT_1020b430[index].neighbour][0] != 0 &&
            (DAT_1021bdf8[DAT_1020b430[index].neighbour][0]->worldMatrix.GetTranslation() -
             DAT_10db6ee8[DAT_1020b430[index].neighbour].matrix.GetTranslation()).LengthSquared3() <
                PERCH_HEAD_REACH_SQUARED &&
            DAT_10db6ee8[DAT_1020b430[index].neighbour].matrix.translation.x > PERCH_UNPLACED_X)
            seat.Copy(DAT_1021bdf8[DAT_1020b430[index].neighbour][0]->worldMatrix.GetTranslation());
        else
            seat.Copy(DAT_10db6ee8[DAT_1020b430[index].neighbour].matrix.GetTranslation());
        DAT_10db6ee8[index].SetPosition(seat.x, seat.y, seat.z + PERCH_SEAT_HEIGHT);
        return 0;
    }
    if (DAT_1020b430[index].sawPlayer != 0)
        DAT_1020b430[index].sawPlayer = 0;
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].state = 1;
        DAT_1020b430[index].neighbour = index - DAT_1020b430[index].talk / PERCH_MODE_DIVISOR;
        DAT_1020b430[index].perchMode = DAT_1020b430[index].talk % PERCH_MODE_DIVISOR;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(PERCH_HEAD_NODE, 0);
        if (DAT_1020b430[index].perchMode == PERCH_MODE_CARRIED)
            DAT_1020b430[DAT_1020b430[index].neighbour].shotCount = 1;
    }
    else if (DAT_1020b430[index].state == 1)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        if (DAT_1020b430[index].perchMode == PERCH_MODE_FOLLOW)
        {
            if (DAT_1020b430[index].neighbour > 0)
                DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[DAT_1020b430[index].neighbour].targetHeading;
        }
        else if (DAT_1020b430[index].perchMode == PERCH_MODE_SPIT)
        {
            DAT_10db6ee8[index].targetHeading =
                atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + PERCH_TURN_OFFSET;
            DAT_1020b430[index].wait++;
            if (DAT_1020b430[index].wait > PERCH_FIRST_TURN_FRAMES &&
                FUN_1013f880() % PERCH_SPIT_CHANCE_RANGE < PERCH_SPIT_CHANCE)
            {
                DAT_1020b430[index].shotCount++;
                if (DAT_1020b430[index].shotCount > PERCH_MAX_SPITS)
                {
                    DAT_1020b430[index].wait = PERCH_BURST_PAUSE_ONE;
                    DAT_1020b430[index].shotCount = 0;
                }
                else
                    DAT_1020b430[index].wait -= PERCH_FIRST_TURN_FRAMES;
                if (DAT_1030b8f8[index] != 0 && range < PERCH_SPIT_RANGE)
                {
                    EffectOptions options;
                    Matrix4 matrix;
                    Vec4 mouth;
                    matrix.SetIdentity();
                    mouth.y = PERCH_MOUTH_FRONT;
                    mouth.RotateZ(PERCH_TURN_OFFSET - DAT_10db6ee8[index].heading);
                    if (DAT_1021bdf8[index][0] != 0)
                        mouth.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() + mouth);
                    else
                        mouth.Copy(selfPosition + mouth);
                    Vec4 from;
                    Vec4 target;
                    from.Copy(mouth);
                    target.Copy(playerPosition);
                    target.z = target.z + PERCH_TARGET_HEIGHT;
                    if ((from - target).Length() < PERCH_AIM_MIN_LENGTH)
                        matrix.SetIdentity();
                    else if (from.z > target.z)
                    {
                        range = (from - target).Length2D();
                        matrix = RotationX(-acosf(range / (from - target).Length()));
                    }
                    else
                    {
                        range = (from - target).Length2D();
                        matrix = RotationX(acosf(range / (from - target).Length()));
                    }
                    matrix = matrix * RotationZ(DAT_10db6ee8[index].heading);
                    matrix.SetTranslation(mouth);
                    range = 0.0f;
                    mouth.Fill(range);
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0000 = 0;
                    FUN_1001d8b0(index, PERCH_SPIT_EFFECT, mouth, -1, -1, 0, 0, 0, &matrix, &options);
                    FUN_10065b10(index, PERCH_SPIT_SOUND, 0, 0);
                }
            }
        }
        else if (DAT_1020b430[index].perchMode == PERCH_MODE_SPIT_BURSTS)
        {
            DAT_10db6ee8[index].targetHeading =
                atan2f(playerPosition.x - selfPosition.x, playerPosition.y - selfPosition.y) + PERCH_TURN_OFFSET;
            DAT_1020b430[index].wait++;
            if ((DAT_1020b430[index].wait > PERCH_FIRST_TURN_FRAMES &&
                 FUN_1013f880() % PERCH_SPIT_CHANCE_RANGE < PERCH_SPIT_CHANCE) ||
                (DAT_1020b430[index].wait > PERCH_BURST_START && DAT_1020b430[index].wait % PERCH_BURST_STEP == 0))
            {
                if (DAT_1020b430[index].shotCount == 0)
                    DAT_1020b430[index].wait = PERCH_BURST_START;
                DAT_1020b430[index].shotCount++;
                if (DAT_1020b430[index].shotCount > PERCH_MAX_SPITS)
                {
                    DAT_1020b430[index].wait = PERCH_BURST_PAUSE_TWO;
                    DAT_1020b430[index].shotCount = 0;
                }
                if (DAT_1030b8f8[index] != 0 && range < PERCH_SPIT_RANGE)
                {
                    EffectOptions options;
                    Matrix4 matrix;
                    Vec4 mouth;
                    matrix.SetIdentity();
                    mouth.y = PERCH_MOUTH_FRONT;
                    mouth.RotateZ(PERCH_TURN_OFFSET - DAT_10db6ee8[index].heading);
                    if (DAT_1021bdf8[index][0] != 0)
                        mouth.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() + mouth);
                    else
                        mouth.Copy(selfPosition + mouth);
                    Vec4 from;
                    Vec4 target;
                    from.Copy(mouth);
                    target.Copy(playerPosition);
                    target.z = target.z + PERCH_TARGET_HEIGHT;
                    if ((from - target).Length() < PERCH_AIM_MIN_LENGTH)
                        matrix.SetIdentity();
                    else if (from.z > target.z)
                    {
                        range = (from - target).Length2D();
                        matrix = RotationX(-acosf(range / (from - target).Length()));
                    }
                    else
                    {
                        range = (from - target).Length2D();
                        matrix = RotationX(acosf(range / (from - target).Length()));
                    }
                    matrix = matrix * RotationZ(DAT_10db6ee8[index].heading);
                    matrix.SetTranslation(mouth);
                    range = 0.0f;
                    mouth.Fill(range);
                    options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    options.m_0000 = 0;
                    FUN_1001d8b0(index, PERCH_SPIT_EFFECT, mouth, -1, -1, 0, 0, 0, &matrix, &options);
                    FUN_10065b10(index, PERCH_SPIT_SOUND, 0, 0);
                }
            }
        }
    }
    if (DAT_1020b430[index].neighbour <= 0 || DAT_10daaae8[DAT_1020b430[index].neighbour].stats[STAT_HEALTH] <= 0 ||
        DAT_10210d30[DAT_1020b430[index].neighbour] != 0)
    {
        DAT_10db6ee8[index].mode = PERCH_MOVE_MODE_NORMAL;
        FUN_10043cc0(index);
        return 0;
    }
    if (DAT_10210d30[index] != 0)
        return 0;
    if (DAT_10daaae8[index].stats[STAT_HEALTH] <= 0)
        return 0;
    {
        Vec4 seat;
        if (DAT_1021bdf8[DAT_1020b430[index].neighbour][0] != 0 &&
            (DAT_1021bdf8[DAT_1020b430[index].neighbour][0]->worldMatrix.GetTranslation() -
             DAT_10db6ee8[DAT_1020b430[index].neighbour].matrix.GetTranslation()).LengthSquared3() <
                PERCH_HEAD_REACH_SQUARED &&
            DAT_10db6ee8[DAT_1020b430[index].neighbour].matrix.translation.x > PERCH_UNPLACED_X)
            seat.Copy(DAT_1021bdf8[DAT_1020b430[index].neighbour][0]->worldMatrix.GetTranslation());
        else
            seat.Copy(DAT_10db6ee8[DAT_1020b430[index].neighbour].matrix.GetTranslation());
        DAT_10db6ee8[index].SetPosition(seat.x, seat.y, seat.z + PERCH_SEAT_HEIGHT);
        DAT_10254598[DAT_1020b430[index].neighbour] = 0;
    }
    return 0;
}
