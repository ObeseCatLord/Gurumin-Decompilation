// Save game helpers (gdataNN.sav in the save folder, see docs/SAVE_FORMAT.md).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Save game helpers (gdataNN.sav in the save folder).

// File wrapper from the engine library (0x101b2xxx): constructor 0x101ad9e0 clears the handle, destructor 0x101b2570
// closes it. 0x28 bytes: the size comes from Save_LoadSlot's stack frame.
struct GameFile
{
    FILE *handle;
    char name[0x24];  // base name of the open file (Path_GetBaseName 0x10161120)
    GameFile();
    ~GameFile();
    int Open(const char *path, const char *mode);    // 0x101b2310
    void Close();                                    // 0x101b22f0
    int Size();                                      // 0x101b2370
    int Seek(int offset, int origin);                // 0x101b2380
    int Read(void *buffer, int size, int count);     // 0x101b23a0
};

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                          // 0x101ac990: zero
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

extern "C" int FUN_10161120(char *name, const char *path);   // Path_GetBaseName
extern "C" int FUN_101615d0(unsigned char *data, int size, char *name, int sum, unsigned int check);  // ItmDescramble

// 0x10011240 (Save_ReadThumbnail, not called). Reads the 0x30000-byte thumbnail at the start of save `name` into
// `thumbnail`: a scrambled save (header 9999) is read whole and descrambled with its base name (every second of the
// first four characters shifted by the key), an old plain one (header 0x36b1) is read directly. Returns 1, or 0
// when the file is missing, has another header or does not descramble.
int FUN_10011240(const char *name, void *thumbnail)
{
    GameFile file;
    char path[0x278];
    sprintf(path, "%s%s", DAT_10f60590, name);
    file.Open(path, "rb");
    if (file.handle != 0)
    {
        Vec4 unused;
        int header;
        file.Read(&header, 1, 4);
        if (header == 9999)
        {
            char key[4];
            file.Read(key, 1, 4);
            file.Seek(0, 2);
            int size = file.Size() - 0x14;
            unsigned char *body = (unsigned char *)operator new(size);
            int offset = 0;
            file.Seek(8, 0);
            file.Read(body, 1, size);
            file.Read(&offset, 1, 4);
            int sum;
            file.Read(&sum, 1, 4);
            unsigned int check;
            file.Read(&check, 1, 4);
            file.Close();
            char baseName[0x100];
            FUN_10161120(baseName, name);
            baseName[0] += key[0];
            baseName[2] += key[0];
            baseName[4] += key[0];
            baseName[6] += key[0];
            if (FUN_101615d0(body, size, baseName, sum, check) != 0)
            {
                offset += 0x30000;
                memcpy(thumbnail, body, 0x30000);
                free(body);
                return 1;
            }
            free(body);
        }
        else if (header == 0x36b1)
        {
            file.Read(thumbnail, 1, 0x30000);
            file.Close();
            return 1;
        }
        else
            file.Close();
    }
    return 0;
}
