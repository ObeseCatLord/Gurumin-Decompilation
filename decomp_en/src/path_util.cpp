// File path helpers (JP twin: path_util.cpp).

#include <string.h>
#include <windows.h>

// Bytes of the working copy of the path (MAX_PATH).
static const int PATH_BUFFER_SIZE = 260;
// Directory separator of Windows paths (a single backslash).
static const char PATH_SEPARATOR = '\\';

// Creates every directory along `path` (like mkdir -p): cuts a copy of the path at each backslash in turn and calls
// CreateDirectoryA on the prefix, then on the whole path. Called before the game writes a file into a folder that
// may not exist yet (save and config folders). Returns 0.
extern "C" int Path_CreateDirectories(const char *path)
{
    char buffer[PATH_BUFFER_SIZE];
    strcpy(buffer, path);
    char *slash = strchr(buffer, PATH_SEPARATOR) + 1;
    for (;;)
    {
        slash = strchr(slash, PATH_SEPARATOR);
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
