// Unpacking of a character's packed parameter (CharSlot::m_003c, set from the map data).

#include "world.h"
#include "char_state.h"
#include "char_slot.h"

#include "char_tables.h"

// Splits the packed value: digits from 100000 up go to the formation number (CharState m_0004[1]), the
// middle four digits (value / 10 % 10000) to CharState m_003c. When those middle digits are not zero the
// character is switched to kind 0x62, remembering its original kind, colour (m_00a7) and two slot values the
// first time so they can be restored later.
extern "C" int FUN_1001b5b0(int index)
{
    int value = DAT_10daaae8[index].m_00d8;
    DAT_1020b430[index].values[1] = value / 100000;
    int rest = value / 10 % 10000;
    DAT_1020b430[index].behaviour = rest;
    if (rest != 0)
    {
        DAT_1030ce38[index] = 1;
        if (DAT_1030b668[index] == 0)
        {
            DAT_1030b668[index] = DAT_10daaae8[index].kind;
            if (DAT_1030b668[index] == 0)
                DAT_1030b668[index] = -1;
            DAT_104679a0[index] = DAT_10daaae8[index].m_0143;
            DAT_10314ec0[index] = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            DAT_1021f440[index] = DAT_10daaae8[index].stats[STAT_VALUE_4];
        }
        DAT_10daaae8[index].kind = 0x62;
    }
    return 0;
}
