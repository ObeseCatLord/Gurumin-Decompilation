// The rolling ball scene (behaviour update 0x1007a370 of the per-kind update 0x100f9670): a character is dropped
// into the scene from above, then a big ball (the map's "@ball" node) rolls over the map, throwing up dust and
// sparks, knocking the player back when it passes close and rumbling with a 3D sound until it stops.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "last_position.h"

#include "char_state.h"

#include "char_slot.h"

#include "char_tables.h"

#include "map_records.h"

#include "map_vars.h"

#include "doppler_loops.h"

#include "effect_manager.h"

#include "game.h"

#include "game_vars.h"

#include "char_spawn.h"

#include "rolling_ball.h"

#define BALL_STATE_DROP_IN 1
#define BALL_STATE_DROP_LAST 0x1e
#define BALL_STATE_ROLL 1000
#define BALL_STATE_LINK 2000
#define BALL_STATE_RUMBLE 3000
#define BALL_PHASE_FIND_BALL 0
#define BALL_PHASE_DROP_IN 1
#define BALL_PHASE_DROPPED 2
#define BALL_DROPPED_KIND 0x2b67
#define BALL_DROP_HEIGHT 750.0f
#define BALL_DROP_START 0x2ee
#define BALL_DROP_STEP 20
#define BALL_LANDING_FLAG 0x19a
#define BALL_FLAG_LAST 3999
#define BALL_NEAR_SCENES 8
#define BALL_FILLER_CHARACTER 0x826
#define BALL_FILLER_X 750.0f
#define BALL_FILLER_Y 1950.0f
#define BALL_FILLER_Z 150.0f
#define BALL_NODE "@ball"
#define BALL_CAPTION "454536L"
#define BALL_LOOP_SOUND 0x237
#define BALL_FLAME_FIRST 0x3f8
#define BALL_FLAME_LAST 0x551
#define BALL_SPARK_FRAME 0x550
#define BALL_DUST_PERIOD 20
#define BALL_DUST_SKIP_FIRST 0x53c
#define BALL_DUST_SKIP_LAST 0x54c
#define BALL_DUST_DROP 100.0f
#define BALL_TRAIL_EFFECT 0x45d
#define BALL_DUST_EFFECT 0x2df
#define BALL_SPARK_EFFECT 0x2de
#define BALL_TRAIL_PARAM 0xf
#define BALL_TRAIL_FLAG 5
#define BALL_SPARK_FRAMES 5
#define BALL_HIT_RANGE 140.0f
#define BALL_HIT_LOW -100.0f
#define BALL_HIT_HIGH 200.0f
#define BALL_PI 3.1415f
#define BALL_SOUND_TRACK_UNTIL 0x550
#define BALL_SOUND_SCALE 0.15f
#define BALL_EVENT_FRAME 0x166
#define BALL_EVENT 0x130
#define BALL_END_FRAME 0x578
#define BALL_RUMBLE_START 0x11e
#define BALL_RUMBLE_PULSE 0x186
#define BALL_RUMBLE_END 0x11f
#define BALL_RUMBLE_END_FRAME 200

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

extern "C" int FUN_10017bb0(int id);
extern "C" int FUN_1016b230(const char *text);
extern "C" void FUN_100102a0(int stream);
extern "C" int FUN_10055ee0(void);
Matrix4 RotationZ(float angle);   // 0x101b1b40

// 0x1007a370 (Char_RollingBall). One frame of the ball scene character `index`. On its first frame it takes the
// position and state from its map record. State 1 waits for the scene to start (DAT_10c63524 = 1) and then drops
// a new character in (states 2..30 lower it; at the last one its kind is restored). State 1000 finds the "@ball"
// node, starts the caption, the rumbling sound and the "roll" motion, then every frame follows the ball: the
// position and velocity feed the 3D sound, every 20 frames (and with a fiery trail in the middle part) it throws
// up dust, the player is knocked back when it is within reach, and at the end the ball is let go. State 2000
// links the nodes' frame sources to the counters, state 3000 plays the rumble sounds. Returns 0.
int Char_RollingBall(int index)
{
    if (DAT_1020b430[index].ballFrame == 0)
    {
        DAT_10db6ee8[index].mode = 0;
        DAT_10db6ee8[index].m_0267 = 1;
        DAT_10db6ee8[index].m_17e8 = 1;
        int record = DAT_10db6ee8[index].bankIndex;
        DAT_10daaae8[index].m_0141 = 1;
        if (record > 0)
        {
            DAT_1020b430[index].state = DAT_10210fc0[record].points[0].x;
            DAT_10802bf8[index].position.x = (float)DAT_10210fc0[record].points[0].y;
            DAT_10802bf8[index].position.y = (float)DAT_10210fc0[record].points[0].z;
            DAT_10802bf8[index].position.z = (float)DAT_10210fc0[record].points[0].code;
        }
        DAT_1020b430[index].ballFrame = 1;
    }
    if (DAT_1020b430[index].state == BALL_STATE_DROP_IN)
    {
        if (DAT_10c63524 != BALL_PHASE_DROP_IN)
            return 0;
        if (DAT_10c655e8 >= BALL_NEAR_SCENES)
        {
            int filler = FUN_10017bb0(BALL_FILLER_CHARACTER);
            DAT_10db6ee8[filler].SetPosition(BALL_FILLER_X, BALL_FILLER_Y, BALL_FILLER_Z);
            int flag = DAT_10c655e8 + BALL_LANDING_FLAG;
            DAT_102199b8[filler] = 1;
            DAT_10c63524 = BALL_PHASE_DROPPED;
            if (flag <= BALL_FLAG_LAST && flag != 0)
                DAT_10c62edc[flag] = 1;
        }
        else
        {
            int dropped = FUN_10050510(DAT_10802bf8[index].position.x, DAT_10802bf8[index].position.y, DAT_10802bf8[index].position.z, 0, -DAT_10c655e8);
            int flag = DAT_10c655e8 + BALL_LANDING_FLAG;
            DAT_10254090[dropped] = 0;
            DAT_10c63524 = BALL_PHASE_DROPPED;
            if (flag <= BALL_FLAG_LAST && flag != 0)
                DAT_10c62edc[flag] = 1;
            DAT_1020b430[index].state = 2;
            int kind = DAT_10daaae8[dropped].kind;
            DAT_10db6ee8[dropped].m_238c = 0;
            DAT_10db6ee8[dropped].mode = 1;
            DAT_1020b430[index].ballSlot = dropped;
            DAT_1020b430[index].ballSavedKind = kind;
            DAT_10daaae8[dropped].kind = BALL_DROPPED_KIND;
            DAT_10db6ee8[dropped].matrix.m[14] = BALL_DROP_HEIGHT;
            DAT_102199b8[dropped] = 1;
            return 0;
        }
        return 0;
    }
    if (DAT_1020b430[index].state == BALL_STATE_DROP_LAST)
    {
        DAT_10daaae8[DAT_1020b430[index].ballSlot].kind = DAT_1020b430[index].ballSavedKind;
        DAT_1020b430[index].state = BALL_STATE_DROP_IN;
        return 0;
    }
    if (DAT_1020b430[index].state > 1 && DAT_1020b430[index].state < BALL_STATE_DROP_LAST)
    {
        int next = DAT_1020b430[index].state + 1;
        DAT_1020b430[index].state = next;
        DAT_10db6ee8[DAT_1020b430[index].ballSlot].matrix.m[14] = (float)(BALL_DROP_START - next * BALL_DROP_STEP);
        return 0;
    }
    if (DAT_1020b430[index].state == BALL_STATE_ROLL)
    {
        if (DAT_10c63524 != BALL_PHASE_FIND_BALL)
        {
            DAT_1020b430[index].ballFrame = BALL_STATE_ROLL;
            if (DAT_10255f08 != 0)
                DAT_1021bdf8[index][0] = DAT_10255f08->Find(BALL_NODE, 0);
            if (DAT_1021bdf8[index][0] == 0)
            {
                DAT_1020b430[index].ballFrame = 1;
            }
            else
            {
                DAT_10802bf8[index].position.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
                float zero = 0;
                DAT_10802bf8[index].step.Fill(zero);
            }
            DAT_10c63524 = BALL_PHASE_FIND_BALL;
        }
        if (DAT_1020b430[index].ballFrame < BALL_STATE_ROLL)
            return 0;
        if (DAT_1020b430[index].ballFrame == BALL_STATE_ROLL)
        {
            FUN_1016b230(BALL_CAPTION);
            DAT_10c635d4 = FUN_1003ad20(BALL_LOOP_SOUND, &DAT_10802bf8[index].position, 1.0f);
            DAT_1021bdf8[index][0]->RestartColourPlugs("roll", 0);
            DAT_1021bdf8[index][0]->frameSource = &DAT_1020b430[index].ballRollFrame;
            DAT_1020b430[index].ballRollFrame = -1;
        }
        DAT_1020b430[index].ballFrame++;
        DAT_1020b430[index].ballRollFrame++;
        DAT_10802bf8[index].step.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() - DAT_10802bf8[index].position);
        DAT_10802bf8[index].position.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
        if (DAT_1020b430[index].ballFrame >= BALL_FLAME_FIRST && DAT_1020b430[index].ballFrame <= BALL_FLAME_LAST)
        {
            Matrix4 matrix;
            Vec4 offset;
            EffectOptions options;
            options.m_0000 = 0;
            options.m_0004 = BALL_TRAIL_PARAM;
            options.m_000e = BALL_TRAIL_FLAG;
            matrix = RotationZ(Atan2(DAT_10802bf8[index].step.x, DAT_10802bf8[index].step.y) + BALL_PI);
            matrix.SetTranslation(DAT_10802bf8[index].position);
            DAT_10daaa80.Start(BALL_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options, 0);
            if (DAT_1020b430[index].ballFrame < BALL_DUST_SKIP_FIRST || DAT_1020b430[index].ballFrame > BALL_DUST_SKIP_LAST)
            {
                matrix.m[14] = matrix.m[14] - BALL_DUST_DROP;
                DAT_10daaa80.Start(BALL_DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
            }
            if (DAT_1020b430[index].ballFrame == BALL_SPARK_FRAME)
            {
                DAT_10daaa80.Start(BALL_SPARK_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
                DAT_10daaa80.Start(BALL_SPARK_EFFECT, offset, -1, -1, 0, 0, BALL_SPARK_FRAMES, &matrix, 0, 0);
                if (DAT_10c635d4 > -1)
                {
                    FUN_100102a0(BALL_LOOP_SOUND);
                    DAT_10c635d4 = -1;
                }
            }
            offset.Copy(DAT_10802bf8[index].position - DAT_10db6ee8[0].matrix.GetTranslation());
            if (offset.Length2D() < BALL_HIT_RANGE && offset.z > BALL_HIT_LOW && offset.z < BALL_HIT_HIGH)
                FUN_10055ee0();
        }
        else if (DAT_1020b430[index].ballFrame % BALL_DUST_PERIOD == 0)
        {
            Matrix4 matrix;
            Vec4 offset;
            matrix = RotationZ(Atan2(DAT_10802bf8[index].step.x, DAT_10802bf8[index].step.y) + BALL_PI);
            matrix.SetTranslation(DAT_10802bf8[index].position);
            matrix.m[14] = matrix.m[14] - BALL_DUST_DROP;
            DAT_10daaa80.Start(BALL_DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
        }
        if (DAT_1020b430[index].ballFrame < BALL_SOUND_TRACK_UNTIL && DAT_10c635d4 > -1)
        {
            Vec4 soundPosition;
            Vec4 soundVelocity;
            soundPosition.Copy(DAT_10802bf8[index].position * BALL_SOUND_SCALE);
            soundVelocity.Copy(DAT_10802bf8[index].step * BALL_SOUND_SCALE);
            int stream = DAT_10c635d4;
        FUN_10001890(stream, &soundPosition.x, &soundPosition.y, &soundPosition.z, &soundVelocity.x,
                         &soundVelocity.y, &soundVelocity.z);
        }
        if (DAT_1020b430[index].ballRollFrame == BALL_EVENT_FRAME)
            DAT_10874848.RegisterEvent(BALL_EVENT);
        if (DAT_1020b430[index].ballFrame == BALL_END_FRAME)
        {
            DAT_1020b430[index].ballFrame = 1;
            DAT_1021bdf8[index][0]->frameSource = 0;
            DAT_1021bdf8[index][0]->RestartColourPlugs("end", DAT_104bf8a4);
        }
    }
    else if (DAT_1020b430[index].state == BALL_STATE_LINK)
    {
        Node *first = DAT_1021bdf8[index][0];
        if (first != 0)
            first->frameSource = &DAT_1020b430[index].ballFrame;
        Node *second = DAT_1021bdf8[index][1];
        if (second != 0)
        {
            DAT_1020b430[index].ballSlot++;
            second->frameSource = &DAT_1020b430[index].ballSlot;
        }
        Node *third = DAT_1021bdf8[index][2];
        if (third != 0)
        {
            DAT_1020b430[index].ballSavedKind++;
            third->frameSource = &DAT_1020b430[index].ballSavedKind;
        }
    }
    else if (DAT_1020b430[index].state == BALL_STATE_RUMBLE)
    {
        if (DAT_1020b430[index].ballFrame == 1)
        {
            DAT_1020b430[index].ballFrame = 2;
            DAT_1020b430[index].ballTimer = 0;
        }
        if (DAT_10c63524 != 0)
        {
            DAT_1020b430[index].ballTimer++;
            if (DAT_1020b430[index].ballTimer == 1)
                FUN_1003ad20(BALL_RUMBLE_START, 0, 1.0f);
            if (DAT_1020b430[index].ballTimer > 0x19 && DAT_1020b430[index].ballTimer % 10 == 0 &&
                DAT_1020b430[index].ballTimer < 0xe1)
                FUN_1003ad20(BALL_RUMBLE_PULSE, 0, 1.0f);
            if (DAT_1020b430[index].ballTimer == BALL_RUMBLE_END_FRAME)
                FUN_1003ad20(BALL_RUMBLE_END, 0, 1.0f);
        }
        else
        {
            DAT_1020b430[index].ballTimer = 0;
        }
    }
    return 0;
}
