// Per-kind character update dispatch. CharSlot::m_0000 (DAT_10daab84) is the character's kind; kinds 100-107 are
// the special characters with their own update routines.

#include "world.h"

extern "C" int DAT_104bfe78;  // non-zero while character updates are suspended
extern "C" int FUN_1010c650(int index);
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
    if (DAT_10db6ed8[index].m_2368.x > -50000.0f && DAT_104bfe78 == 0)
    {
        switch (DAT_10daab84[index].m_0000)
        {
        case 100:
            FUN_1010c650(index);
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
    int kind = DAT_10daab84[index].m_0000;
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
