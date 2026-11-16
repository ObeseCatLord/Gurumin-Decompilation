#include <windows.h>
#include <stdlib.h>

extern "C" int DAT_10f66aac;
extern "C" int DAT_10f66ab0;
extern "C" int DAT_10f66de8;
extern "C" int DAT_10f66dd0;

extern "C" int FUN_1015fa90(void)
{
    int remaining = DAT_10f66aac - DAT_10f66ab0 * DAT_10f66de8 - DAT_10f66dd0;
    if (remaining >= 9)
        remaining = 9;
    return remaining;
}

extern "C" BOOL FUN_1015fe20(const char *file, const char *section, const char *key, int value)
{
    char text[64];
    return WritePrivateProfileStringA(section, key, _itoa(value, text, 10), file);
}

extern "C" int FUN_1015fe70(const char *file, const char *section, const char *key, int *value)
{
    *value = GetPrivateProfileIntA(section, key, 0, file);
    return 1;
}
