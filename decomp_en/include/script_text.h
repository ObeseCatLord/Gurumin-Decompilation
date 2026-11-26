// Text helpers of the script reader (EN build).
#ifndef GURUMIN_EN_SCRIPT_TEXT_H
#define GURUMIN_EN_SCRIPT_TEXT_H

// 1 when `text` starts and ends with a double quote (a quoted script string), else 0. EN 0x5f4b00 (stdcall).
int __stdcall Text_IsQuoted(const char *text);

#endif
