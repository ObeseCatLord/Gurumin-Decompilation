// Text helpers: CSV fields, ability names, OS-dependent paths.

#include <windows.h>
#include <string.h>

#include "world_renderer.h"

// 0x101672c0. Copies field number `field` (0-based, separated by commas or line ends) of line `line` into `out`.
// Returns 1, or -1 when the line has fewer fields.
int WorldRenderer::GetMessageField(int line, int field, char *out)
{
    char buffer[128];
    int length = 0;
    int count = 0;
    int size = strlen(messages[line]) + 2;
    for (int i = 0; i < size; i++)
    {
        char c = messages[line][i];
        if (c == 0 || c == '\n' || c == '\r' || c == ',')
        {
            if (count == field)
            {
                strcpy(out, buffer);
                return 1;
            }
            count++;
            length = 0;
        }
        else
        {
            buffer[length] = c;
            buffer[length + 1] = 0;
            length++;
        }
    }
    return -1;
}

extern "C" unsigned int FUN_100144c0(unsigned int flags, char *text);

// Builds the list of ability names for the ability bits in `flags` (Ability_GetDescription 0x100144c0 returns
// one name and its bit at a time), separated by the Shift-JIS middle dot. Used for the costume description.
extern "C" int FUN_1003bba0(unsigned int flags, char *out)
{
    char text[128];
    int any = 0;
    *out = 0;
    while (flags != 0)
    {
        unsigned int bit = FUN_100144c0(flags, text);
        if (bit == 0)
            break;
        if (any)
            strcat(out, "\x81\x45");
        strcat(out, text);
        flags &= ~bit;
        any = 1;
    }
    return 0;
}

extern "C" int FUN_1013e260(char *path, int a, int b);
extern "C" int FUN_1013e170(char *path, int a, int b);

// Builds a per-user data path into `path` with the NT (version 4+) or Windows 9x variant of the folder lookup,
// then appends `suffix`. Returns -1 when the lookup fails.
extern "C" int FUN_1013dfc0(char *path, int a, const char *suffix, int b)
{
    int result;
    OSVERSIONINFOA info;
    info.dwOSVersionInfoSize = sizeof(info);
    GetVersionExA(&info);
    if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        if (info.dwMajorVersion >= 4)
            result = FUN_1013e260(path, a, b);
    }
    else if (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        result = FUN_1013e170(path, a, b);
    if (result < 0)
        return -1;
    strcat(path, suffix);
    return 0;
}
