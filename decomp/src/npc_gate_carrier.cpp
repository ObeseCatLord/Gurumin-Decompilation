// A character that flies along a gated map route (0x1007b870, behaviour 0xd of Char_RunBehaviour 0x100dfcf0): at a
// pick-up gate it spawns an enemy (kind 0x21) and carries it under its head until a drop-off gate, where the
// enemy is let go. When the carrier is defeated its passenger dies.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "char_state.h"

#include "char_slot.h"

#include "char_tables.h"
#include "route_point.h"

#include "map_records.h"

#include "world.h"

#include "route_gates.h"
#include "char_spawn.h"

#define CARRIER_PICKUP_KIND 0x21
#define CARRIER_GATE_DROP 2
#define CARRIER_GATE_PICKUP 4
#define CARRIER_MAX_HEIGHT 2200.0f
#define CARRIER_DROP_SOUND 0x2ce
#define CARRIER_FAR_RANGE 500.0f
#define CARRIER_TURN_UNIT 1.0472f
#define CARRIER_TURN_PER_PACE 0.174533f
#define CARRIER_PACE_LIMIT 12.0f
#define CARRIER_PACE_MIN 1.0f
#define CARRIER_PACE_GROW 0.2f
#define CARRIER_PACE_DISTANCE 16.0f
#define CARRIER_PASSENGER_DROP 150.0f
#define CARRIER_HEADING_BIAS 0.785375f
#define CARRIER_PI 3.1415f
#define CARRIER_TWO_PI 6.28319f
#define CARRIER_PICKUP_PUSH_MASK 0x400
#define CARRIER_PICKUP_FLAG_MASK 0x200
#define CARRIER_PICKUP_COLOUR 0x21

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound

// 0x1007b870 (Npc_GatedRouteCarrier). One frame of carrier `index`. When it is defeated its passenger loses its
// health. On the first frame it picks its first route point (point 0 when the record's first code is set, else 1),
// scales its pace to its height, and is put on the route. Every frame it follows the gated route (the route number
// is the record's first point's y): it turns up to a limit per frame towards the next point (the limit grows when it
// is near), moves with a pace that eases with the distance, keeps its passenger under its head and plays its idle
// motion. On reaching a drop-off gate with a passenger the passenger is released (behaviour 1, sound 0x2ce); at a
// pick-up gate, unless the player is outside the height band, it spawns the passenger. Returns 0.
extern "C" int FUN_1007b870(int index)
{
    Vec4 position;
    Vec4 aim;
    int nextPoint = 0;
    int type = 0;
    int param = 0;
    int value = 0;
    if (DAT_10210d30[index] != 0)
    {
        if (DAT_1020b430[index].passenger != 0)
            DAT_10daaae8[DAT_1020b430[index].passenger].stats[STAT_HEALTH] = 0;
        return 0;
    }
    float height;
    if (DAT_1020b430[index].passenger < 1 &&
        ((height = DAT_10db6ee8[index].matrix.m[14] - DAT_10db6ee8[0].shadowMatrix.m[14]),
         height < (float)DAT_1020b430[index].pickupHeight || height > CARRIER_MAX_HEIGHT))
        DAT_1020b430[index].pickupBlocked = 1;
    else
        DAT_1020b430[index].pickupBlocked = 0;
    if (DAT_1020b430[index].gateStarted != 2)
    {
        if (DAT_10210fc0[DAT_10daaae8[index].routeRecord].points[0].code != 0)
        {
            DAT_1020b430[index].gatePoint = 0;
            DAT_1020b430[index].gatePrevious = 1;
        }
        else
        {
            DAT_1020b430[index].gatePoint = 1;
            DAT_1020b430[index].gatePrevious = 0;
        }
        DAT_1020b430[index].gateStarted = 2;
        DAT_1020b430[index].passenger = 0;
        DAT_1020b430[index].pickupBlocked = 0;
        DAT_10255a88[index] = 0;
        DAT_10db6ee8[index].m_0267 = 1;
        DAT_10255160[index][2] = DAT_10db6ee8[index].matrix.m[14] * 0.01f;
        DAT_10db6ee8[index].m_17e8 = 1;
        DAT_1020b430[index].pickupHeight = (int)DAT_10db6ee8[index].matrix.m[13];
        position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        int route = DAT_10210fc0[DAT_10daaae8[index].routeRecord].points[0].y;
        if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
            DAT_10db6ee8[index].mode = 0;
        FUN_1002f050(index, route, DAT_1020b430[index].gatePoint, &nextPoint, &position.x, &type, &param, &value);
        DAT_10db6ee8[index].matrix.m[12] = position.x;
        DAT_10db6ee8[index].matrix.m[13] = position.y;
        DAT_10db6ee8[index].matrix.m[14] = position.z;
        FUN_1002f050(index, route, DAT_1020b430[index].gatePoint, &nextPoint, &position.x, &type, &param, &value);
        DAT_10db6ee8[index].heading = Atan2(position.x - DAT_10db6ee8[index].matrix.m[12],
                                            position.y - DAT_10db6ee8[index].matrix.m[13]) +
                                      CARRIER_HEADING_BIAS;
        if (DAT_10db6ee8[index].root != 0)
            DAT_10db6ee8[index].root->M_101946b0(0, 0xffffffff);
        DAT_10daaae8[index].stats[2] = -1;
        DAT_10255f10[index] = -1;
    }
    DAT_10db6ee8[index].m_238c = 0;
    position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
        DAT_10db6ee8[index].mode = 0;
    FUN_1002f050(index, DAT_10210fc0[DAT_10daaae8[index].routeRecord].points[0].y, DAT_1020b430[index].gatePoint,
                 &nextPoint, &position.x, &type, &param, &value);
    aim.Copy(position - DAT_10db6ee8[index].matrix.GetTranslation());
    float rise = aim.z;
    float distance = aim.Length2D();
    if (aim.x != 0.0f || aim.y != 0.0f || aim.z != 0.0f)
        aim.Normalized3();
    float maxTurn;
    if (distance > CARRIER_FAR_RANGE)
        maxTurn = DAT_10255160[index][2] * CARRIER_TURN_PER_PACE;
    else
    {
        maxTurn = (CARRIER_TURN_UNIT / distance) * 0.01f;
        if (maxTurn > CARRIER_TURN_UNIT)
            maxTurn = CARRIER_TURN_UNIT;
        maxTurn = DAT_10255160[index][2] * CARRIER_TURN_PER_PACE + maxTurn;
    }
    float turn = Atan2(aim.x, aim.y) + CARRIER_PI;
    turn = turn - DAT_10db6ee8[index].heading;
    if (turn > CARRIER_PI)
        turn = -(CARRIER_TWO_PI - turn);
    else if (turn < -CARRIER_PI)
        turn = turn + CARRIER_TWO_PI;
    if ((float)fabs(turn) < maxTurn)
        DAT_10db6ee8[index].targetHeading = turn + DAT_10db6ee8[index].heading;
    else if (turn > 0.0f)
        DAT_10db6ee8[index].targetHeading = maxTurn + DAT_10db6ee8[index].heading;
    else
        DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading - maxTurn;
    if (distance > CARRIER_FAR_RANGE)
    {
        float pace = DAT_10255160[index][2] * CARRIER_PACE_GROW + DAT_10255160[index][0];
        DAT_10255160[index][0] = pace;
        if (pace > CARRIER_PACE_LIMIT)
            DAT_10255160[index][0] = CARRIER_PACE_LIMIT;
    }
    else if (distance < CARRIER_FAR_RANGE)
    {
        float pace = distance / (CARRIER_PACE_DISTANCE / DAT_10255160[index][2]);
        DAT_10255160[index][0] = pace;
        if (pace > CARRIER_PACE_LIMIT)
            DAT_10255160[index][0] = CARRIER_PACE_LIMIT;
        if (DAT_10255160[index][0] < CARRIER_PACE_MIN)
            DAT_10255160[index][0] = CARRIER_PACE_MIN;
    }
    aim.x = 0.0f;
    aim.y = DAT_10255160[index][0] * -1.0f;
    aim.RotateZ(-DAT_10db6ee8[index].heading);
    float x = aim.x + DAT_10db6ee8[index].matrix.m[12];
    float y = aim.y + DAT_10db6ee8[index].matrix.m[13];
    DAT_10db6ee8[index].matrix.m[12] = x;
    DAT_10db6ee8[index].matrix.m[13] = y;
    float z = (rise / distance) * DAT_10255160[index][0] + DAT_10db6ee8[index].matrix.m[14];
    DAT_10db6ee8[index].matrix.m[14] = z;
    DAT_10db6ee8[index].SetPosition(x, y, z);
    if (DAT_1020b430[index].passenger > 0)
        DAT_10db6ee8[DAT_1020b430[index].passenger].SetPosition(
            DAT_10db6ee8[index].headNode->worldMatrix.m[12], DAT_10db6ee8[index].headNode->worldMatrix.m[13],
            DAT_10db6ee8[index].headNode->worldMatrix.m[14] - CARRIER_PASSENGER_DROP);
    if (DAT_102550e0[index] == 0)
        DAT_10db6ee8[index].PlayMotion(0, 0, 5, 1.0f);
    if (DAT_1020b430[index].gatePoint != nextPoint)
    {
        DAT_1020b430[index].gatePrevious = DAT_1020b430[index].gatePoint;
        int carried = DAT_1020b430[index].passenger;
        DAT_1020b430[index].gatePoint = nextPoint;
        if (carried > 0 && type == CARRIER_GATE_DROP)
        {
            DAT_1020b430[DAT_1020b430[index].passenger].behaviour = 1;
            DAT_1020b430[DAT_1020b430[index].passenger].talk = DAT_10210fc0[DAT_10daaae8[index].routeRecord].points[0].z;
            DAT_1020b430[index].passenger = 0;
            FUN_10065b10(index, CARRIER_DROP_SOUND, 0, 0);
            return 0;
        }
        if (DAT_1020b430[index].pickupBlocked == 0 && type == CARRIER_GATE_PICKUP)
        {
            DAT_1020b430[index].passenger =
                FUN_10050510(DAT_10db6ee8[index].matrix.m[12], DAT_10db6ee8[index].matrix.m[13],
                             DAT_10db6ee8[index].matrix.m[14], CARRIER_PICKUP_KIND, 0);
            if (DAT_1020b430[index].passenger == -1)
            {
                DAT_1020b430[index].passenger = 0;
                return 0;
            }
            DAT_10db6ee8[DAT_1020b430[index].passenger].root->M_10194730(CARRIER_PICKUP_COLOUR,
                                                                         CARRIER_PICKUP_PUSH_MASK);
            DAT_10db6ee8[DAT_1020b430[index].passenger].root->M_10194730(CARRIER_PICKUP_COLOUR,
                                                                         CARRIER_PICKUP_FLAG_MASK);
            DAT_1020b430[DAT_1020b430[index].passenger].behaviour = 0;
            DAT_10db6ee8[DAT_1020b430[index].passenger].shadowHidden = 0;
            DAT_10daaae8[DAT_1020b430[index].passenger].money = 0;
        }
    }
    return 0;
}
