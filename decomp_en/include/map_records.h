// The map record table of the EN build (JP: decomp/include/map_records.h, route_point.h): 0x100-byte records of sixteen
// four-integer rows, read as a map entry with a route of up to 15 points. This is the one declaration in decomp_en.
#ifndef GURUMIN_EN_MAP_RECORDS_H
#define GURUMIN_EN_MAP_RECORDS_H

// Rows (route points) per map record.
#include "vec4.h"
const int MAP_RECORD_ROWS = 16;

// A route point: a position and a code. The code's low four digits (as a signed value, 5001..9999 meaning -4999..-1)
// are an extra cost of the point; the digits above them (signed the same way) modulo 4 above 1 end the route.
struct RoutePoint
{
    int x;      // 0x00: x position
    int y;      // 0x04: y position
    int z;      // 0x08: z position
    int code;   // 0x0c: extra cost and route-end digits
};

// A map record read as a map entry: its id and icon, then its route.
struct MapEntry
{
    int id;                                  // 0x00: map id; several entries can share one
    int pad_0004;                            // 0x04: not used
    int icon;                                // 0x08: index into the icon name table
    int pad_000c;                            // 0x0c: not used
    RoutePoint route[MAP_RECORD_ROWS - 1];   // 0x10: the route points
};

// One record of the table (0x100 bytes).
union MapRecord
{
    RoutePoint points[MAP_RECORD_ROWS];      // a route or per-state values
    MapEntry entry;                          // the record as a map entry
};

// The map's records. EN 0x00927210 (JP DAT_10210fc0): the table starts 0x10 before the first route point of record 0.
extern "C" MapRecord g_mapRecords[];

// A signed four-digit field: values above 5000 count down from 10000.
inline int SignedField(int value)
{
    if (value > 5000)
        value -= 10000;
    return value;
}

// Length of the route of map record `map` (walked until a point ends it and back), plus each point's extra cost
// times `costScale`. EN 0x5ebf60 (JP 0x10049990).
extern "C" float Map_RouteLength(int map, int costScale);
// Writes into `out` the point at fraction `t` of the way along the route, measured from the point before `part`.
// EN 0x5edd60 (JP 0x10049b00).
extern "C" int Map_RoutePosition(int map, int part, float t, Vec4 *out, int costScale);

#endif
