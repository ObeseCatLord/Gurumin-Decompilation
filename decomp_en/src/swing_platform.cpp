// A character swinging on an arm round a pivot (EN build; JP: decomp/src/swing_platform.cpp). It hangs from a pivot (the map
// record's first point, or 900 units above its start), swings back and forth like a pendulum with a speed that grows by an
// acceleration each frame, and turns its "@O_056:Layer" node with the swing. The angle limits it reached over ground are
// kept so the shadow can be put on the ground it passes over.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_shots.h"
#include "collide_query.h"
#include "effect_options.h"
#include "enemy_flee.h"
#include "enemy_moves_long.h"
#include "map_records.h"
#include "node.h"
#include "swing_platform.h"
#include "vec4.h"

// CharState::values indices the swing keeps its state in.
const int SWING_VALUE_READY = 1;       // 1 once the platform was set up
const int SWING_VALUE_STATUS = 2;      // 7 = the swing is running, -1 = the character was told to leave
const int SWING_VALUE_FLIP = 3;        // non-zero = the acceleration flips when the angle passes 0 or +-180
const int SWING_VALUE_LENGTH = 4;      // length of the arm in units (distance from the pivot + 50)
const int SWING_VALUE_MIRROR = 5;      // non-zero = the swing turns the other way round
const int SWING_VALUE_GROUND = 7;      // highest ground under the swing so far (-3500 = none)
const int SWING_VALUE_MAX_ANGLE = 9;   // largest swing angle seen over ground
const int SWING_VALUE_MIN_ANGLE = 10;  // smallest swing angle seen over ground

// Heading of the vector (x, y) from the y axis, as the game's own float wrapper of atan2, and the float absolute value.
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
#define SWING_ROLL g_characterNodeMatrices[index][1].m[0]

// Char_SwingPlatform. One frame of the swinging character `index`. The first call (swingReady 0) puts the
// pivot where the map record says (record points[0], in 1/10000; the swing angle, speed and acceleration come from
// points[1]) or 900 units above the character, finds the "@O_056:Layer" node and measures the arm. Then every frame:
// the swing angle and speed advance, the arm is turned to the angle and the character is put at the end of it; the
// roll of the layer node follows the swing and its matrix is rebuilt. The highest ground and the angle limits over
// ground are tracked every seventh frame. Returns 1 when the character is gone, else 0.
int Char_SwingPlatform(int index)
{
    if (g_charRecords[index].stats[STAT_HEALTH] > 0)
        g_characterModels[index].mode = 0;
    g_characterModels[index].locked = 1;
    g_charTalkPending[index] = 0;
    g_characterModels[index].uniformScale = SWING_SCALE;
    g_charRecords[index].defeated = 1;
    if (g_charPhase[index] != 0)
        return 1;
    if (g_charAwake[index] == 0)
        return 0;
    if (g_charToldToReact[index] != 0)
        g_charToldToReact[index] = 0;
    g_charClearedEachFrame[index] = 0;
    if (g_charState[index].values[SWING_VALUE_READY] == 0)
    {
        Vec4 here;
        if (g_characterModels[index].matrix.m[12] < SWING_PLAY_LOW)
            return 0;
        int record = g_charRecords[index].routeRecord;
        g_charState[index].values[SWING_VALUE_READY] = 1;
        if (record != 0)
        {
            g_charState[index].values[SWING_VALUE_FLIP] = g_mapRecords[record].points[0].code;
            g_shotSlots[index].position.x = (float)g_mapRecords[record].points[0].x / SWING_RECORD_UNIT;
            g_shotSlots[index].position.y = (float)g_mapRecords[record].points[0].y / SWING_RECORD_UNIT;
            g_shotSlots[index].position.z = (float)g_mapRecords[record].points[0].z / SWING_RECORD_UNIT;
            g_shotParams[index][0] = (float)g_mapRecords[record].points[1].x / SWING_RECORD_UNIT;
            g_shotParams[index][1] = (float)g_mapRecords[record].points[1].y / SWING_RECORD_UNIT;
            g_shotParams[index][2] = (float)g_mapRecords[record].points[1].z / SWING_RECORD_UNIT;
            g_charState[index].values[SWING_VALUE_MIRROR] = g_mapRecords[record].points[1].code;
            here.Copy(g_characterModels[index].matrix.GetTranslation());
            float base = -(Atan2(here.x - g_shotSlots[index].position.x, here.y - g_shotSlots[index].position.y) + SWING_PI);
            g_shotParams[index][3] = base;
            if (g_mapRecords[g_charRecords[index].routeRecord].points[1].code != 0)
                g_characterModels[index].targetHeading = -base - SWING_HALF_PI;
            else
                g_characterModels[index].targetHeading = SWING_HALF_PI - base;
        }
        else
        {
            g_shotSlots[index].position.x = g_characterModels[index].matrix.m[12];
            g_shotSlots[index].position.y = g_characterModels[index].matrix.m[13];
            g_shotSlots[index].position.z = g_characterModels[index].matrix.m[14] + SWING_DEFAULT_LIFT;
            g_charState[index].values[SWING_VALUE_FLIP] = 1;
            g_charState[index].values[SWING_VALUE_MIRROR] = 1;
            g_shotParams[index][3] = SWING_HALF_PI - g_characterModels[index].heading;
            g_shotParams[index][0] = SWING_DEFAULT_ANGLE;
            g_shotParams[index][1] = 0;
            g_shotParams[index][2] = SWING_DEFAULT_ACCELERATION;
            g_characterModels[index].targetHeading = g_characterModels[index].heading;
            g_charState[index].values[SWING_VALUE_MAX_ANGLE] = 0;
            g_charState[index].values[SWING_VALUE_MIN_ANGLE] = 0;
        }
        here.Copy(g_characterModels[index].matrix.GetTranslation());
        g_charState[index].values[SWING_VALUE_LENGTH] = (int)(here - g_shotSlots[index].position).Length() + SWING_ARM_EXTRA;
        g_charNodes[index][0] = g_characterModels[index].root->Find(SWING_LAYER_NODE, 0);
        g_characterNodeMatrices[index][0] = g_charNodes[index][0]->matrix;
        g_charState[index].values[SWING_VALUE_GROUND] = SWING_GROUND_NONE;
    }
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    Vec4 line;
    // The original constructs these three (a Vec4, an EffectOptions and a Matrix4) and never reads them.
    EffectOptions options;
    Vec4 spare;
    Matrix4 unused;
    g_charRecords[index].stats[STAT_BUSY] = 1;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    toPlayer.Length();
    toPlayer.Length2D();
    if (g_frameCounter % SWING_GROUND_PERIOD == index % SWING_GROUND_PERIOD)
    {
        g_characterModels[index].m_0012 = 2;
        Char_PlaceOnGround(index, 1);
        if (g_characterModels[index].shadowMatrix.m[14] > SWING_GROUND_LOW)
        {
            if (g_characterModels[index].shadowMatrix.m[14] > (float)g_charState[index].values[SWING_VALUE_GROUND])
                g_charState[index].values[SWING_VALUE_GROUND] = (int)(g_characterModels[index].shadowMatrix.m[14] + SWING_GROUND_ROUND);
            if (g_shotParams[index][0] > 0.0f)
            {
                if ((float)g_charState[index].values[SWING_VALUE_MAX_ANGLE] < g_shotParams[index][0])
                    g_charState[index].values[SWING_VALUE_MAX_ANGLE] = (int)g_shotParams[index][0];
            }
            else if ((float)g_charState[index].values[SWING_VALUE_MIN_ANGLE] > g_shotParams[index][0])
                g_charState[index].values[SWING_VALUE_MIN_ANGLE] = (int)g_shotParams[index][0];
        }
    }
    if ((float)g_charState[index].values[SWING_VALUE_MIN_ANGLE] <= g_shotParams[index][0] &&
        g_shotParams[index][0] <= (float)g_charState[index].values[SWING_VALUE_MAX_ANGLE])
        g_characterModels[index].shadowMatrix.m[14] = (float)g_charState[index].values[SWING_VALUE_GROUND];
    else
        g_characterModels[index].shadowMatrix.m[14] = SWING_GROUND_NONE;
    while (g_shotParams[index][0] > SWING_HALF_TURN)
        g_shotParams[index][0] -= SWING_FULL_TURN;
    while (g_shotParams[index][0] < -SWING_HALF_TURN)
        g_shotParams[index][0] += SWING_FULL_TURN;
    float oldAngle = g_shotParams[index][0];
    float newSpeed = g_shotParams[index][2] + g_shotParams[index][1];
    g_shotParams[index][1] = newSpeed;
    float newAngle = newSpeed + oldAngle;
    g_shotParams[index][0] = newAngle;
    line.z = -(float)g_charState[index].values[SWING_VALUE_LENGTH];
    line.RotateY(newAngle / SWING_HALF_TURN * SWING_PI);
    line.RotateZ(g_shotParams[index][3] + SWING_HALF_PI);
    if ((Fabs(g_shotParams[index][2]) * 2.0f < g_shotParams[index][1] && g_charState[index].values[SWING_VALUE_MIRROR] == 0) ||
        (g_shotParams[index][1] < Fabs(g_shotParams[index][2]) * -2.0f && g_charState[index].values[SWING_VALUE_MIRROR] != 0))
    {
        float arm = (line - g_shotSlots[index].step).Length();
        SWING_ROLL += arm * SWING_ROLL_PER_UNIT;
    }
    else if ((Fabs(g_shotParams[index][2]) * 2.0f < g_shotParams[index][1] && g_charState[index].values[SWING_VALUE_MIRROR] != 0) ||
             (g_shotParams[index][1] < Fabs(g_shotParams[index][2]) * -2.0f && g_charState[index].values[SWING_VALUE_MIRROR] == 0))
    {
        float arm = (line - g_shotSlots[index].step).Length();
        SWING_ROLL -= arm * SWING_ROLL_PER_UNIT;
    }
    else
        g_charState[index].values[SWING_VALUE_STATUS] = SWING_RUNNING;
    while (SWING_ROLL > SWING_TWO_PI)
        SWING_ROLL -= SWING_TWO_PI;
    while (SWING_ROLL < -SWING_TWO_PI)
        SWING_ROLL += SWING_TWO_PI;
    g_charNodes[index][0]->matrix = RotationY(-SWING_ROLL) * TranslationMatrix(g_characterNodeMatrices[index][0].m[12],
                                                                              g_characterNodeMatrices[index][0].m[13],
                                                                              g_characterNodeMatrices[index][0].m[14]);
    g_shotSlots[index].step.Copy(line);
    if (((oldAngle < 0.0f && g_shotParams[index][0] > 0.0f) || (oldAngle > 0.0f && g_shotParams[index][0] < 0.0f) ||
         g_shotParams[index][0] == 0.0f || g_shotParams[index][0] == SWING_HALF_TURN ||
         g_shotParams[index][0] == -SWING_HALF_TURN) &&
        g_charState[index].values[SWING_VALUE_FLIP] != 0)
        g_shotParams[index][2] = g_shotParams[index][2] * -1.0f;
    if (g_enemiesFrozen == 0)
        g_characterModels[index].SetPosition(line.x + g_shotSlots[index].position.x, line.y + g_shotSlots[index].position.y,
                                        line.z + g_shotSlots[index].position.z);
    if (g_charHitLevel[index] != 0)
        g_charState[index].values[SWING_VALUE_STATUS] = SWING_LEFT;
    g_enemyTurnFlag[index] = 1;
    return 0;
}
