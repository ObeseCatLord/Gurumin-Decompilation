// EN .ini helpers (JP twin: config_ini.cpp).

#include <windows.h>
#include <stdlib.h>
#include "config_ini.h"

// Value GetPrivateProfileInt returns for a missing key.
const int CONFIG_MISSING = -1;

// 1 when `key` of `section` in `file` holds an integer.
extern "C" int Config_HasInt(const char *file, const char *section, const char *key)
{
    return GetPrivateProfileIntA(section, key, CONFIG_MISSING, file) != CONFIG_MISSING;
}

// Writes integer `value` as `key` in `section` of `file`.
extern "C" int Config_SetInt(const char *file, const char *section, const char *key, int value)
{
    char text[64];
    return WritePrivateProfileStringA(section, key, _itoa(value, text, 10), file);
}
