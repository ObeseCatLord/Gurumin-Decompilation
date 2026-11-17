#include <string.h>

extern "C" int DAT_1030c908[0x22];
extern "C" int DAT_1030b870[0x22];

extern "C" int FUN_100235e0(int mode)
{
    int i;
    memset(DAT_1030c908, 0, sizeof(DAT_1030c908));
    memset(DAT_1030b870, 0xff, sizeof(DAT_1030b870));
    if (mode == 0)
    {
        for (i = 1; i <= 0x11; i++)
            DAT_1030c908[i] = 1;
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
