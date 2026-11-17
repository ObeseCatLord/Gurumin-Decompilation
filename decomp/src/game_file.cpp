// Engine file wrapper (most methods live in the engine library at 0x101b2xxx).

#include <stdio.h>

// 0x28 bytes: the stdio handle and the base name of the open file (set by Open via Path_GetBaseName).
struct GameFile
{
    FILE *handle;
    char name[0x24];
    GameFile();
};

// 0x101ad9e0. Constructor: no file open yet. (The same code is shared with KeyTrack's constructor.)
GameFile::GameFile()
{
    handle = 0;
}
