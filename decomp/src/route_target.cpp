// The route target of a character walking its map record's route (caller Char_WalkRoute, route_walk.cpp): the same
// packed route points as route_step.cpp and spirit_route.cpp, with relative point kinds and the "behaviour 2" mode.

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

// Point kinds (RoutePoint::z).
#define POINT_WALK 0               // goes on in the walking direction
#define POINT_FLAG_GATE 1          // extra = script flag: goes on while it is clear, else jumps to the code's point
#define POINT_HOLD_END 2           // code = 0 stops the route (ends it); else turns round
#define POINT_COUNTDOWN 3          // code = frames to wait here
#define POINT_TURN_FORWARD 4       // turns to walk forwards
#define POINT_TURN_BACK 5          // code = 0 turns to walk forwards, else turns round
#define POINT_RELATIVE 6           // coordinates are relative: negative ones are taken modulo 10000
#define POINT_RELATIVE_KEEP_X 7    // as 6 but x stays as it is
#define POINT_WAIT_FLAG 8          // code = script flag: turns to walk forwards once it is set, else waits here
#define POINT_RELATIVE_NO_Y 9      // relative x and z only
#define POINT_WIDE_RELATIVE 10     // x / 100000 and y % 100000 (y unwrapped), relative like 6
#define POINT_MODE_FOLLOW 2        // CharState::behaviour: the point's extra names the point to head for
#define POINT_REACH_MARGIN 0.1f    // added to the character's last step: how close counts as arrived
#define POINT_RELATIVE_WRAP 10000.0f
#define PI_APPROX 3.14159f         // pi as the game writes it
#define HALF_TURN_DEGREES 180.0f

// An absolute value on floats (an inline helper: the argument is rounded to float first).
inline float Fabs(float value)
{
    return (float)fabs(value);
}

// 0x100286d0 (Char_StepRouteTarget). Finds the point character `slot` (standing on route point *lastPoint, heading
// for `point`) walks to along its route, the map record AnimModel::bankIndex, like Route_StepPatrol and
// Spirit_StepRoute: a point packs x * 10000 + y and z * 10000 + extra (each part wrapped to -4999..5000), a kind and
// a code (see POINT_*; relative kinds are rewritten into plain ones by the code). In follow mode (behaviour 2) the
// point named by the previous point's extra is headed for instead. When the character at `target` is inside the
// box of its last step plus a margin around the point, it is put there, *lastPoint = point and the kind decides the
// next point (the walking direction is CharState::state). The next point is read into *nextPoint, its position into
// `target`, its kind and code into *kind and *value and its extra into *extra. Crossing between the route's first
// two points sets the heading between headingFrom and headingTo (degrees) by the distance along the way. Returns 1
// when a hold, turn or wait point was reached, else 0.
extern "C" int FUN_100286d0(int slot, int *lastPoint, int point, int *nextPoint, Vec4 *target, int *kind,
                            int *value, int *extra)
{
    float reach = DAT_10802bf8[slot].step.Length() + POINT_REACH_MARGIN;
    int record = DAT_10db6ee8[slot].bankIndex;
    int rowBase = record * MAP_RECORD_ROWS;
    int packedExtra = DAT_10210fc0[0].points[rowBase + *lastPoint].y % 10000;
    int result = 0;
    if (packedExtra > 5000)
        packedExtra -= 10000;
    *extra = packedExtra;
    int lastKind = DAT_10210fc0[0].points[rowBase + *lastPoint].z;
    int row = rowBase + point;
    *kind = DAT_10210fc0[0].points[row].z;
    int code = DAT_10210fc0[0].points[row].code;
    *value = code;
    int packedX;
    int packedY;
    if (*kind == POINT_WIDE_RELATIVE)
    {
        packedX = DAT_10210fc0[0].points[row].x / 100000;
        packedY = DAT_10210fc0[0].points[row].x % 100000;
        if (packedX > 5000)
            packedX -= 10000;
    }
    else
    {
        packedX = DAT_10210fc0[0].points[row].x / 10000;
        packedY = DAT_10210fc0[0].points[row].x % 10000;
        if (packedX > 5000)
            packedX -= 10000;
        if (packedY > 5000)
            packedY -= 10000;
    }
    float x = (float)packedX;
    float y = (float)packedY;
    int packedZ = DAT_10210fc0[0].points[row].y / 10000;
    if (packedZ > 5000)
        packedZ -= 10000;
    float z = (float)packedZ;
    if (*kind == POINT_RELATIVE || *kind == POINT_RELATIVE_KEEP_X || *kind == POINT_RELATIVE_NO_Y ||
        *kind == POINT_WIDE_RELATIVE)
    {
        if (*kind == POINT_RELATIVE_NO_Y)
        {
            if (x < 0.0f)
                x += POINT_RELATIVE_WRAP;
            if (z < 0.0f)
                z += POINT_RELATIVE_WRAP;
        }
        else
        {
            if (x < 0.0f && *kind == POINT_RELATIVE)
                x += POINT_RELATIVE_WRAP;
            if (y < 0.0f)
                y += POINT_RELATIVE_WRAP;
            if (z < 0.0f && *kind == POINT_RELATIVE)
                z += POINT_RELATIVE_WRAP;
        }
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
    if (DAT_1020b430[slot].behaviour == POINT_MODE_FOLLOW && point != *extra - 1 && lastKind != POINT_FLAG_GATE)
    {
        point = *extra - 1;
        row = rowBase + point;
        int jumpX = DAT_10210fc0[0].points[row].x / 10000;
        int jumpY = DAT_10210fc0[0].points[row].x % 10000;
        if (jumpX > 5000)
            jumpX -= 10000;
        x = (float)jumpX;
        if (jumpY > 5000)
            jumpY -= 10000;
        y = (float)jumpY;
        int jumpZ = DAT_10210fc0[0].points[row].y / 10000;
        if (jumpZ > 5000)
            jumpZ -= 10000;
        z = (float)jumpZ;
        *kind = DAT_10210fc0[0].points[row].z;
        *value = DAT_10210fc0[0].points[row].code;
    }
    if (x - reach < target->x && target->x < x + reach && y - reach < target->y && target->y < y + reach &&
        z - reach < target->z && target->z < z + reach)
    {
        DAT_10db6ee8[slot].matrix._41 = x;
        DAT_10db6ee8[slot].matrix._42 = y;
        DAT_10db6ee8[slot].matrix._43 = z;
        *lastPoint = point;
        if (DAT_1020b430[slot].behaviour == POINT_MODE_FOLLOW && *kind != POINT_HOLD_END &&
            *kind != POINT_TURN_FORWARD && *kind != POINT_FLAG_GATE)
        {
            *extra = FUN_100286b0(DAT_10210fc0[0].points[rowBase + point].y % 10000);
            point = *extra - 1;
        }
        else
        {
            switch (*kind)
            {
            case POINT_TURN_BACK:
                if (*value == 0)
                    goto turnForward;
                DAT_1020b430[slot].state = 1;
                point--;
                break;
            case POINT_FLAG_GATE:
                *extra = FUN_100286b0(DAT_10210fc0[0].points[row].y % 10000);
                if (*extra != 0 && DAT_10874848.TestScriptFlag(*extra) == 0)
                {
                    if (DAT_1020b430[slot].state != 0)
                        point--;
                    else
                        point++;
                }
                else
                    point = *value;
                break;
            case POINT_HOLD_END:
                result = 1;
                if (*value == 0)
                    point = 0;
                else
                {
                    DAT_1020b430[slot].state = 1;
                    point--;
                }
                break;
            case POINT_COUNTDOWN:
                if (DAT_1020b430[slot].wait == 0)
                    DAT_1020b430[slot].wait = *value;
                if (--DAT_1020b430[slot].wait >= 1)
                    break;
                DAT_1020b430[slot].wait = 0;
            case POINT_WALK:
                if (DAT_1020b430[slot].state == 0)
                    point++;
                else
                    point--;
                break;
            case POINT_TURN_FORWARD:
                result = 1;
                goto turnForward;
            case POINT_WAIT_FLAG:
                result = 1;
                if (DAT_10874848.TestScriptFlag(*value) != 0)
                {
                turnForward:
                    DAT_1020b430[slot].state = 0;
                    point++;
                }
                break;
            }
        }
        row = rowBase + point;
        float nextX = (float)FUN_100286b0(DAT_10210fc0[0].points[row].x / 10000);
        float nextY = (float)FUN_100286b0(DAT_10210fc0[0].points[row].x % 10000);
        float nextZ = (float)FUN_100286b0(DAT_10210fc0[0].points[row].y / 10000);
        *extra = FUN_100286b0(DAT_10210fc0[0].points[row].y % 10000);
        x = nextX;
        y = nextY;
        z = nextZ;
        *kind = DAT_10210fc0[0].points[row].z;
        *value = DAT_10210fc0[0].points[row].code;
    }
    if ((*lastPoint == 0 && point == 1) || (*lastPoint == 1 && point == 0))
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
            float from = ((float)DAT_1020b430[slot].headingFrom * PI_APPROX) / HALF_TURN_DEGREES;
            float to = ((float)DAT_1020b430[slot].headingTo * PI_APPROX) / HALF_TURN_DEGREES;
            float difference = Fabs(from - to);
            float turn = (travelled / total) * difference;
            if (from > to)
                from = from - turn;
            else
                from = turn + from;
            DAT_10db6ee8[slot].targetHeading = from;
        }
    }
    *nextPoint = point;
    target->x = x;
    target->y = y;
    target->z = z;
    DAT_1020b430[slot].forceStep = 0;
    return result;
}
