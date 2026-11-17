// INI settings helpers (gurumin.ini style files, see docs/config_keys.tsv).

#include <windows.h>
#include <stdlib.h>

// Writes integer `value` as `key` in `section` of `file`.
extern "C" BOOL FUN_1015fe20(const char *file, const char *section, const char *key, int value)
{
    char text[64];
    return WritePrivateProfileStringA(section, key, _itoa(value, text, 10), file);
}

// Reads integer `key` from `section` of `file` into *value (0 when missing). Always returns 1.
extern "C" int FUN_1015fe70(const char *file, const char *section, const char *key, int *value)
{
    *value = GetPrivateProfileIntA(section, key, 0, file);
    return 1;
}
