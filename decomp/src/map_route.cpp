// Route lengths: each entry of the map table at 0x10210fc0 (see map_icons.cpp) holds a route of up to 15 points.

#include "vec4.h"

#include "route_point.h"


// A signed four-digit field: values above 5000 count down from 10000.
inline int SignedField(int value)
{
    if (value > 5000)
        value -= 10000;
    return value;
}

#include "map_records.h"

// 0x10049990 (Map_RouteLength). Returns the length of the route of map table entry `map`, walked until a point ends
// it and back again, plus each point's extra cost times `costScale`. Point i is stored at entries i and 2 * i of one
// array and the segments are measured between the 2 * i entries. Because entry 2 is overwritten by the second point
// in the same step, the first segment measures 0 and the route closes at the second point, not the first.
extern "C" float FUN_10049990(int map, int costScale)
{
    float total = 0;
    Vec4 points[32];
    int count = 0;
    for (int i = 1; i < 16; i++)
    {
        points[i].x = points[2 * i].x = (float)DAT_10210fc0[map].entry.route[i - 1].x;
        points[i].y = points[2 * i].y = (float)DAT_10210fc0[map].entry.route[i - 1].y;
        points[i].z = points[2 * i].z = (float)DAT_10210fc0[map].entry.route[i - 1].z;
        if (i > 1)
            total += (points[2 * i] - points[2 * (i - 1)]).Length();
        if (SignedField(DAT_10210fc0[map].entry.route[i - 1].code % 10000) != 0)
            total += (float)(SignedField(DAT_10210fc0[map].entry.route[i - 1].code % 10000) * costScale);
        count++;
        if (SignedField(DAT_10210fc0[map].entry.route[i - 1].code / 10000) % 4 > 1)
            break;
    }
    total += (points[2 * count] - points[2]).Length();
    return total;
}

// Two marks per route point (after its segment, after its cost), numbered from 1.
#define ROUTE_MAX_MARKS 32

// 0x10049b00 (Map_RoutePosition). Walks the same route as Map_RouteLength and writes into `out` the point that lies
// at fraction `t` of the way along it, measured from the point before `part` (so `t` is relative to that point; it
// wraps into [0, 1)). `marks` holds the running length after each segment and after each point's extra cost,
// normalised to 0..1; the point is interpolated inside the interval that contains the fraction, the last interval
// running back to the first point. Returns 0.
extern "C" int FUN_10049b00(int map, int part, float t, Vec4 *out, int costScale)
{
    float total = 0;
    float step;
    float walked = 0;
    Vec4 points[32];
    float marks[ROUTE_MAX_MARKS];
    int count = 0;
    marks[1] = 0;
    for (int i = 1; i < 16; i++)
    {
        points[2 * i].x = points[2 * i + 1].x = (float)DAT_10210fc0[map].entry.route[i - 1].x;
        points[2 * i].y = points[2 * i + 1].y = (float)DAT_10210fc0[map].entry.route[i - 1].y;
        points[2 * i].z = points[2 * i + 1].z = (float)DAT_10210fc0[map].entry.route[i - 1].z;
        if (i > 1)
        {
            step = (points[2 * i] - points[2 * (i - 1)]).Length();
            total += step;
            marks[2 * i] = total;
            if (i < part + 1)
                walked += step;
        }
        if (SignedField(DAT_10210fc0[map].entry.route[i - 1].code % 10000) != 0)
        {
            step = (float)(SignedField(DAT_10210fc0[map].entry.route[i - 1].code % 10000) * costScale);
            total += step;
            if (i < part)
                walked += step;
        }
        marks[2 * i + 1] = total;
        count++;
        if (SignedField(DAT_10210fc0[map].entry.route[i - 1].code / 10000) % 4 > 1)
            break;
    }
    total += (points[2 * count] - points[2]).Length();
    for (int k = 1; k < 2 * count + 2; k++)
        marks[k] = marks[k] / total;
    walked = walked / total + t;
    while (walked >= 1.0f)
        walked -= 1.0f;
    while (walked < 0.0f)
        walked += 1.0f;
    for (int k = 1; k < 2 * count + 2; k++)
    {
        if (k == 2 * count + 1 && walked >= marks[k])
        {
            walked -= marks[k];
            float room = 1.0f - marks[k];
            float scale = (walked < room) ? room : walked;
            walked = walked / scale;
            float rest = 1.0f - walked;
            out->Copy(points[2] * walked + points[k] * rest);
            return 0;
        }
        if (walked >= marks[k] && walked < marks[k + 1])
        {
            walked -= marks[k];
            float room = marks[k + 1] - marks[k];
            float scale = (walked < room) ? room : walked;
            walked = walked / scale;
            float rest = 1.0f - walked;
            out->Copy(points[k + 1] * walked + points[k] * rest);
            return 0;
        }
    }
    return 0;
}
