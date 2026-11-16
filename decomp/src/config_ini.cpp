#include <windows.h>
#include <stdlib.h>

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
