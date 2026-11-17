// Per-kind character update dispatch. CharSlot::m_0000 (DAT_10daab84) is the character's kind; kinds 100-107 are
// the special characters with their own update routines.

#include "world.h"

extern "C" int DAT_104bfe78;  // non-zero while character updates are suspended
extern "C" int FUN_1010c650(int index);
extern "C" int FUN_10011f80(int index);
extern "C" int FUN_10104640(int index);
extern "C" int FUN_100d24a0(int index);

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
