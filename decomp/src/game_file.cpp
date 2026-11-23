// Engine file wrapper (most methods live in the engine library at 0x101b2xxx).

#include <stdio.h>

#include "game_file.h"

// 0x101ad9e0. Constructor: no file open yet. (The same code is shared with KeyTrack's constructor.)
GameFile::GameFile()
{
    handle = 0;
}
