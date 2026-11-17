// Equipment and slot defaults.

#include <string.h>

extern "C" int DAT_1030c908[0x22];
extern "C" int DAT_1030b870[0x22];

// Resets the two 0x22-entry slot tables. Mode 0 (new game) enables slots 1..0x11; any other mode sets the
// special layout (slots 0x17, 0x18, 0x1f disabled, 0x19..0x1e set to the ids 0x6f4..0x6fa).
// Written as separate stores: a loop would compile to `rep stosd` under cl 13.00.
extern "C" int FUN_100235e0(int mode)
{
    memset(DAT_1030c908, 0, sizeof(DAT_1030c908));
    memset(DAT_1030b870, 0xff, sizeof(DAT_1030b870));
    if (mode == 0)
    {
        DAT_1030c908[0x01] = 1;
        DAT_1030c908[0x02] = 1;
        DAT_1030c908[0x03] = 1;
        DAT_1030c908[0x04] = 1;
        DAT_1030c908[0x05] = 1;
        DAT_1030c908[0x06] = 1;
        DAT_1030c908[0x07] = 1;
        DAT_1030c908[0x08] = 1;
        DAT_1030c908[0x09] = 1;
        DAT_1030c908[0x0a] = 1;
        DAT_1030c908[0x0b] = 1;
        DAT_1030c908[0x0c] = 1;
        DAT_1030c908[0x0d] = 1;
        DAT_1030c908[0x0e] = 1;
        DAT_1030c908[0x0f] = 1;
        DAT_1030c908[0x10] = 1;
        DAT_1030c908[0x11] = 1;
        return 0;
    }
    DAT_1030c908[0x17] = -1;
    DAT_1030c908[0x18] = -1;
    DAT_1030c908[0x1f] = -1;
    DAT_1030c908[0x19] = 0x6f4;
    DAT_1030c908[0x1a] = 0x6f7;
    DAT_1030c908[0x1b] = 0x6f6;
    DAT_1030c908[0x1c] = 0x6f8;
    DAT_1030c908[0x1d] = 0x6f9;
    DAT_1030c908[0x1e] = 0x6fa;
    return 0;
}
