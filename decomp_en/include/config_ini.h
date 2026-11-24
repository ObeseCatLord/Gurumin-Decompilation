// Integer settings in .ini files (EN build). Declared once here.
#ifndef GURUMIN_EN_CONFIG_INI_H
#define GURUMIN_EN_CONFIG_INI_H

// 1 when `key` of `section` in `file` holds an integer (the value reads back as something other than -1).
// EN 0x7af070 (JP 0x1015fe70 stores the value instead).
extern "C" int Config_HasInt(const char *file, const char *section, const char *key);

// Writes integer `value` as `key` in `section` of `file`; returns the Windows result. EN 0x7af370 (JP 0x1015fe20).
extern "C" int Config_SetInt(const char *file, const char *section, const char *key, int value);

#endif
