// Folder helpers of the EN build (EN-only: no JP twin found): the Falcom settings folder and directory removal.
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>

#include "desktop_path.h"

// Characters of the buffers the directory removal builds its search pattern and file paths in.
const int DIRECTORY_PATH_BYTES = 0x200;
// Profile folder used when the shell cannot name one, and the path separator whose last occurrence cuts the desktop
// folder down to the profile.
static const char FALLBACK_PROFILE_FORMAT[] = "C:\\Users\\%s";
const int PATH_SEPARATOR_CHAR = '\\';

// Builds the path of a Falcom data folder in `path`: the user's profile folder (CSIDL_PROFILE; else the parent of the
// desktop folder; else "C:\Users\<userName>") followed by "\Application Data\FALCOM\" and `name`. Called by the
// launcher when it looks for its settings. Returns 0.
extern "C" int FalcomPath_Get(char *path, const char *userName, const char *name, const char *modulePath)
{
    if (SHGetFolderPathA(0, CSIDL_PROFILE, 0, 0, path) < 0)
    {
        if (SHGetFolderPathA(0, CSIDL_DESKTOP, 0, 0, path) >= 0)
        {
            char *separator = strrchr(path, PATH_SEPARATOR_CHAR);
            if (separator != 0)
                *separator = 0;
            else
                sprintf(path, FALLBACK_PROFILE_FORMAT, userName);
        }
        else
            sprintf(path, FALLBACK_PROFILE_FORMAT, userName);
    }
    strcat(path, "\\Application Data\\FALCOM\\");   // folder below the profile that holds the Falcom games' data
    strcat(path, name);
    return 0;
}

// Deletes every file directly inside the directory `path`, then the directory itself (a directory that is not empty
// afterwards stays). Called when the launcher removes a folder it created.
void __stdcall Directory_RemoveWithFiles(const char *path)
{
    char pattern[DIRECTORY_PATH_BYTES];
    char filePath[DIRECTORY_PATH_BYTES];
    WIN32_FIND_DATAA found;
    sprintf(pattern, "%s\\*.*", path);
    HANDLE search = FindFirstFileA(pattern, &found);
    if (search != INVALID_HANDLE_VALUE)
    {
        sprintf(filePath, "%s\\%s", path, found.cFileName);
        DeleteFileA(filePath);
        while (FindNextFileA(search, &found) != 0)
        {
            sprintf(filePath, "%s\\%s", path, found.cFileName);
            DeleteFileA(filePath);
        }
        FindClose(search);
    }
    RemoveDirectoryA(path);
}
