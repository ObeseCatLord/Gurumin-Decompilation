struct ItemInfo
{
    int type;
    char pad_0004[0xbc];
};

extern "C" ItemInfo DAT_1088e6b0[];
extern "C" char DAT_103150c0[0x50];
extern "C" char DAT_1042de28[0x50];
extern "C" int FUN_1000e810(void);

extern "C" int FUN_10012130(int *item, int *count)
{
    int id = *item;
    if (id != 0x10 && DAT_1088e6b0[id].type != 4)
    {
        int total = DAT_103150c0[id] + *count;
        if (total > 3)
        {
            *count = total - 3;
            DAT_103150c0[*item] = 3;
            DAT_1042de28[*item] = 1;
            FUN_1000e810();
            return 0;
        }
    }
    int total = DAT_103150c0[id] + *count;
    if (total > 99)
    {
        *count = total - 99;
        DAT_103150c0[*item] = 99;
        DAT_1042de28[*item] = 1;
        FUN_1000e810();
        return 0;
    }
    DAT_1042de28[id] = 1;
    DAT_103150c0[*item] += (char)*count;
    *count = 0;
    *item = 0;
    FUN_1000e810();
    return 1;
}