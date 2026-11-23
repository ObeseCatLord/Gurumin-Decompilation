// Damage zones: a zone character (its m_0280 picks the zone) hurts the enemies (slots 0x3a-0x5b) standing in its
// box, checking every step-th enemy per frame. Enemies that are away (x < -20000) are tested at their saved position.

#include <stdlib.h>

#include "damage_zone.h"

#include "vec3.h"

#include "anim_model.h"

#include "char_slot.h"

#include "game.h"

#include "char_state.h"

#include "map_records.h"
#include "char_tables.h"
#include "world.h"
extern "C" int FUN_100119d0(int index, int damage, int a);   // shows the damage on character `index`

// 0x1002ebf0 (DamageZone_Update). One frame of zone character `index`: unless its zone's on flag is clear or its
// off flag set, the turn counter advances (wrapping after `step`) and the enemies from slot 0x3a + turn on, every
// step-th one, that stand inside the box (or are away with their saved position inside it, when the zone
// character allows that) lose `damage` health. Returns 0.
extern "C" int FUN_1002ebf0(int index)
{
    int zone = DAT_10db6ee8[index].zone;
    if (DAT_10210fc0[zone].zone.onFlag != 0)
    {
        int offFlag = DAT_10210fc0[zone].zone.offFlag;
        int on = DAT_10874848.TestScriptFlag(DAT_10210fc0[zone].zone.onFlag);
        if (offFlag == 0)
        {
            if (on == 0)
                return 0;
        }
        else
        {
            if (on == 0)
                return 0;
            if (DAT_10874848.TestScriptFlag(offFlag) != 0)
                return 0;
        }
    }
    int step = DAT_10210fc0[zone].zone.step;
    if (++DAT_1020b430[index].turn > step)
        DAT_1020b430[index].turn = 0;
    for (int slot = DAT_1020b430[index].turn + 0x3a; slot < 0x5c; slot += __max(1, step))
    {
        if ((DAT_10db6ee8[slot].matrix.translation.x > DAT_10210fc0[zone].zone.left &&
             DAT_10db6ee8[slot].matrix.translation.x < DAT_10210fc0[zone].zone.right &&
             DAT_10db6ee8[slot].matrix.translation.y > DAT_10210fc0[zone].zone.front &&
             DAT_10db6ee8[slot].matrix.translation.y < DAT_10210fc0[zone].zone.back &&
             DAT_10db6ee8[slot].matrix.translation.z > DAT_10210fc0[zone].zone.bottom &&
             DAT_10db6ee8[slot].matrix.translation.z < DAT_10210fc0[zone].zone.top) ||
            (DAT_10210d30[slot] == 0 && DAT_1020d540[index] != 0 && DAT_10db6ee8[slot].matrix.translation.x < -20000.0f &&
             DAT_10867e18[slot].x > DAT_10210fc0[zone].zone.left && DAT_10867e18[slot].x < DAT_10210fc0[zone].zone.right &&
             DAT_10867e18[slot].y > DAT_10210fc0[zone].zone.front && DAT_10867e18[slot].y < DAT_10210fc0[zone].zone.back &&
             DAT_10867e18[slot].z > DAT_10210fc0[zone].zone.bottom && DAT_10867e18[slot].z < DAT_10210fc0[zone].zone.top))
        {
            int damage = DAT_10210fc0[zone].zone.damage;
            FUN_100119d0(slot, damage, 0);
            DAT_10daaae8[slot].stats[STAT_HEALTH] -= damage;
        }
    }
    return 0;
}
