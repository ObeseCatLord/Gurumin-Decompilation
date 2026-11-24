// The front tables of the map data object (Game::LoadMapData): the part below 0x64f10 of the object that holds
// bin\map.itm's tables. Game (game.h) declares the menu fields over this range, so the tables are described by this
// overlay until the object is split; the tables from 0x64f10 on are Game members.
#ifndef GURUMIN_MAP_FRONT_H
#define GURUMIN_MAP_FRONT_H

#include "key_track.h"
#include "map_tables.h"

// Face table row (0x700 bytes): frames and names of up to 64 faces of one character.
struct MapFaceRow
{
    int frameA[0x40];
    int frameB[0x40];                         // 0x100
    int pad_0200[0x40];
    char names[0x40][MAP_LABEL_SIZE];         // 0x300
};

// Entry of "#BtMapTable" (0x30 bytes).
struct MapBtMapEntry
{
    int first;
    int second;
    int values[2];                            // 0x08
    char labels[2][MAP_LABEL_SIZE];           // 0x10
};

// Entry of "#CCTable" (0x124 bytes).
struct MapCcEntry
{
    char pad_0000[0x44];
    char name[0x60];                          // 0x44
    char labels[8][MAP_LABEL_SIZE];           // 0xa4
};

// Skill entry (0xcb4 bytes): "#SkillTable" numbers and names, then the motion curve "#MoveTable" fills.
struct MapSkillEntry
{
    int values[0x1c];
    char labels[0x1c][MAP_LABEL_SIZE];        // 0x70
    KeyBank motion;                           // 0x230
};

struct MapFront
{
    int levels[0x30][8];                      // 0x0000: "#LVTable" (five numbers each)
    char stageNames[0x40][0x40];              // 0x0600
    char stageCodes[0x40][0x20];              // 0x1600
    void (*progress)(const char *, int);      // 0x1e00: called while the file is parsed
    MapFaceRow faces[0x30];                   // 0x1e04
    MapBtMapEntry btMap[0x100];               // 0x16e04
    int btMapCount;                           // 0x19e04
    MapItemEntry items[0x80];                 // 0x19e08
    int pad_1fe08;
    MapCcEntry cc[0x100];                     // 0x1fe0c
    int ccCount;                              // 0x3220c
    MapSkillEntry skills[0x40];               // 0x32210
};

#endif
