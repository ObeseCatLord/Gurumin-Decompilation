// Scene tables cleared when a scene is reset (EN build). Their meaning is not recovered yet; names carry the EN address.
// This is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_SCENE_TABLES_H
#define GURUMIN_EN_SCENE_TABLES_H

#include "char.h"

// Groups of the first table, and ints per group (11 used, 1 spare).
const int SCENE_TABLE_GROUPS = 3;
const int SCENE_TABLE_GROUP_INTS = 12;
// Entries of the first table a reset clears per group, and the entry that is set to SCENE_TABLE_NONE afterwards.
const int SCENE_TABLE_CLEARED_ENTRIES = 11;
const int SCENE_TABLE_NONE_ENTRY = 9;
const int SCENE_TABLE_NONE = -1;
// Rows of the second table and ints per row.
const int SCENE_ROW_COUNT = 6;
const int SCENE_ROW_INTS = 6;

extern "C" int g_sceneTable9185d0[SCENE_TABLE_GROUPS][SCENE_TABLE_GROUP_INTS];   // EN 0x009185d0: three groups of values; entry 9 is -1 while unused
extern "C" int g_sceneTable9182e4[SCENE_ROW_COUNT][SCENE_ROW_INTS];              // EN 0x009182e4: rows whose first value is cleared by the reset
extern "C" int g_charGroupValue91fc18[CHAR_GROUP_COUNT];                         // EN 0x0091fc18: a value per character group, cleared like g_charGroupTimer

// Clears the scene tables above and the per-group timers (g_charGroupTimer, g_charGroupValue91fc18). Returns 0. Called by
// the scene update (EN 0x5b49d0). EN 0x6de960 (see scene_tables.cpp).
extern "C" int SceneTables_Reset(void);

#endif
