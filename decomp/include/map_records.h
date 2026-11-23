// MapEntry, MapRecord: declared here once. The map's record table at DAT_10210fc0: 0x100-byte records of sixteen
// four-integer rows, read as a map entry, a damage zone, a camera point or a route depending on the record.
#ifndef GURUMIN_MAP_RECORDS_H
#define GURUMIN_MAP_RECORDS_H

#include "route_point.h"
#include "damage_zone.h"
#include "camera_point.h"

// Rows (route points) per map record.
#define MAP_RECORD_ROWS 16

// A map record read as a map entry: its id and icon, then its route (map_icons.cpp, map_route.cpp).
struct MapEntry
{
    int id;                   // map id; several entries can share one
    int pad_0004;
    int icon;                 // 0x08: index into the icon name table
    int pad_000c;
    RoutePoint route[MAP_RECORD_ROWS - 1];  // 0x10
};

// One record of the table (0x100 bytes).
union MapRecord
{
    RoutePoint points[MAP_RECORD_ROWS];  // a route (route_follow.cpp, route_carrier.cpp) or per-state values
                                         // (npc_voice.cpp: the voice of a script node's state is points[state].y)
    MapEntry entry;
    DamageZone zone;
    CameraPoint camera;
};

extern "C" MapRecord DAT_10210fc0[];  // the map's records

#endif
