// Per-map entry settings (EN build; JP: decomp/src/map_entry_flags.cpp). Script flags 0x400, 0x418, 0x43c, 0x453 and 0x454
// hold the settings of the map being entered; this table sets them for each map number. The case values are the game's map
// numbers and the assigned values are the data of that map (spawn point, group, step, map id and event numbers).
#include "listbank.h"

// Indices in g_scriptFlags of the five entry settings.
const int ENTRY_FLAG_SPAWN_POINT = 0x400;   // spawn point number in the map
const int ENTRY_FLAG_SPAWN_GROUP = 0x418;   // spawn group number
const int ENTRY_FLAG_BGM_STEP = 0x43c;      // step of the map's music
const int ENTRY_FLAG_MAP_ID = 0x453;        // id of the map entry
const int ENTRY_FLAG_EVENT = 0x454;         // event number of the entry, 0 = none

// Map_SetEntryFlags. Sets the five entry flags for map `map` and returns 1, or returns 0 for a map without entry flags
// (only the event flag is cleared then). Called when a map is entered.
extern "C" int Map_SetEntryFlags(int map)
{
    switch (map)
    {
    case 0x15:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8f0;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0;
        return 1;
    case 0x16:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8f1;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0;
        return 1;
    case 0x17:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8f2;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0;
        return 1;
    case 0x18:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8f3;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0;
        return 1;
    case 0x19:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8f5;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0;
        return 1;
    case 0x1a:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8f6;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0;
        return 1;
    case 0x1b:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8f7;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0;
        return 1;
    case 0x6f:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x1e;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 1;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x32;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8ee;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4e2;
        return 1;
    case 0x66:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x24;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 3;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x37;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8d4;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4b0;
        return 1;
    case 0x69:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 1;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8d5;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4b2;
        return 1;
    case 0xc9:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x19;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 3;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x21;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8d8;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4ba;
        return 1;
    case 0xcb:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x3a;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 2;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x30;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8e1;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4bc;
        return 1;
    case 0xce:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 1;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8d9;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4be;
        return 1;
    case 0x12d:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x1b;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 5;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x12;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8dc;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4c6;
        return 1;
    case 0x1f5:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x18;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 1;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x1b;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8d6;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4b4;
        return 1;
    case 0x1f7:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x1e;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 1;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x1c;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8df;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4b6;
        return 1;
    case 0x130:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x2b;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 2;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x12;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8dd;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4c8;
        return 1;
    case 0x132:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 2;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8f4;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4ee;
        return 1;
    case 0x133:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8de;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0;
        return 1;
    case 0x1f9:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 1;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8d7;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4b8;
        return 1;
    case 0x25c:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x22;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 2;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x2e;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8e0;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4c2;
        return 1;
    case 0x259:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x35;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 4;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x25;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8da;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4c0;
        return 1;
    case 0x25e:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 1;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8db;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4c4;
        return 1;
    case 0x3e7:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 8;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4de;
        return 1;
    case 0x44e:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x1c;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 1;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x29;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8e4;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4cc;
        return 1;
    case 0x458:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x1a;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 3;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x29;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8ef;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4e4;
        return 1;
    case 0x4b5:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x2f;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 1;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x21;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8e7;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4d2;
        return 1;
    case 0x4b2:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x23;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 3;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x1b;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8e8;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4d4;
        return 1;
    case 0x519:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x1b;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 3;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0xf;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8eb;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4da;
        return 1;
    case 0x516:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x27;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 7;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 9;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8ec;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4dc;
        return 1;
    case 0x51c:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 1;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8ed;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4e0;
        return 1;
    case 0x5e0:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x2e;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 1;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x1b;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8e5;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4ce;
        return 1;
    case 0x5de:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x1a;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 0;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x19;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8e6;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4d0;
        return 1;
    case 0x645:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x2e;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 1;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x25;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8e9;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4d6;
        return 1;
    case 0x643:
        g_scriptFlags[ENTRY_FLAG_SPAWN_POINT] = 0x34;
        g_scriptFlags[ENTRY_FLAG_SPAWN_GROUP] = 5;
        g_scriptFlags[ENTRY_FLAG_BGM_STEP] = 0x30;
        g_scriptFlags[ENTRY_FLAG_MAP_ID] = 0x8ea;
        g_scriptFlags[ENTRY_FLAG_EVENT] = 0x4d8;
        return 1;
    }
    g_scriptFlags[ENTRY_FLAG_EVENT] = 0;
    return 0;
}
