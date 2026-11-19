// The per-user data folder: <...>\falcom\USERS\<user>\ (saves and settings live there).
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
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

// fsetup.dll's GetUserPath(user, path): writes the folder of Windows user `user` to `path`; non-zero on success.
typedef int (*GetUserPathProc)(const char *user, char *path);

// 0x1013e260 (UserFolder_FindNt). Writes the Application Data folder of user `user` followed by "\FALCOM\" to
// `path` (on Windows NT). The folder comes from GetUserPath in fsetup.dll, looked for next to `base` (or the
// program) and in each folder above it; without that DLL it is built from the profile folder of the current user
// (the desktop's location with its last element replaced by `user`). Returns 0 from fsetup.dll, 1 from the
// profile folder, -1 when neither works.
extern "C" int FUN_1013e260(char *path, const char *user, const char *base)
{
    char dllPath[MAX_PATH];
    LPMALLOC shellMalloc;
    LPITEMIDLIST desktop;
    HMODULE dll;
    path[0] = 0;
    if (base == 0)
    {
        if (GetModuleFileNameA(0, dllPath, MAX_PATH) == 0)
            goto from_profile;
    }
    else
        strcpy(dllPath, base);
    // try fsetup.dll in the folder of the path, then in each folder above it
    do
    {
        char *slash = strrchr(dllPath, '\\');
        if (slash == 0)
            break;
        strcpy(slash, "\\fsetup.dll");
        dll = LoadLibraryA(dllPath);
        slash = strrchr(dllPath, '\\');
        if (slash == 0)
            break;
        *slash = 0;
    } while (dll == 0);
    if (dll != 0)
    {
        GetUserPathProc getUserPath = (GetUserPathProc)GetProcAddress(dll, "GetUserPath");
        if (getUserPath != 0 && getUserPath(user, path) != 0)
        {
            strcat(path, "\\Application Data\\FALCOM\\");
            return 0;
        }
        FreeLibrary(dll);
    }
from_profile:
    if (SHGetMalloc(&shellMalloc) >= 0)
    {
        SHGetSpecialFolderLocation(GetDesktopWindow(), CSIDL_DESKTOP, &desktop);
        SHGetPathFromIDListA(desktop, path);
        shellMalloc->Free(desktop);
        shellMalloc->Release();
        char *slash = strrchr(path, '\\');
        if (slash != 0)
        {
            *slash = 0;
            slash = strrchr(path, '\\');
            if (slash != 0)
            {
                sprintf(slash, "\\%s%s", user, "\\Application Data\\FALCOM\\");
                return 1;
            }
        }
    }
    return -1;
}
