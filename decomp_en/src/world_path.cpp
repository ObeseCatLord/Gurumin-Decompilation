// Base-name helper linked into the world renderer (EN build): a second copy of Path_GetBaseName (game_file.cpp).
#include <string.h>

#include "game_file.h"

const int WORLD_PATH_BUFFER_SIZE = 0x400;     // longest path the copy copes with
const int WORLD_BASE_NAME_CLEARED_BYTES = 8;  // bytes of the output cleared before the name is copied
const char WORLD_PATH_SEPARATOR = '\\';       // separates the folders of a path

// Path_GetBaseNameWorldCopy: copies the file name of `path` without directory and extension into `out` (the part after
// the last backslash and before the last dot; the whole name when there is no dot). Returns 0. Called by the world
// renderer and the save slot code (WorldRenderer::LoadCharacter and others).
extern "C" int Path_GetBaseNameWorldCopy(char *out, const char *path)
{
    char copy[WORLD_PATH_BUFFER_SIZE];
    strcpy(copy, path);
    int nameStart = 0;
    int dot = 0;
    for (int cleared = 0; cleared < WORLD_BASE_NAME_CLEARED_BYTES; cleared++)
        out[cleared] = 0;
    for (int i = 0; copy[i] != 0; i++)
    {
        if (copy[i] == WORLD_PATH_SEPARATOR)
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
