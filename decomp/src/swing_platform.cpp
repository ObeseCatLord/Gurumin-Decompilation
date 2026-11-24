// A character swinging on an arm round a pivot (behaviour of kind 0x0e, run from the per-kind update 0x100f9670): it
// hangs from a pivot (the map record's first point, or 900 units above its start), swings back and forth like a
// pendulum with a speed that grows by an acceleration each frame, and turns its "@O_056:Layer" node with the swing. The
// angle limits it reached over ground are kept so the shadow can be put on the ground it passes over.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "last_position.h"

#include "char_state.h"

#include "char_slot.h"

#include "char_tables.h"

#include "char_vars.h"

#include "char_ground.h"

#include "world.h"

#include "effect_manager.h"

#include "map_records.h"

#include "swing_platform.h"

Matrix4 TranslationMatrix(float x, float y, float z);   // 0x101b1360
Matrix4 RotationY(float angle);                         // 0x101b1bb0

inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

inline float Fabs(float value)
{
    return (float)fabs(value);
}

#define SWING_PLAY_LOW -20000.0f
#define SWING_DEFAULT_LIFT 900.0f
#define SWING_DEFAULT_ANGLE 30.0f
#define SWING_DEFAULT_ACCELERATION -0.15f
#define SWING_RECORD_UNIT 10000.0f
#define SWING_ARM_EXTRA 50
#define SWING_GROUND_NONE -3500
#define SWING_GROUND_LOW -3000.0f
#define SWING_GROUND_ROUND 0.5f
#define SWING_SCALE 0.2f
#define SWING_LAYER_NODE "@O_056:Layer"
#define SWING_GROUND_PERIOD 7
#define SWING_PI 3.1415f
#define SWING_HALF_PI 1.57075f
#define SWING_TWO_PI 6.283f
#define SWING_HALF_TURN 180.0f
#define SWING_FULL_TURN 360.0f
#define SWING_ROLL_PER_UNIT 0.007f
#define SWING_RUNNING 7
#define SWING_LEFT -1

// The roll of the layer node is kept in the first float after its saved matrix.
#define SWING_ROLL DAT_10868838[index][1].m[0]

// 0x10025ce0 (Char_SwingPlatform). One frame of the swinging character `index`. The first call (swingReady 0) puts the
// pivot where the map record says (record points[0], in 1/10000; the swing angle, speed and acceleration come from
// points[1]) or 900 units above the character, finds the "@O_056:Layer" node and measures the arm. Then every frame:
// the swing angle and speed advance, the arm is turned to the angle and the character is put at the end of it; the
// roll of the layer node follows the swing and its matrix is rebuilt. The highest ground and the angle limits over
// ground are tracked every seventh frame. Returns 1 when the character is gone, else 0.
int Char_SwingPlatform(int index)
{
    if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
        DAT_10db6ee8[index].mode = 0;
    DAT_10db6ee8[index].locked = 1;
    DAT_10255a88[index] = 0;
    DAT_10db6ee8[index].uniformScale = SWING_SCALE;
    DAT_10daaae8[index].m_0141 = 1;
    if (DAT_10210d30[index] != 0)
        return 1;
    if (DAT_104483e8[index] == 0)
        return 0;
    if (DAT_10210b30[index] != 0)
        DAT_10210b30[index] = 0;
    DAT_10218fc0[index] = 0;
    if (DAT_1020b430[index].swingReady == 0)
    {
        Vec4 here;
        if (DAT_10db6ee8[index].matrix.m[12] < SWING_PLAY_LOW)
            return 0;
        int record = DAT_10daaae8[index].routeRecord;
        DAT_1020b430[index].swingReady = 1;
        if (record != 0)
        {
            DAT_1020b430[index].swingFlip = DAT_10210fc0[record].points[0].code;
            DAT_10802bf8[index].position.x = (float)DAT_10210fc0[record].points[0].x / SWING_RECORD_UNIT;
            DAT_10802bf8[index].position.y = (float)DAT_10210fc0[record].points[0].y / SWING_RECORD_UNIT;
            DAT_10802bf8[index].position.z = (float)DAT_10210fc0[record].points[0].z / SWING_RECORD_UNIT;
            DAT_10255160[index][0] = (float)DAT_10210fc0[record].points[1].x / SWING_RECORD_UNIT;
            DAT_10255160[index][1] = (float)DAT_10210fc0[record].points[1].y / SWING_RECORD_UNIT;
            DAT_10255160[index][2] = (float)DAT_10210fc0[record].points[1].z / SWING_RECORD_UNIT;
            DAT_1020b430[index].swingMirror = DAT_10210fc0[record].points[1].code;
            here.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            float base = -(Atan2(here.x - DAT_10802bf8[index].position.x, here.y - DAT_10802bf8[index].position.y) + SWING_PI);
            DAT_10255160[index][3] = base;
            if (DAT_10210fc0[DAT_10daaae8[index].routeRecord].points[1].code != 0)
                DAT_10db6ee8[index].targetHeading = -base - SWING_HALF_PI;
            else
                DAT_10db6ee8[index].targetHeading = SWING_HALF_PI - base;
        }
        else
        {
            DAT_10802bf8[index].position.x = DAT_10db6ee8[index].matrix.m[12];
            DAT_10802bf8[index].position.y = DAT_10db6ee8[index].matrix.m[13];
            DAT_10802bf8[index].position.z = DAT_10db6ee8[index].matrix.m[14] + SWING_DEFAULT_LIFT;
            DAT_1020b430[index].swingFlip = 1;
            DAT_1020b430[index].swingMirror = 1;
            DAT_10255160[index][3] = SWING_HALF_PI - DAT_10db6ee8[index].heading;
            DAT_10255160[index][0] = SWING_DEFAULT_ANGLE;
            DAT_10255160[index][1] = 0;
            DAT_10255160[index][2] = SWING_DEFAULT_ACCELERATION;
            DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading;
            DAT_1020b430[index].swingMaxAngle = 0;
            DAT_1020b430[index].swingMinAngle = 0;
        }
        here.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        DAT_1020b430[index].swingLength = (int)(here - DAT_10802bf8[index].position).Length() + SWING_ARM_EXTRA;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(SWING_LAYER_NODE, 0);
        DAT_10868838[index][0] = DAT_1021bdf8[index][0]->matrix;
        DAT_1020b430[index].swingGround = SWING_GROUND_NONE;
    }
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    Vec4 line;
    // The original constructs these three (a Vec4, an EffectOptions and a Matrix4) and never reads them.
    EffectOptions options;
    Vec4 spare;
    Matrix4 unused;
    DAT_10daaae8[index].stats[STAT_BUSY] = 1;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    toPlayer.Length();
    toPlayer.Length2D();
    if (DAT_104c0380 % SWING_GROUND_PERIOD == index % SWING_GROUND_PERIOD)
    {
        DAT_10db6ee8[index].m_0012 = 2;
        FUN_1001c090(index, 1);
        if (DAT_10db6ee8[index].shadowMatrix.m[14] > SWING_GROUND_LOW)
        {
            if (DAT_10db6ee8[index].shadowMatrix.m[14] > (float)DAT_1020b430[index].swingGround)
                DAT_1020b430[index].swingGround = (int)(DAT_10db6ee8[index].shadowMatrix.m[14] + SWING_GROUND_ROUND);
            if (DAT_10255160[index][0] > 0.0f)
            {
                if ((float)DAT_1020b430[index].swingMaxAngle < DAT_10255160[index][0])
                    DAT_1020b430[index].swingMaxAngle = (int)DAT_10255160[index][0];
            }
            else if ((float)DAT_1020b430[index].swingMinAngle > DAT_10255160[index][0])
                DAT_1020b430[index].swingMinAngle = (int)DAT_10255160[index][0];
        }
    }
    if ((float)DAT_1020b430[index].swingMinAngle <= DAT_10255160[index][0] &&
        DAT_10255160[index][0] <= (float)DAT_1020b430[index].swingMaxAngle)
        DAT_10db6ee8[index].shadowMatrix.m[14] = (float)DAT_1020b430[index].swingGround;
    else
        DAT_10db6ee8[index].shadowMatrix.m[14] = SWING_GROUND_NONE;
    while (DAT_10255160[index][0] > SWING_HALF_TURN)
        DAT_10255160[index][0] -= SWING_FULL_TURN;
    while (DAT_10255160[index][0] < -SWING_HALF_TURN)
        DAT_10255160[index][0] += SWING_FULL_TURN;
    float oldAngle = DAT_10255160[index][0];
    float newSpeed = DAT_10255160[index][2] + DAT_10255160[index][1];
    DAT_10255160[index][1] = newSpeed;
    float newAngle = newSpeed + DAT_10255160[index][0];
    DAT_10255160[index][0] = newAngle;
    line.z = -(float)DAT_1020b430[index].swingLength;
    line.RotateY(newAngle / SWING_HALF_TURN * SWING_PI);
    line.RotateZ(DAT_10255160[index][3] + SWING_HALF_PI);
    if ((Fabs(DAT_10255160[index][2]) * 2.0f < DAT_10255160[index][1] && DAT_1020b430[index].swingMirror == 0) ||
        (DAT_10255160[index][1] < Fabs(DAT_10255160[index][2]) * -2.0f && DAT_1020b430[index].swingMirror != 0))
    {
        float arm = (line - DAT_10802bf8[index].step).Length();
        SWING_ROLL += arm * SWING_ROLL_PER_UNIT;
    }
    else if ((Fabs(DAT_10255160[index][2]) * 2.0f < DAT_10255160[index][1] && DAT_1020b430[index].swingMirror != 0) ||
             (DAT_10255160[index][1] < Fabs(DAT_10255160[index][2]) * -2.0f && DAT_1020b430[index].swingMirror == 0))
    {
        float arm = (line - DAT_10802bf8[index].step).Length();
        SWING_ROLL -= arm * SWING_ROLL_PER_UNIT;
    }
    else
        DAT_1020b430[index].swingStatus = SWING_RUNNING;
    while (SWING_ROLL > SWING_TWO_PI)
        SWING_ROLL -= SWING_TWO_PI;
    while (SWING_ROLL < -SWING_TWO_PI)
        SWING_ROLL += SWING_TWO_PI;
    DAT_1021bdf8[index][0]->matrix = RotationY(-SWING_ROLL) * TranslationMatrix(DAT_10868838[index][0].m[12],
                                                                              DAT_10868838[index][0].m[13],
                                                                              DAT_10868838[index][0].m[14]);
    DAT_10802bf8[index].step.Copy(line);
    if (((oldAngle < 0.0f && DAT_10255160[index][0] > 0.0f) || (oldAngle > 0.0f && DAT_10255160[index][0] < 0.0f) ||
         DAT_10255160[index][0] == 0.0f || DAT_10255160[index][0] == SWING_HALF_TURN ||
         DAT_10255160[index][0] == -SWING_HALF_TURN) &&
        DAT_1020b430[index].swingFlip != 0)
        DAT_10255160[index][2] = DAT_10255160[index][2] * -1.0f;
    if (DAT_104bfd08 == 0)
        DAT_10db6ee8[index].SetPosition(line.x + DAT_10802bf8[index].position.x, line.y + DAT_10802bf8[index].position.y,
                                        line.z + DAT_10802bf8[index].position.z);
    if (DAT_102550e0[index] != 0)
        DAT_1020b430[index].swingStatus = SWING_LEFT;
    DAT_1021f6c0[index] = 1;
    return 0;
}
