// An NPC that performs a gesture and cheers along with a neighbour (0x100b9150, behaviour 0x2e of Char_RunBehaviour
// 0x100dfcf0): the character `talk / 100` slots before it is the one it watches.

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

#define GESTURE_FIRST_NEIGHBOUR 0x3a
#define GESTURE_NEIGHBOUR_LIMIT 0x5c
#define GESTURE_STATE_START 0
#define GESTURE_STATE_PERFORM 1
#define GESTURE_STATE_BOW 6
#define GESTURE_STATE_CHEER 99
#define GESTURE_STATE_DONE 100
#define GESTURE_STATE_SLEEP 0x65
#define GESTURE_VOICE_FRAME 5
#define GESTURE_VOICE 0x152
#define GESTURE_SHOW_BEFORE 0xf
#define GESTURE_SHOW_END 0x23
#define GESTURE_END_FRAME 0x36
#define GESTURE_BOW_FRAMES 0x3c
#define GESTURE_CHEER_FRAMES 0xd
#define GESTURE_SLEEP_FRAMES 0x1d
#define GESTURE_CHEER_MOTION 0x5a
#define GESTURE_CHEER_VARIANTS 4
#define GESTURE_SPARK_EFFECT 0x665
#define GESTURE_SPARK_HEIGHT 175.0f
#define GESTURE_SPARK_SPREAD 50.0f
#define GESTURE_SPARK_SOUND_ARMED 0x1d7
#define GESTURE_SPARK_SOUND_CHEER 0x1d8
#define GESTURE_BOW_CHANCE 10
#define GESTURE_WAKE_RANGE 1500.0f
#define GESTURE_TURN_OFFSET 3.1415f

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1001b670(int index, float range, int count);               // Enemies_WakeNearby
extern "C" int FUN_10043cc0(int index);                                       // Char_ResetState
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

// 0x100b9150 (Npc_GestureWithNeighbour). The character it watches is slot `talk / 100` below its own (it must be a
// character slot 0x3a..0x5b that is not flagged in DAT_10210d30, else the state is reset). State 1 plays a gesture
// (motion 5, voice 0x152 on frame 5, "gesturing" for frames 16..34 so the neighbours can react) until frame 54, then
// state 100 goes back to state 1 (or sleeps in state 101 once it saw the player) and sometimes (10 %) bows (state 6,
// motion 4, 60 frames). A character whose neighbour is gesturing cheers (state 99, motion 0x5a..0x5d, 13 frames) with a
// spark effect 0x665 above it (175 units up, scattered 50 units) and sound 0x1d7 or 0x1d8. It wakes the enemies
// within 1500 units and is flagged as having seen the player when the player comes within the talk id's units * 10
// (or it was told to react). Returns 0.
int FUN_100b9150(int index)
{
    Vec4 player;
    Vec4 self;
    Vec4 neighbour;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float distance = StretchedLength(player - self);
    if (DAT_1020b430[index].state != GESTURE_STATE_START)
    {
        if (DAT_1020b430[index].neighbour < GESTURE_FIRST_NEIGHBOUR ||
            DAT_1020b430[index].neighbour >= GESTURE_NEIGHBOUR_LIMIT)
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
    if (DAT_1020b430[index].state == GESTURE_STATE_START)
    {
        int neighbourSlot = index - DAT_1020b430[index].talk / 100;
        DAT_1020b430[index].neighbour = neighbourSlot;
        DAT_1020b430[index].state = GESTURE_STATE_PERFORM;
        if (neighbourSlot < GESTURE_FIRST_NEIGHBOUR || neighbourSlot >= GESTURE_NEIGHBOUR_LIMIT ||
            DAT_10210d30[neighbourSlot] != 0)
        {
            FUN_10043cc0(index);
            return 0;
        }
        DAT_10db6ee8[index].targetHeading =
            Atan2(DAT_10db6ee8[neighbourSlot].matrix.translation.x - self.x,
                  DAT_10db6ee8[neighbourSlot].matrix.translation.y - self.y) + GESTURE_TURN_OFFSET;
    }
    else if (DAT_1020b430[index].state == GESTURE_STATE_PERFORM)
    {
        DAT_1020b430[index].wait++;
        if (DAT_1020b430[index].wait == 1)
            DAT_10db6ee8[index].PlayMotion(5, 1, 5, 1.0f);
        if (DAT_1020b430[index].wait == GESTURE_VOICE_FRAME)
            FUN_10065b10(index, GESTURE_VOICE, 0, 0);
        if (DAT_1020b430[index].wait > GESTURE_SHOW_BEFORE && DAT_1020b430[index].wait < GESTURE_SHOW_END)
            DAT_1020b430[index].gesturing = 1;
        else
            DAT_1020b430[index].gesturing = 0;
        if (DAT_1020b430[index].wait == GESTURE_END_FRAME)
        {
            DAT_10db6ee8[index].PlayMotion(4, 1, 5, 1.0f);
            DAT_1020b430[index].state = GESTURE_STATE_DONE;
        }
    }
    else if (DAT_1020b430[index].state == GESTURE_STATE_BOW)
    {
        DAT_10db6ee8[index].PlayMotion(4, 0, 5, 1.0f);
        if (++DAT_1020b430[index].wait > GESTURE_BOW_FRAMES)
            DAT_1020b430[index].state = GESTURE_STATE_DONE;
    }
    else if (DAT_1020b430[index].state == GESTURE_STATE_CHEER)
    {
        DAT_1020b430[index].gesturing = 0;
        DAT_10db6ee8[index].PlayMotion(DAT_1020b430[index].cheerMotion, 0, 5, 1.0f);
        if (++DAT_1020b430[index].wait > GESTURE_CHEER_FRAMES)
            DAT_1020b430[index].state = GESTURE_STATE_DONE;
    }
    else if (DAT_1020b430[index].state == GESTURE_STATE_DONE)
    {
        DAT_1020b430[index].cheerReady = 0;
        DAT_1020b430[index].wait = 0;
        if (DAT_1020b430[index].sawPlayer == 0)
            DAT_1020b430[index].state = GESTURE_STATE_PERFORM;
        else if (DAT_1020b430[index].sawPlayer != 0)
            DAT_1020b430[index].state = GESTURE_STATE_SLEEP;
        if (FUN_1013f880() % 100 < GESTURE_BOW_CHANCE && DAT_1020b430[index].state != GESTURE_STATE_SLEEP)
            DAT_1020b430[index].state = GESTURE_STATE_BOW;
    }
    else if (DAT_1020b430[index].state == GESTURE_STATE_SLEEP)
    {
        DAT_10db6ee8[index].PlayMotion(6, 0, 5, 1.0f);
        DAT_1020b430[index].wait++;
        if (DAT_1020b430[index].wait != 1 &&
            (DAT_1020b430[index].wait == GESTURE_SLEEP_FRAMES || DAT_10210b30[index] != 0))
            FUN_10043cc0(index);
    }
    if (DAT_1020b430[index].state != GESTURE_STATE_SLEEP)
    {
        if (DAT_1020b430[DAT_1020b430[index].neighbour].gesturing != 0 &&
            DAT_1020b430[index].state != GESTURE_STATE_CHEER)
        {
            if (DAT_1020b430[index].cheerReady == 0)
                DAT_1020b430[index].cheerPending = 1;
            else
            {
                Matrix4 matrix;
                Vec4 offset;
                matrix.SetIdentity();
                offset.Copy(self);
                offset.z += GESTURE_SPARK_HEIGHT;
                offset.x = (float)(FUN_1013f880() % 100) - GESTURE_SPARK_SPREAD + offset.x;
                offset.y = (float)(FUN_1013f880() % 100) - GESTURE_SPARK_SPREAD + offset.y;
                offset.z = (float)(FUN_1013f880() % 100) - GESTURE_SPARK_SPREAD + offset.z;
                matrix = RotationZ(DAT_10db6ee8[index].heading);
                matrix.SetTranslation(offset);
                offset.Fill(0.0f);
                FUN_1001d8b0(index, GESTURE_SPARK_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0);
                FUN_10065b10(index, GESTURE_SPARK_SOUND_ARMED, 0, 0);
            }
        }
        if (DAT_1020b430[index].cheerPending != 0)
        {
            DAT_1020b430[index].state = GESTURE_STATE_CHEER;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].cheerMotion = FUN_1013f880() % GESTURE_CHEER_VARIANTS + GESTURE_CHEER_MOTION;
            DAT_10db6ee8[index].PlayMotion(DAT_1020b430[index].cheerMotion, 1, 5, 1.0f);
            DAT_1020b430[index].cheerPending = 0;
            DAT_1020b430[index].gesturing = 0;
            Matrix4 matrix;
            Vec4 offset;
            matrix.SetIdentity();
            offset.Copy(self);
            offset.z += GESTURE_SPARK_HEIGHT;
            offset.x = (float)(FUN_1013f880() % 100) - GESTURE_SPARK_SPREAD + offset.x;
            offset.y = (float)(FUN_1013f880() % 100) - GESTURE_SPARK_SPREAD + offset.y;
            offset.z = (float)(FUN_1013f880() % 100) - GESTURE_SPARK_SPREAD + offset.z;
            matrix = RotationZ(DAT_10db6ee8[index].heading);
            matrix.SetTranslation(offset);
            offset.Fill(0.0f);
            FUN_1001d8b0(index, GESTURE_SPARK_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0);
            FUN_10065b10(index, GESTURE_SPARK_SOUND_CHEER, 0, 0);
        }
        if (distance < (float)(DAT_1020b430[index].talk % 100 * 10) || DAT_10210b30[index] != 0)
        {
            FUN_1001b670(index, GESTURE_WAKE_RANGE, 0);
            DAT_1020b430[index].sawPlayer = 1;
            DAT_10210b30[index] = 0;
        }
    }
    return 0;
}
