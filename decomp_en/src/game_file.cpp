// The GameFile wrapper around a stdio file and the base-name helper (EN build).
#include <stdio.h>
#include <string.h>

#include "game_file.h"

const int PATH_BUFFER_SIZE = 0x400;     // longest path Path_GetBaseName copes with
const int BASE_NAME_CLEARED_BYTES = 8;  // bytes of the output cleared before the name is copied
const char PATH_SEPARATOR = '\\';       // separates the folders of a path
const int FILE_CHUNK_SIZE = 0x8000;     // GameFile::Read reads in blocks of this many bytes

// Constructor: no file open yet.
GameFile::GameFile()
{
    handle = 0;
}

// Destructor: closes the file when it is still open.
GameFile::~GameFile()
{
    if (handle != 0)
        fclose(handle);
    handle = 0;
}

// Closes the file when it is open. Called when a loader is done with the file.
void GameFile::Close()
{
    if (handle != 0)
        fclose(handle);
    handle = 0;
}

// Opens `path` with `mode` (closing an open file first) and keeps the base name of the path in `name`. Returns the
// stdio handle, or 0 when the path is empty or the file cannot be opened. Called by every file loader.
FILE *GameFile::Open(const char *path, const char *mode)
{
    if (path == 0 || path[0] == 0)
        return 0;
    if (handle != 0)
        fclose(handle);
    handle = 0;
    Path_GetBaseName(name, path);
    handle = fopen(path, mode);
    return handle;
}

// Reads `count` records of `size` bytes into `buffer`, in blocks of FILE_CHUNK_SIZE bytes. Does nothing when no file is
// open. Called by the loaders for headers and whole files.
size_t GameFile::Read(void *buffer, size_t size, size_t count)
{
    if (handle != 0)
    {
        int total = size * count;
        int done = 0;
        while (total > FILE_CHUNK_SIZE)
        {
            fread((char *)buffer + done, 1, FILE_CHUNK_SIZE, handle);
            done += FILE_CHUNK_SIZE;
            total -= FILE_CHUNK_SIZE;
        }
        return fread((char *)buffer + done, 1, total, handle);
    }
    return 0;
}

// Moves the file position (fseek). Called by the loaders to find a file's length.
int GameFile::Seek(int offset, int origin)
{
    return fseek(handle, offset, origin);
}

// Returns the current file position (ftell). Called after a seek to the end to get the file's length.
int GameFile::Size()
{
    return ftell(handle);
}

// Copies the file name of `path` without directory and extension into `out` (the part after the last backslash and
// before the last dot; the whole name when there is no dot). Returns 0. Called by the loaders to name their files.
extern "C" int Path_GetBaseName(char *out, const char *path)
{
    char copy[PATH_BUFFER_SIZE];
    strcpy(copy, path);
    int nameStart = 0;
    int dot = 0;
    memset(out, 0, BASE_NAME_CLEARED_BYTES);
    for (int i = 0; copy[i] != 0; i++)
    {
        if (copy[i] == PATH_SEPARATOR)
            nameStart = i + 1;
        if (copy[i] == '.')
            dot = i;
    }
    if (dot == 0)
        dot = strlen(copy);
    copy[dot] = 0;
    strcpy(out, copy + nameStart);
    return 0;
}
