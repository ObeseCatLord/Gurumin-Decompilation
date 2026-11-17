extern "C" int DAT_1020d660[0x60];
extern "C" int DAT_104463f8[0x60];
extern "C" int DAT_10468230[0x60];
extern "C" int DAT_10447fd8[0x60];
extern "C" char DAT_1021f640[0x60];

extern "C" int FUN_100119d0(int id, int value, char flag)
{
    int i;
    for (i = 0; i < 0x60; i++)
    {
        if (DAT_1020d660[i] == 0)
            break;
    }
    if (i != 0x60)
    {
        DAT_1020d660[i] = 1;
        DAT_104463f8[i] = 0;
        if (value < 0)
            value = -value;
        DAT_10468230[i] = value;
        DAT_10447fd8[i] = id;
        DAT_1021f640[i] = flag;
    }
    return 0;
}

extern "C" int FUN_10011aa0(int id, int value)
{
    int i;
    for (i = 0; i < 0x60; i++)
    {
        if (DAT_1020d660[i] == 0)
            break;
    }
    if (i != 0x60)
    {
        DAT_1020d660[i] = 1;
        DAT_104463f8[i] = 0;
        if (value < 0)
            value = -value;
        DAT_10468230[i] = -100000 - value;
        DAT_10447fd8[i] = id;
    }
    return 0;
}
