// Route lengths (EN build). Ported from the matched JP source decomp/src/map_route.cpp.
#include "map_records.h"

// Modulus of the two fields packed into a route point's code, and the number of route-end values.
const int ROUTE_CODE_MODULUS = 10000;
const int ROUTE_END_VARIANTS = 4;
// Route-end digit values above this end the route.
const int ROUTE_END_THRESHOLD = 1;

// Map_RouteLength: returns the length of the route of map record `map`, walked until a point ends it and back again,
// plus each point's extra cost times `costScale`. Point i is stored at entries i and 2 * i of one array and the
// segments are measured between the 2 * i entries. Because entry 2 is overwritten by the second point in the same
// step, the first segment measures 0 and the route closes at the second point, not the first.
extern "C" float Map_RouteLength(int map, int costScale)
{
    float total = 0;
    Vec4 points[2 * MAP_RECORD_ROWS];
    int count = 0;
    for (int i = 1; i < MAP_RECORD_ROWS; i++)
    {
        points[i].x = points[2 * i].x = (float)g_mapRecords[map].entry.route[i - 1].x;
        points[i].y = points[2 * i].y = (float)g_mapRecords[map].entry.route[i - 1].y;
        points[i].z = points[2 * i].z = (float)g_mapRecords[map].entry.route[i - 1].z;
        if (i > 1)
            total += (points[2 * i] - points[2 * (i - 1)]).Length();
        if (SignedField(g_mapRecords[map].entry.route[i - 1].code % ROUTE_CODE_MODULUS) != 0)
            total += (float)(SignedField(g_mapRecords[map].entry.route[i - 1].code % ROUTE_CODE_MODULUS) * costScale);
        count++;
        if (SignedField(g_mapRecords[map].entry.route[i - 1].code / ROUTE_CODE_MODULUS) % ROUTE_END_VARIANTS >
            ROUTE_END_THRESHOLD)
            break;
    }
    total += (points[2 * count] - points[2]).Length();
    return total;
}

// Number of entries of the running-length table: one per point and one per segment, counted from 1.
const int ROUTE_MAX_MARKS = 32;

// Map_RoutePosition: walks the same route as Map_RouteLength and writes into `out` the point that lies at fraction
// `t` of the way along it, measured from the point before `part` (so `t` is relative to that point; it wraps into
// [0, 1)). `marks` holds the running length after each segment and after each point's extra cost, normalised to 0..1;
// the point is interpolated inside the interval that contains the fraction, the last interval running back to the
// first point. Returns 0.
extern "C" int Map_RoutePosition(int map, int part, float t, Vec4 *out, int costScale)
{
    float total = 0;
    float step;
    float walked = 0;
    Vec4 points[2 * MAP_RECORD_ROWS];
    float marks[ROUTE_MAX_MARKS];
    int count = 0;
    marks[1] = 0;
    for (int i = 1; i < MAP_RECORD_ROWS; i++)
    {
        points[2 * i].x = points[2 * i + 1].x = (float)g_mapRecords[map].entry.route[i - 1].x;
        points[2 * i].y = points[2 * i + 1].y = (float)g_mapRecords[map].entry.route[i - 1].y;
        points[2 * i].z = points[2 * i + 1].z = (float)g_mapRecords[map].entry.route[i - 1].z;
        if (i > 1)
        {
            step = (points[2 * i] - points[2 * (i - 1)]).Length();
            total += step;
            marks[2 * i] = total;
            if (i < part + 1)
                walked += step;
        }
        if (SignedField(g_mapRecords[map].entry.route[i - 1].code % ROUTE_CODE_MODULUS) != 0)
        {
            step = (float)(SignedField(g_mapRecords[map].entry.route[i - 1].code % ROUTE_CODE_MODULUS) * costScale);
            total += step;
            if (i < part)
                walked += step;
        }
        marks[2 * i + 1] = total;
        count++;
        if (SignedField(g_mapRecords[map].entry.route[i - 1].code / ROUTE_CODE_MODULUS) % ROUTE_END_VARIANTS >
            ROUTE_END_THRESHOLD)
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
