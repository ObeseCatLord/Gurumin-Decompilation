// A character that follows a map record's route point by point (the caller is 0x1002cc70): the points pack their
// coordinates like the step scripts of char_script.cpp.

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

// Point kinds (RoutePoint::z).
#define POINT_WALK 0          // goes on in the walking direction
#define POINT_FLAG_GATE 1     // code = script flag: goes on while it is set, else turns round (or waits)
#define POINT_HOLD_END 2      // code = 0 stops the route (ends it); else turns round
#define POINT_COUNTDOWN 3     // code = frames to wait here
#define POINT_TURN_FORWARD 4  // turns to walk forwards
#define POINT_TURN_BACK 5     // code = 0 turns to walk forwards, else turns round
#define POINT_REACH_MARGIN 0.1f   // added to the character's last step: how close counts as arrived
#define PI_APPROX 3.14159f        // pi as the game writes it
#define HALF_TURN_DEGREES 180.0f

// An absolute value on floats (an inline helper: the argument is rounded to float first).
inline float Fabs(float value)
{
    return (float)fabs(value);
}

// 0x10029730 (Route_StepPatrol). Moves character `slot` (standing on route point `*lastPoint`, heading for `point`)
// along its route, the map record AnimModel::bankIndex. A point packs x * 10000 + y and z * 10000 + extra (each
// part wrapped to -4999..5000), a kind and a code. When the character at `position` is within its last step plus
// a margin of the point (or forceStep is set) the point is taken: *lastPoint = point and the kind decides the next
// one (see POINT_*; the walking direction is CharState::walkingBack). The next point is read into *nextPoint, its
// position into `position`, its kind and code into *kind and *value, and *extra gets the previous point's extra.
// Crossing between the route's first two points sets the character's heading between headingFrom and headingTo
// (degrees) by its distance along the way. Returns 1 for a countdown or hold point that was reached, else 0.
extern "C" int FUN_10029730(int slot, int *lastPoint, int point, int *nextPoint, Vec4 *position, int *kind,
                            int *value, int *extra)
{
    float reach = DAT_10802bf8[slot].step.Length() + POINT_REACH_MARGIN;
    int record = DAT_10db6ee8[slot].bankIndex;
    int packedX = DAT_10210fc0[record].points[point].x / 10000;
    if (packedX > 5000)
        packedX -= 10000;
    float x = (float)packedX;
    int packedY = DAT_10210fc0[record].points[point].x % 10000;
    if (packedY > 5000)
        packedY -= 10000;
    float y = (float)packedY;
    int result = 0;
    int packedZ = DAT_10210fc0[record].points[point].y / 10000;
    if (packedZ > 5000)
        packedZ -= 10000;
    float z = (float)packedZ;
    int packedExtra = DAT_10210fc0[record].points[*lastPoint].y % 10000;
    if (packedExtra > 5000)
        packedExtra -= 10000;
    *extra = packedExtra;
    *kind = DAT_10210fc0[record].points[point].z;
    *value = DAT_10210fc0[record].points[point].code;
    if ((x - reach < position->x && position->x < x + reach && y - reach < position->y && position->y < y + reach &&
         z - reach < position->z && position->z < z + reach) ||
        DAT_1020b430[slot].forceStep != 0)
    {
        if (DAT_1020b430[slot].forceStep == 0)
        {
            DAT_10db6ee8[slot].matrix.m[12] = x;
            DAT_10db6ee8[slot].matrix.m[13] = y;
            DAT_10db6ee8[slot].matrix.m[14] = z;
        }
        *lastPoint = point;
        switch (*kind)
        {
        case POINT_TURN_BACK:
            if (*value == 0)
            {
                DAT_1020b430[slot].walkingBack = 0;
                point++;
            }
            else
            {
                DAT_1020b430[slot].walkingBack = 1;
                point--;
            }
            break;
        case POINT_FLAG_GATE:
        {
            int gate = DAT_10210fc0[record].points[point].y % 10000;
            if (gate > 5000)
                gate -= 10000;
            *extra = gate;
            if (gate != 0 && DAT_10874848.TestScriptFlag(gate) == 0)
            {
                if (DAT_1020b430[slot].walkingBack == 0)
                    point++;
                else
                    point--;
            }
            else
                point = *value;
            break;
        }
        case POINT_HOLD_END:
            result = 1;
            if (*value != 0)
            {
                DAT_1020b430[slot].walkingBack = 1;
                point--;
            }
            else
                point = 0;
            break;
        case POINT_COUNTDOWN:
            if (DAT_1020b430[slot].wait == 0)
                DAT_1020b430[slot].wait = *value;
            if (--DAT_1020b430[slot].wait >= 1)
                break;
            DAT_1020b430[slot].wait = 0;
        case POINT_WALK:
            if (DAT_1020b430[slot].walkingBack == 0)
                point++;
            else
                point--;
            break;
        case POINT_TURN_FORWARD:
            result = 1;
            DAT_1020b430[slot].walkingBack = 0;
            point++;
            break;
        }
        int nextX = DAT_10210fc0[record].points[point].x / 10000;
        if (nextX > 5000)
            nextX -= 10000;
        x = (float)nextX;
        int nextY = DAT_10210fc0[record].points[point].x % 10000;
        if (nextY > 5000)
            nextY -= 10000;
        y = (float)nextY;
        int nextZ = DAT_10210fc0[record].points[point].y / 10000;
        if (nextZ > 5000)
            nextZ -= 10000;
        z = (float)nextZ;
        int nextExtra = DAT_10210fc0[record].points[point].y % 10000;
        if (nextExtra > 5000)
            nextExtra -= 10000;
        *extra = nextExtra;
        *kind = DAT_10210fc0[record].points[point].z;
        *value = DAT_10210fc0[record].points[point].code;
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
        float travelled = (first - *position).Length();
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
    position->x = x;
    position->y = y;
    position->z = z;
    int forced = DAT_1020b430[slot].forceStep;
    if (forced != 0)
    {
        DAT_1020b430[slot].forceStep = 0;
        return 0;
    }
    return result;
}
