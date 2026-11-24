// Per-kind character update dispatch. CharSlot::kind (DAT_10daaae8) is the character's kind; kinds 100-107 are
// the special characters with their own update routines.

#include "barrage_enemy.h"
#include "perch_swooper.h"
#include "dash_slash_enemy.h"
#include "enemy_hover_strike.h"
#include "strike_enemy.h"
#include "world.h"
#include "anim_model.h"
#include "char_state.h"
#include "char_slot.h"

#include "char_tables.h"
#include "char_vars.h"
#include "enemy_chase_lunge.h"
#include "claw_enemy.h"

#include "bs001.h"
extern "C" int FUN_10011f80(int index);
extern "C" int FUN_10104640(int index);
extern "C" int FUN_100d24a0(int index);
extern "C" int FUN_1009d100(int index);
extern "C" int FUN_10011dd0(int index);
extern "C" int FUN_1009dfd0(int index);
extern "C" int FUN_1009ddd0(int index);
extern "C" int FUN_10098be0(int index);
extern "C" int FUN_1009a690(int index);
extern "C" int FUN_10099b30(int index);
extern "C" int FUN_10099040(int index);
extern "C" int FUN_1009d490(int index);
extern "C" int FUN_1009b120(int index);
extern "C" int FUN_1009c070(int index);

// Runs the kind-specific update of character `index` (kinds 100, 101, 105 and 107) while it is in the world and
// updates are not suspended. Always returns 0.
extern "C" int FUN_101148b0(int index)
{
    if (DAT_10db6ee8[index].matrix._41 > -50000.0f && DAT_104bfe78 == 0)
    {
        switch (DAT_10daaae8[index].kind)
        {
        case 100:
            Bs001_Update(index);
            return 0;
        case 101:
            FUN_10011f80(index);
            return 0;
        case 105:
            FUN_10104640(index);
            return 0;
        case 107:
            FUN_100d24a0(index);
            return 0;
        }
    }
    return 0;
}

// Runs the update routine of character `index` for kinds 0x47-0x51 (one routine per kind). Returns its result, or 0
// for other kinds.
extern "C" int FUN_1009e2d0(int index)
{
    int kind = DAT_10daaae8[index].kind;
    if (kind == 0x47)
        return FUN_1009d100(index);
    if (kind == 0x48)
        return FUN_10011dd0(index);
    if (kind == 0x49)
        return FUN_1009dfd0(index);
    if (kind == 0x4a)
        return FUN_1009ddd0(index);
    if (kind == 0x4b)
        return FUN_10098be0(index);
    if (kind == 0x4c)
        return FUN_1009a690(index);
    if (kind == 0x4d)
        return FUN_10099b30(index);
    if (kind == 0x4e)
        return FUN_10099040(index);
    if (kind == 0x4f)
        return FUN_1009d490(index);
    if (kind == 0x50)
        return FUN_1009b120(index);
    if (kind == 0x51)
        return FUN_1009c070(index);
    return 0;
}

// Enemy behaviour updaters selected by DAT_1020b230 (one value per character slot).
extern "C" int FUN_1001d2d0(int index);

// 0x100bcb30. Runs the behaviour updater of character `index` chosen by its behaviour number in DAT_1020b230
// (1..17 and 99; any other number uses Enemy_ChaseLungeAndSpit variant 0). Returns 0.
extern "C" int FUN_100bcb30(int index)
{
    switch (DAT_1020b230[index])
    {
    case 1:
        Enemy_StalkAndStrike(index, 0);
        return 0;
    case 2:
        Enemy_StalkAndStrike(index, 1);
        return 0;
    case 3:
    case 5:
        Enemy_StalkAndClaw(index, 2);
        return 0;
    case 4:
    case 6:
        Enemy_StalkAndClaw(index, 1);
        return 0;
    case 7:
        Enemy_StalkAndDashSlash(index);
        return 0;
    case 8:
        Enemy_HoverAndCallStrike(index, 0);
        return 0;
    case 9:
        Enemy_HoverAndCallStrike(index, 1);
        return 0;
    case 10:
        Enemy_PerchAndSwoop(index);
        return 0;
    case 11:
        Enemy_ChaseLungeAndSpit(index, 1);
        return 0;
    case 12:
        Enemy_AimedShots(index);
        return 0;
    case 13:
        Enemy_StarVolley(index);
        return 0;
    case 14:
        Enemy_SpinFire(index);
        return 0;
    case 15:
        Enemy_CallBarrage(index);
        return 0;
    case 16:
        Enemy_CallBursts(index, 0);
        return 0;
    case 17:
        Enemy_CallBursts(index, 1);
        return 0;
    case 99:
        FUN_1001d2d0(index);
        return 0;
    default:
        Enemy_ChaseLungeAndSpit(index, 0);
        return 0;
    }
}
