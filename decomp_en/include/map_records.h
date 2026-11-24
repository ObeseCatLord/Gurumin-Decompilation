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

// Reads point `point` of route `route` for walker `walker`: stores its action (low two bits), value and group in
// *outAction, *outValue and *outGroup. When `position` lies within `radius` of the point on all three axes the walker
// moves on (see ROUTE_ACTION_*). Stores the point it heads for in *outPoint and that point's coordinates in `position`.
// Returns 1-3 for the actions 1-3 when the point was reached, else 0. EN 0x5edb90 (JP 0x1002f320).
extern "C" int Route_Follow(int walker, int route, int point, int *outPoint, float *position, int *outAction,
                            int *outValue, int *outGroup, int radius);

// Length of the route of map record `map` (walked until a point ends it and back), plus each point's extra cost
// times `costScale`. EN 0x5ebf60 (JP 0x10049990).
extern "C" float Map_RouteLength(int map, int costScale);
// Writes into `out` the point at fraction `t` of the way along the route, measured from the point before `part`.
// EN 0x5edd60 (JP 0x10049b00).
extern "C" int Map_RoutePosition(int map, int part, float t, Vec4 *out, int costScale);

// Number of entries of g_mapRecords that describe maps. EN 0x0092d014 (JP DAT_10216dc4).
extern "C" int g_mapEntryCount;
// Index of the map record of the map being played. EN 0x00916f70 (JP DAT_1020c2c0).
extern "C" int g_currentMapEntry;
// Bytes of an icon file name in g_mapIconNames.
const int MAP_ICON_NAME_SIZE = 0x10;
// The icon file names (index MapEntry::icon). EN 0x00848610 (JP DAT_101d65b0).
extern "C" char g_mapIconNames[][MAP_ICON_NAME_SIZE];

// Returns the texture slot of the current map's icon, or 0 when no entry has the current map's id. EN 0x609b20 (JP 0x1001e240).
extern "C" int Map_IconTexture(void);

#endif
