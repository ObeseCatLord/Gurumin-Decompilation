// Picking up a counted item: a pickup effect goes into the ring of 32 effect slots (app_0009.cpp draws them) and the
// player's pickup counter (CharSlot stats[STAT_PICKUPS]) goes up; each full hundred is banked in DAT_10c64ad8 (up to 300, or 400 with DAT_10c64ad0).

#include "char_slot.h"

#include "char_vars.h"
#include "game_vars.h"

extern "C" int FUN_10011aa0(int id, int value);   // EventSlots_AddWaiting
extern "C" void FUN_1000e750(void);               // AntiTamper_Store

// 0x1003ab40 (Pickup_Collect). Collects a pickup: unless `quiet`, event `id` waits with value 2. The counter goes up
// by 1, by 2 with costume 11 or helmet bit 0x400000, by 3 with helmet bit 0x800000. Past 99 a hundred is banked and
// the counter restarts at 0, or it stays at 99 when the bank is full. The HUD counter shows for 8 frames and the
// anti-tamper copies are refreshed.
extern "C" void FUN_1003ab40(int id, int quiet)
{
    if (quiet == 0)
        FUN_10011aa0(id, 2);
    DAT_10255040[DAT_10210fb8] = 5;
    DAT_102196f0[DAT_10210fb8] = -1;
    DAT_1020d5c8[DAT_10210fb8] = 0;
    DAT_10210fb8++;
    if (DAT_10210fb8 > 31)
        DAT_10210fb8 = 0;
    int count = DAT_10daaae8[0].stats[STAT_PICKUPS] + 1;
    if (DAT_10c655f0 == 11)
        count++;
    else if (DAT_104c02c8 & 0x800000)
        count += 2;
    else if (DAT_104c02c8 & 0x400000)
        count++;
    if (count > 99)
    {
        int banked = DAT_10c64ad8;
        if (banked >= (DAT_10c64ad0 != 0 ? 100 : 0) + 300)
            count = 99;
        else
        {
            DAT_10c64ad8 = banked + 100;
            count = 0;
        }
    }
    DAT_10daaae8[0].stats[STAT_PICKUPS] = count;
    DAT_104c0119 = 8;
    FUN_1000e750();
}
