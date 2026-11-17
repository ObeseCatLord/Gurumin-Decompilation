// Save game helpers (gdataNN.sav in the save folder, see docs/SAVE_FORMAT.md).

#include <stdio.h>

// Save game helpers (gdataNN.sav in the save folder).

// File wrapper from the engine library (0x101b2xxx): constructor 0x101ad9e0 clears the handle, destructor 0x101b2570
// closes it. 0x28 bytes: the size comes from Save_LoadSlot's stack frame.
struct GameFile
{
    FILE *handle;
    char name[0x24];  // base name of the open file (Path_GetBaseName 0x10161120)
    GameFile();
    ~GameFile();
};

extern "C" char DAT_10f60590[];  // save directory prefix, e.g. "SaveData\\"
int FUN_1003b2c0(const char *path, int mode);

// Save_LoadSlot: loads save file `name` from the save folder through Save_Load (0x1003b2c0).
// The GameFile local is constructed and destroyed but never used (left over from older code); it is why the
// function has an exception frame. C++ linkage on purpose: an extern "C" definition would lose that frame.
int FUN_1005cdc0(const char *name, int mode)
{
    GameFile file;
    char path[256];
    sprintf(path, "%s%s", DAT_10f60590, name);
    return FUN_1003b2c0(path, mode);
}

extern "C" int FUN_100106e0(const char *path);  // Save_Write

// Writes the save file `name` (e.g. "gdata01.sav") into the save folder.
// DAT_10f60590 is the save directory prefix (with trailing backslash) set at start-up; Save_Write (0x100106e0)
// does the actual work (thumbnail + scrambled state blocks, see docs/SAVE_FORMAT.md).
extern "C" int FUN_1003b260(const char *name)
{
    char path[256];
    sprintf(path, "%s%s", DAT_10f60590, name);
    return FUN_100106e0(path);
}
