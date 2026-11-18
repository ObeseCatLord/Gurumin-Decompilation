// The per-user data folder: <...>\falcom\USERS\<user>\ (saves and settings live there).
#include <windows.h>
#include <string.h>

// 0x1013e170. Builds the user folder in `path`: starting from `base` (or the program's own file name when `base`
// is 0) it strips one path element after the other until the path ends in "\falcom" (any case), then appends
// "\USERS\" and, when `user` is not empty, "<user>\". Returns 0, or -1 when there is no "\falcom" in the path.
extern "C" int FUN_1013e170(char *path, const char *user, const char *base)
{
    if (base == 0)
        GetModuleFileNameA(0, path, MAX_PATH);
    else
        strcpy(path, base);
    char *slash = strrchr(path, '\\');
    while (slash != 0 && _strnicmp("\\falcom", slash, 7) != 0)
    {
        *slash = 0;
        slash = strrchr(path, '\\');
    }
    if (slash != 0)
    {
        strcat(path, "\\USERS\\");
        if (user[0] != 0)
        {
            strcat(path, user);
            strcat(path, "\\");
        }
        return 0;
    }
    return -1;
}
