// MapPlacement: one placement record of the map's character lists (0xf4 bytes), and the two lists
// (map_place_characters.cpp). The records are read from the map file; a record applies when its map is 9999 or the
// current map (DAT_10c62ed0) and its four conditions hold (script flags, a negative number tests that the flag is
// clear).
// This is the one declaration of these types: include this header, never redeclare them.
#ifndef GURUMIN_MAP_PLACEMENT_H
#define GURUMIN_MAP_PLACEMENT_H

#include "char_slot.h"

#define PLACEMENT_CONDITIONS 4
#define PLACEMENT_MODEL_FILE_SIZE 0x10
#define PLACEMENT_NAME_SIZE 0x60
#define PLACEMENT_ANY_MAP 9999            // the record applies on every map
#define PLACEMENT_ACTOR_COUNT 0x100       // records of the actor list

struct MapPlacement
{
    int slot;                             // 0x00: actor list: the character slot; character list: the template slot
                                          // whose model is cloned
    int map;                              // 0x04: map the record applies to (PLACEMENT_ANY_MAP = all)
    int conditions[PLACEMENT_CONDITIONS]; // 0x08: script flags that must all hold
    int x;                                // 0x18: position
    int y;                                // 0x1c
    int z;                                // 0x20
    int look;                             // 0x24: heading (look % 100) * 30 degrees, costume variant (look % 1000) / 100,
                                          // tint (look % 100000) / 10000
    int routeRecord;                      // 0x28: map record (DAT_10210fc0) the character walks
    int scriptCode;                       // 0x2c: packed script code, kept in CharSlot::stats[9]
    union
    {
        int healthPercent;                // 0x30: character list: percent of the template's health
        int modelDefinition;              // 0x30: actor list: model definition (index into DAT_10b314c8)
    };
    int stat3Percent;                     // 0x34: percent of CharSlot::stats[3]
    int stat4Percent;                     // 0x38: percent of CharSlot::stats[4]
    int coinsPercent;                     // 0x3c: percent of CharSlot::coins
    int score;                            // 0x40: CharSlot::m_00d8
    int pad_0044[16];                     // 0x44
    char modelFile[PLACEMENT_MODEL_FILE_SIZE];  // 0x84: actor list: the character's model file
    char name[PLACEMENT_NAME_SIZE];       // 0x94: actor list: the character's display name
};

typedef char check_map_placement[sizeof(MapPlacement) == 0xf4 ? 1 : -1];

extern "C" MapPlacement DAT_1092eee8[PLACEMENT_ACTOR_COUNT];  // the actors (explicit slots, own model files)
extern "C" int DAT_1093e2e8;                                  // number of actors
extern "C" MapPlacement DAT_1093e2ec[];                       // the map's characters (cloned from template slots)
extern "C" int DAT_10993f6c;                                  // number of map characters

extern "C" char DAT_1021a3c0[CHAR_SLOT_COUNT][0x20];          // name of each slot's model node that carries its matrix
extern "C" int DAT_102192c0[CHAR_SLOT_COUNT];                 // per-slot value copied from the template slot
extern "C" char DAT_10c62e50[CHAR_SLOT_COUNT];                // per-slot flags cleared when the map is populated

extern "C" int DAT_10c66cc4;                                  // non-zero = the armour (slot 16) carries its item model
extern "C" int DAT_10c66cc8;                                  // the same for slot 6
extern "C" int DAT_10c66ccc;                                  // the same for slot 7
extern "C" int DAT_10c66cd0;                                  // the same for slot 11

extern "C" int FUN_1005def0(void);                            // Map_PlaceCharacters

#endif
