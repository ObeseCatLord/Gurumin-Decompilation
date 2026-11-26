// Reset of the scene tables (EN build, EN-only: no JP twin found).
#include "scene_tables.h"

// SceneTables_Reset: clears every group of g_sceneTable9185d0 (entry 9 becomes SCENE_TABLE_NONE), the first value of
// each row of g_sceneTable9182e4 and the two per-character-group tables. Returns 0. Called by the scene update.
extern "C" int SceneTables_Reset(void)
{
    for (int group = 0; group < SCENE_TABLE_GROUPS; group++)
    {
        for (int entry = 0; entry < SCENE_TABLE_CLEARED_ENTRIES; entry++)
        {
            g_sceneTable9185d0[group][entry] = 0;
            if (entry == SCENE_TABLE_NONE_ENTRY)
                g_sceneTable9185d0[group][SCENE_TABLE_NONE_ENTRY] = SCENE_TABLE_NONE;
        }
    }
    for (int row = 0; row < SCENE_ROW_COUNT; row++)
        g_sceneTable9182e4[row][0] = 0;
    for (int group = 0; group < CHAR_GROUP_COUNT; group++)
        g_charGroupTimer[group] = 0;
    for (int group = 0; group < CHAR_GROUP_COUNT; group++)
        g_charGroupValue91fc18[group] = 0;
    return 0;
}
