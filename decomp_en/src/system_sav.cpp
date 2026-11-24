// System save records (EN build). The JP version scrambles the local file; the EN (Steam) version keeps the file in
// Steam Remote Storage instead.
#include <stdlib.h>
#include <string.h>

#include "steam_storage.h"
#include "system_sav.h"

// Character that ends a folder name in a path.
const char PATH_SEPARATOR = '\\';
// Characters of the file name buffer.
const int FILE_NAME_SIZE = 260;

// SystemSav_WriteRecord: loads the cloud copy of the system save (named after the file part of `path`) when it exists,
// sets dword `index` of it to `value` and, when `write` is nonzero, writes it back. Returns 0. Called by the save
// functions.
extern "C" int SystemSav_WriteRecord(const char *path, int index, unsigned int value, char write)
{
    unsigned int *data = (unsigned int *)operator new(SYSTEM_SAV_SIZE);
    char fileName[FILE_NAME_SIZE];
    const char *slash = path + strlen(path) - 1;
    while (*slash != 0 && *slash != PATH_SEPARATOR)
        slash--;
    strcpy(fileName, slash + 1);
    if (SteamRemoteStorage()->FileExists(fileName))
        SteamRemoteStorage()->FileRead(fileName, data, SYSTEM_SAV_SIZE);
    data[index] = value;
    if (write != 0)
        SteamRemoteStorage()->FileWrite(fileName, data, SYSTEM_SAV_SIZE);
    free(data);
    return 0;
}
