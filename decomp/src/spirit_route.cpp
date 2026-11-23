// The route step of the spirit character (caller Char_SpiritUpdate, char_spirit.cpp): the same packed route points as
// route_step.cpp (Route_StepPatrol) and char_script.cpp, with a few more point kinds.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "last_position.h"

#include "char_state.h"

#include "char_vars.h"
#include "map_vars.h"
#include "map_records.h"
#include "game.h"

#include "char_script.h"

// Point kinds (RoutePoint::z), as in Route_StepPatrol.
#define POINT_WALK 0          // goes on in the walking direction
#define POINT_FLAG_GATE 1     // extra = script flag: goes on while it is clear, else jumps to the code's point
#define POINT_HOLD_END 2      // code = 0 stops the route (ends it); else turns round
#define POINT_COUNTDOWN 3     // code = frames to wait here
#define POINT_TURN_FORWARD 4  // turns to walk forwards
#define POINT_TURN_BACK 5     // code = 0 turns to walk forwards, else turns round
#define POINT_RELATIVE 6      // coordinates are relative: negative ones are taken modulo 10000 (kind 7 keeps x)
#define POINT_RELATIVE_KEEP_X 7
#define POINT_REACH_MARGIN 0.1f   // added to the character's last step: how close counts as arrived
#define POINT_RELATIVE_WRAP 10000.0f
#define PI_APPROX 3.14159f        // pi as the game writes it
#define HALF_TURN_DEGREES 180.0f

// Route modes (CharState::routeMode) that put the character on a fixed point.
#define ROUTE_MODE_START_HOLD 1
#define ROUTE_MODE_FIRST 2
#define ROUTE_MODE_RESET_WAIT 3

// An absolute value on floats (an inline helper: the argument is rounded to float first).
inline float Fabs(float value)
{
    return (float)fabs(value);
}

// 0x10028fa0 (Spirit_StepRoute). Moves the spirit `index` (standing on route point *lastPoint, heading for `point`)
// along its route, the map record AnimModel::bankIndex, like Route_StepPatrol: a point packs x * 10000 + y and
// z * 10000 + extra (each part wrapped to -4999..5000), a kind and a code. The route mode (1 = hold at the start,
// 2 = go to the first point, 3 = wait there) overrides the point headed for. When the spirit at `target` is within
// its last step plus a margin of the point's height the point is taken: *lastPoint = point and its kind decides the
// next one (see POINT_*; the walking direction is CharState::state). The next point is read into *nextPoint, its
// position into `target`, its kind and code into *kind and *value and its extra into *extra. Crossing between the
// route's first two points sets the heading between headingFrom and headingTo (degrees) by the distance along the
// way. Returns 1 when a hold or turn point was reached, else 0.
extern "C" int FUN_10028fa0(int index, int *lastPoint, int point, int *nextPoint, Vec4 *target, int *kind,
                            int *value, int *extra)
{
    float reach = DAT_10802bf8[index].step.Length() + POINT_REACH_MARGIN;
    int mode = DAT_1020b430[index].routeMode;
    int record = DAT_10db6ee8[index].bankIndex;
    int result = 0;
    int current = point;
    if (mode == ROUTE_MODE_START_HOLD)
    {
        DAT_1020b430[index].wait = 0;
        current = ROUTE_MODE_FIRST;
    }
    else if (mode == ROUTE_MODE_FIRST)
    {
        DAT_1020b430[index].wait = 0;
        current = 0;
    }
    else
    {
        if (mode == ROUTE_MODE_RESET_WAIT)
        {
            DAT_1020b430[index].wait = 0;
            if (point != ROUTE_MODE_FIRST)
                current = ROUTE_MODE_FIRST;
        }
    }
    int rowBase = record * MAP_RECORD_ROWS;
    int packedX = DAT_10210fc0[0].points[rowBase + current].x / 10000;
    if (packedX > 5000)
        packedX -= 10000;
    float x = (float)packedX;
    int packedY = DAT_10210fc0[0].points[rowBase + current].x % 10000;
    if (packedY > 5000)
        packedY -= 10000;
    float y = (float)packedY;
    int packedZ = DAT_10210fc0[0].points[rowBase + current].y / 10000;
    if (packedZ > 5000)
        packedZ -= 10000;
    float z = (float)packedZ;
    int packedExtra = DAT_10210fc0[0].points[rowBase + *lastPoint].y % 10000;
    if (packedExtra > 5000)
        packedExtra -= 10000;
    *extra = packedExtra;
    int lastKind = DAT_10210fc0[0].points[rowBase + *lastPoint].z;
    *kind = DAT_10210fc0[0].points[rowBase + current].z;
    int code = DAT_10210fc0[0].points[rowBase + current].code;
    *value = code;
    if (*kind == POINT_RELATIVE || *kind == POINT_RELATIVE_KEEP_X)
    {
        if (x < 0.0f && *kind == POINT_RELATIVE)
            x += POINT_RELATIVE_WRAP;
        if (y < 0.0f)
            y += POINT_RELATIVE_WRAP;
        if (z < 0.0f)
            z += POINT_RELATIVE_WRAP;
        if (code == 0 || code == 1)
        {
            *kind = POINT_WALK;
            *value = 0;
        }
        else if (code == 2)
        {
            *kind = POINT_HOLD_END;
            *value = 1;
        }
        else if (code == 3)
        {
            *kind = POINT_TURN_FORWARD;
            *value = 0;
        }
        else if (code == 4)
        {
            *kind = POINT_HOLD_END;
            *value = 0;
        }
        else if (code > 4)
            *kind = POINT_COUNTDOWN;
    }
    if (DAT_1020b430[index].behaviour == 2 && current != *extra - 1 && lastKind != POINT_FLAG_GATE)
    {
        current = *extra - 1;
        int jumpX = DAT_10210fc0[0].points[rowBase + current].x / 10000;
        int jumpY = DAT_10210fc0[0].points[rowBase + current].x % 10000;
        if (jumpX > 5000)
            jumpX -= 10000;
        x = (float)jumpX;
        if (jumpY > 5000)
            jumpY -= 10000;
        y = (float)jumpY;
        int jumpZ = DAT_10210fc0[0].points[rowBase + current].y / 10000;
        if (jumpZ > 5000)
            jumpZ -= 10000;
        z = (float)jumpZ;
        *kind = DAT_10210fc0[0].points[rowBase + current].z;
        *value = DAT_10210fc0[0].points[rowBase + current].code;
    }
    if (z - reach < target->z && target->z < z + reach)
    {
        DAT_10db6ee8[index].matrix._43 = z;
        *lastPoint = current;
        if (DAT_1020b430[index].behaviour == 2 && *kind != POINT_HOLD_END && *kind != POINT_TURN_FORWARD &&
            *kind != POINT_FLAG_GATE)
        {
            *extra = FUN_100286b0(DAT_10210fc0[0].points[rowBase + current].y % 10000);
            current = *extra - 1;
        }
        else
        {
            switch (*kind)
            {
            case POINT_TURN_BACK:
                if (*value == 0)
                    goto turnForward;
                DAT_1020b430[index].state = 1;
                current--;
                break;
            case POINT_FLAG_GATE:
                *extra = FUN_100286b0(DAT_10210fc0[0].points[rowBase + current].y % 10000);
                if (*extra != 0 && DAT_10874848.TestScriptFlag(*extra) == 0)
                {
                    if (DAT_1020b430[index].state != 0)
                        current--;
                    else
                        current++;
                }
                else
                    current = *value;
                break;
            case POINT_HOLD_END:
                result = 1;
                if (*value == 0)
                    current = 0;
                else
                {
                    DAT_1020b430[index].state = 1;
                    current--;
                }
                break;
            case POINT_COUNTDOWN:
                if (DAT_1020b430[index].wait == 0)
                    DAT_1020b430[index].wait = *value;
                if (--DAT_1020b430[index].wait >= 1)
                    break;
                DAT_1020b430[index].wait = 0;
            case POINT_WALK:
                if (DAT_1020b430[index].state == 0)
                    current++;
                else
                    current--;
                break;
            case POINT_TURN_FORWARD:
                result = 1;
            turnForward:
                DAT_1020b430[index].state = 0;
                current++;
                break;
            }
        }
        int nextX = DAT_10210fc0[0].points[rowBase + current].x / 10000;
        int nextY = DAT_10210fc0[0].points[rowBase + current].x % 10000;
        if (nextX > 5000)
            nextX -= 10000;
        x = (float)nextX;
        if (nextY > 5000)
            nextY -= 10000;
        y = (float)nextY;
        int nextZ = DAT_10210fc0[0].points[rowBase + current].y / 10000;
        int nextExtra = DAT_10210fc0[0].points[rowBase + current].y % 10000;
        if (nextZ > 5000)
            nextZ -= 10000;
        z = (float)nextZ;
        if (nextExtra > 5000)
            nextExtra -= 10000;
        *extra = nextExtra;
        *kind = DAT_10210fc0[0].points[rowBase + current].z;
        *value = DAT_10210fc0[0].points[rowBase + current].code;
    }
    if ((*lastPoint == 0 && current == 1) || (*lastPoint == 1 && current == 0))
    {
        int part;
        Vec4 first;
        Vec4 second;
        part = DAT_10210fc0[record].points[0].x / 10000;
        if (part > 5000)
            part -= 10000;
        first.x = (float)part;
        part = DAT_10210fc0[record].points[0].x % 10000;
        if (part > 5000)
            part -= 10000;
        first.y = (float)part;
        part = DAT_10210fc0[record].points[0].y / 10000;
        if (part > 5000)
            part -= 10000;
        first.z = (float)part;
        part = DAT_10210fc0[record].points[1].x / 10000;
        if (part > 5000)
            part -= 10000;
        second.x = (float)part;
        part = DAT_10210fc0[record].points[1].x % 10000;
        if (part > 5000)
            part -= 10000;
        second.y = (float)part;
        part = DAT_10210fc0[record].points[1].y / 10000;
        if (part > 5000)
            part -= 10000;
        second.z = (float)part;
        float total = (first - second).Length();
        float travelled = (first - *target).Length();
        if (total != 0.0f)
        {
            float from = ((float)DAT_1020b430[index].headingFrom * PI_APPROX) / HALF_TURN_DEGREES;
            float to = ((float)DAT_1020b430[index].headingTo * PI_APPROX) / HALF_TURN_DEGREES;
            float difference = Fabs(from - to);
            float turn = (travelled / total) * difference;
            if (from > to)
                from = from - turn;
            else
                from = turn + from;
            DAT_10db6ee8[index].targetHeading = from;
        }
    }
    *nextPoint = current;
    target->x = x;
    target->y = y;
    target->z = z;
    DAT_1020b430[index].forceStep = 0;
    return result;
}
