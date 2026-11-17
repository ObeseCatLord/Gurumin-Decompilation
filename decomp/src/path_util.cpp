#include <windows.h>
#include <string.h>

extern "C" char DAT_10fada90[][0x20];

extern "C" int FUN_1013e0c0(const char *path)
{
    char buffer[260];
    strcpy(buffer, path);
    char *slash = strchr(buffer, '\\') + 1;
    for (;;)
    {
        slash = strchr(slash, '\\');
        if (slash != 0)
            *slash = 0;
        CreateDirectoryA(buffer, 0);
        if (slash == 0)
            break;
        slash++;
        strcpy(buffer, path);
    }
    return 0;
}

extern "C" int FUN_10169780(int index, const char *folder, const char *name)
{
    char buffer[256];
    strcpy(buffer, folder);
    strcat(buffer, name);
    buffer[0x1f] = 0;
    strcpy(DAT_10fada90[index], buffer);
    return 0;
}
