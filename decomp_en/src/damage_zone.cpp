// Damage zones (EN build). Ported from the JP source decomp/src/damage_zone.cpp (APP.dll 0x1002ebf0, unmatched) and the
// EN bytes.
#include <stdlib.h>

#include "anim_model.h"
#include "char.h"
#include "event_slots.h"
#include "game.h"
#include "map_records.h"

// A character that is away from the world sits at an x below this; it is tested at its saved position instead.
const float DAMAGE_ZONE_AWAY_X = -20000.0f;

// One frame of zone character `index`: unless its zone's on flag is clear or its off flag set, the turn counter
// advances (wrapping after the zone's step) and the enemies from slot ENEMY_SLOT_FIRST + turn on, every step-th one,
// that stand inside the zone's box (or are away with their saved position inside it, when the zone character allows
// that) lose the zone's damage. Returns 0. Called every frame for a zone character. EN 0x63ac70.
extern "C" int DamageZone_Update(int index)
{
    int zone = g_characterModels[index].zone;
    if (g_mapRecords[zone].zone.onFlag != 0)
    {
        int offFlag = g_mapRecords[zone].zone.offFlag;
        int on = g_game.TestScriptFlag(g_mapRecords[zone].zone.onFlag);
        if (offFlag == 0)
        {
            if (on == 0)
                return 0;
        }
        else
        {
            if (on == 0)
                return 0;
            if (g_game.TestScriptFlag(offFlag) != 0)
                return 0;
        }
    }
    int step = g_mapRecords[zone].zone.step;
    if (++g_charState[index].turn > step)
        g_charState[index].turn = 0;
    for (int slot = g_charState[index].turn + ENEMY_SLOT_FIRST; slot < CHAR_GROUP_COUNT; slot += __max(1, step))
    {
        const Matrix4 &matrix = g_characterModels[slot].matrix;
        if ((matrix.m[MATRIX_TRANSLATION_X] > g_mapRecords[zone].zone.left &&
             matrix.m[MATRIX_TRANSLATION_X] < g_mapRecords[zone].zone.right &&
             matrix.m[MATRIX_TRANSLATION_Y] > g_mapRecords[zone].zone.front &&
             matrix.m[MATRIX_TRANSLATION_Y] < g_mapRecords[zone].zone.back &&
             matrix.m[MATRIX_TRANSLATION_Z] > g_mapRecords[zone].zone.bottom &&
             matrix.m[MATRIX_TRANSLATION_Z] < g_mapRecords[zone].zone.top) ||
            (g_charPhase[slot] == 0 && g_charInPlay[index] != 0 && matrix.m[MATRIX_TRANSLATION_X] < DAMAGE_ZONE_AWAY_X &&
             g_charMarkers[slot].x > g_mapRecords[zone].zone.left && g_charMarkers[slot].x < g_mapRecords[zone].zone.right &&
             g_charMarkers[slot].savedY > g_mapRecords[zone].zone.front &&
             g_charMarkers[slot].savedY < g_mapRecords[zone].zone.back &&
             g_charMarkers[slot].savedZ > g_mapRecords[zone].zone.bottom &&
             g_charMarkers[slot].savedZ < g_mapRecords[zone].zone.top))
        {
            int damage = g_mapRecords[zone].zone.damage;
            EventSlot_Add(slot, damage, 0);
            g_charRecords[slot].stats[STAT_HEALTH] -= damage;
        }
    }
    return 0;
}
