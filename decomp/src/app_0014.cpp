#include <windows.h>

struct TextureObject
{
    char data[0x38];
};

struct MoviePlayer
{
    int pad_0000[3];
    int m_000c;
    int M_10160a90(const char *path);
    int M_10161050();
    int M_10160960(const TextureObject *target);
};

extern "C" const char *FUN_1013f8a0(const char *format, ...);
extern "C" MoviePlayer DAT_10f08ab0;
extern "C" int DAT_10202478;
extern "C" TextureObject DAT_118d0ba8[1600];
extern "C" int DAT_104bfe74;
extern "C" int DAT_104bfe70;
extern "C" int DAT_10c62edc[0x1000];

extern "C" int FUN_10016580(const char *name)
{
    DAT_10f08ab0.M_10160a90(FUN_1013f8a0(name, 0));
    DAT_10f08ab0.M_10161050();
    DAT_10f08ab0.M_10160960(&DAT_118d0ba8[DAT_10202478]);
    DAT_104bfe74 = DAT_10f08ab0.m_000c;
    DAT_104bfe70 = 1;
    return 1;
}

extern "C" int FUN_10017b10(int index, int *high, int *low)
{
    int value = DAT_10c62edc[index];
    if ((value & 0xf) == 1)
    {
        *low = (value >> 5) & 0xff;
        *high = (DAT_10c62edc[index] >> 13) & 0xff;
        if (*low < 0 || *low > 0x80)
            *low = 0;
        if (*high < 0 || *high > 0xff)
            *high = 0;
        return 1;
    }
    return 0;
}

int SeasonalEvent()
{
    SYSTEMTIME now;
    GetLocalTime(&now);
    if (now.wMonth == 12)
    {
        if (now.wDay == 25 || (now.wDay == 24 && now.wHour > 18))
            return 1;
        return 0;
    }
    if (now.wMonth == 1 && now.wDay < 4)
        return 2;
    return 0;
}

