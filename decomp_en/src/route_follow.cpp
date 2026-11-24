// Route walking (EN build). Ported from the matched JP source decomp/src/route_follow.cpp.
#include "char.h"
#include "map_records.h"

// The action of a route point (its code digits above the low four, modulo ROUTE_ACTION_COUNT).
const int ROUTE_ACTION_ADVANCE = 0;   // go on to the next point in the walker's direction
const int ROUTE_ACTION_FORWARDS = 1;  // go on forwards (the walker walks forwards from now on)
const int ROUTE_ACTION_RESTART = 2;   // back to point 1 (the walker walks forwards from now on)
const int ROUTE_ACTION_TURN = 3;      // turn round (the walker walks backwards from now on)
const int ROUTE_ACTION_COUNT = 4;
// Digits of the code that hold the value and those above it (the action and group).
const int ROUTE_CODE_DIVISOR = 10000;
// The point route walkers restart from.
const int ROUTE_FIRST_POINT = 1;

// Route_Follow: reads point `point` of route `route` for walker `walker`: stores its action (low two bits), value and
// group in *outAction, *outValue and *outGroup. When `position` lies within `radius` of the point on all three axes the
// walker moves on: action 0 goes to the next point in its direction, 1 forwards, 2 back to point 1 and 3 turns it round
// (1 and 2 also set it walking forwards). Stores the point it heads for in *outPoint and that point's coordinates in
// `position`. Returns 1-3 for the actions 1-3 when the point was reached, else 0. Called by the walking behaviours.
extern "C" int Route_Follow(int walker, int route, int point, int *outPoint, float *position, int *outAction,
                            int *outValue, int *outGroup, int radius)
{
    float x = (float)g_mapRecords[route].points[point].x;
    float y = (float)g_mapRecords[route].points[point].y;
    float z = (float)g_mapRecords[route].points[point].z;
    int action = g_mapRecords[route].points[point].code / ROUTE_CODE_DIVISOR;
    int result = 0;
    if (action > 5000)
        action -= 10000;
    *outAction = action;
    int value = g_mapRecords[route].points[point].code % ROUTE_CODE_DIVISOR;
    if (value > 5000)
        value -= 10000;
    *outValue = value;
    *outGroup = *outAction / ROUTE_ACTION_COUNT;
    *outAction = *outAction % ROUTE_ACTION_COUNT;
    float range = (float)radius;
    int next = point;
    if (x - range < position[0] && range + x > position[0] && y - range < position[1] && range + y > position[1] &&
        z - range < position[2] && range + z > position[2])
    {
        switch (*outAction)
        {
        case ROUTE_ACTION_ADVANCE:
            if (g_charState[walker].values[CHARSTATE_ROUTE_REVERSED] != 0)
                next = point - 1;
            else
                next = point + 1;
            break;
        case ROUTE_ACTION_FORWARDS:
            result = 1;
            g_charState[walker].values[CHARSTATE_ROUTE_REVERSED] = 0;
            next = point + 1;
            break;
        case ROUTE_ACTION_RESTART:
            result = 2;
            g_charState[walker].values[CHARSTATE_ROUTE_REVERSED] = 0;
            next = ROUTE_FIRST_POINT;
            break;
        case ROUTE_ACTION_TURN:
            result = 3;
            g_charState[walker].values[CHARSTATE_ROUTE_REVERSED] = 1;
            next = point - 1;
            break;
        }
        x = (float)g_mapRecords[route].points[next].x;
        y = (float)g_mapRecords[route].points[next].y;
        z = (float)g_mapRecords[route].points[next].z;
    }
    *outPoint = next;
    position[0] = x;
    position[1] = y;
    position[2] = z;
    return result;
}
