// File path helpers.

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "map_vars.h"
#include "render_vars.h"

// Creates every directory along `path` (like mkdir -p): cuts the path at each backslash in turn and calls
// CreateDirectoryA on the prefix, then on the whole path. Returns 0.
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

// Stores folder + name (cut to 31 characters) as entry `index` of the 32-byte name table DAT_10fada90.
extern "C" int FUN_10169780(int index, const char *folder, const char *name)
{
    char buffer[256];
    strcpy(buffer, folder);
    strcat(buffer, name);
    buffer[0x1f] = 0;
    strcpy(DAT_10fada90[index], buffer);
    return 0;
}

// Same as FUN_10169780 for the 32-byte texture name table DAT_10ff65f8.
extern "C" int FUN_101699f0(int index, const char *folder, const char *name)
{
    char buffer[256];
    strcpy(buffer, folder);
    strcat(buffer, name);
    buffer[0x1f] = 0;
    strcpy(DAT_10ff65f8[index], buffer);
    return 0;
}


// Looks up folder + name (cut to 31 characters) in the model name table. Returns its index when it is there.
// Otherwise the name is stored in the first free entry (its reference count reset), that index goes to *freeSlot
// and -1 is returned. A full table is a fatal error: the original then calls fread with null arguments on purpose
// to crash.
extern "C" int FUN_10169840(const char *folder, const char *name, int *freeSlot)
{
    char buffer[256];
    strcpy(buffer, folder);
    strcat(buffer, name);
    buffer[0x1f] = 0;
    *freeSlot = -1;
    for (int i = 0; i < 0xc80; i++)
    {
        if (DAT_10fada90[i][0] != 0)
        {
            if (strcmp(buffer, DAT_10fada90[i]) == 0)
                return i;
        }
        else if (*freeSlot == -1)
        {
            DAT_10feb3e0[i] = 0;
            *freeSlot = i;
        }
    }
    if (*freeSlot == -1)
        fread(0, 1, 1, 0);
    strcpy(DAT_10fada90[*freeSlot], buffer);
    return -1;
}

// Same as FUN_10169840 for the texture name table (0x400 entries).
extern "C" int FUN_10169ab0(const char *folder, const char *name, int *freeSlot)
{
    char buffer[256];
    strcpy(buffer, folder);
    strcat(buffer, name);
    buffer[0x1f] = 0;
    *freeSlot = -1;
    for (int i = 0; i < 0x400; i++)
    {
        if (DAT_10ff65f8[i][0] != 0)
        {
            if (strcmp(buffer, DAT_10ff65f8[i]) == 0)
                return i;
        }
        else if (*freeSlot == -1)
        {
            DAT_10faae88[i] = 0;
            *freeSlot = i;
        }
    }
    if (*freeSlot == -1)
        fread(0, 1, 1, 0);
    strcpy(DAT_10ff65f8[*freeSlot], buffer);
    return -1;
}

// Path_GetBaseName: copies the file name of `path` without folder and extension into `out` (8 bytes are
// cleared first). Not byte-identical yet (see open.tsv).
extern "C" int FUN_10161120(char *out, const char *path)
{
    char buffer[256];
    strcpy(buffer, path);
    int start = 0;
    int dot = 0;
    memset(out, 0, 8);
    for (int i = 0; buffer[i] != 0; i++)
    {
        if (buffer[i] == '\\')
            start = i + 1;
        if (buffer[i] == '.')
            dot = i;
    }
    if (dot == 0)
        dot = strlen(buffer);
    buffer[dot] = 0;
    strcpy(out, buffer + start);
    return 0;
}
