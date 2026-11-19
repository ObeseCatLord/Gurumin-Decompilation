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

int FUN_100106e0(const char *path);  // Save_Write (save_write.cpp; C++ linkage)

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

// The player character's model (CharModel::M_10162420): places it at (x, y, z).
struct PlayerModel
{
    int M_10162420(float x, float y, float z);
};

extern "C" char DAT_10daaae8[0x188];             // the player's slot record (CharSlot, 4 bytes before slot 0's)
extern "C" int DAT_10c62edc[0x1000];             // script flags
extern "C" int DAT_103150c0[0x20];
extern "C" int DAT_10202464[4];
extern "C" int DAT_1049f200[0x6000];
extern "C" int DAT_104bfd18;
extern "C" int DAT_104c00dc;                     // play time
extern "C" int DAT_1030b660;                     // map to enter
extern "C" int DAT_10c62ed0;                     // current map
extern "C" float DAT_108029b8;                   // where the player starts (x, y, z)
extern "C" float DAT_108029bc;
extern "C" float DAT_108029c0;
extern "C" int DAT_104bfd50;
extern "C" char DAT_10daac2d[];                  // the player's name
extern "C" int DAT_10daabb4;
extern "C" PlayerModel DAT_10db6ee8;
extern "C" int FUN_1013e6f0(int a, const char *text, const char *name);
extern "C" int FUN_100154c0(int value);
extern "C" void FUN_10015910(void);
extern "C" int FUN_1000e750(void);
extern "C" int FUN_1000e810(void);

// 0x1003b2c0 (Save_Load). Loads save file `path`: a scrambled save (header 9999) is read whole, descrambled with
// its base name (as in Save_ReadThumbnail) and its state is taken from behind the 0x30000-byte thumbnail: the map,
// the player position, DAT_104bfd18, the player's slot record, the 4096 script flags, DAT_103150c0, the play time,
// DAT_10202464 and the 0x18000-byte table DAT_1049f200. The game then enters that map with the player there.
// (An old save without the scrambled header takes the map and position from uninitialised locals.) `mode` is not
// used. Returns 1, or 0 when the file is missing or does not descramble.
int FUN_1003b2c0(const char *path, int mode)
{
    GameFile file;
    char name[0x100];
    sprintf(name, "%s", path);
    file.Open(name, "rb");
    if (file.handle != 0)
    {
        int header;
        int map;
        float x;
        float y;
        float z;
        int time;
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
            FUN_10161120(baseName, path);
            baseName[0] += key[0];
            baseName[2] += key[0];
            baseName[4] += key[0];
            baseName[6] += key[0];
            if (FUN_101615d0(body, size, baseName, sum, check) == 0)
            {
                free(body);
                return 0;
            }
            // the state behind the thumbnail, read with a cursor (`offset` follows it)
            unsigned char *p = body + 0x30000;
            offset += 0x30000;
            map = *(int *)p;
            p += 4;
            offset += 4;
            x = *(float *)p;
            p += 4;
            offset += 4;
            y = *(float *)p;
            p += 4;
            offset += 4;
            z = *(float *)p;
            p += 8;
            offset += 8;
            DAT_104bfd18 = *(int *)p;
            p += 0x158;
            offset += 0x158;
            memcpy(DAT_10daaae8, p, 0x188);
            p += 0x188;
            offset += 0x188;
            memcpy(DAT_10c62edc, p, 0x4000);
            p += 0x4000;
            offset += 0x4000;
            memcpy(DAT_103150c0, p, 0x80);
            p += 0x80;
            offset += 0x80;
            time = *(int *)p;
            p += 8;
            offset += 8;
            memcpy(DAT_10202464, p, 0x10);
            p += 0x10;
            offset += 0x10;
            memcpy(DAT_1049f200, p, 0x18000);
            offset += 0x18000;
            free(body);
        }
        DAT_104c00dc = time;
        DAT_10c62ed0 = DAT_1030b660 = map;
        DAT_108029b8 = x;
        DAT_108029bc = y;
        DAT_108029c0 = z;
        FUN_1013e6f0(0, "\x83p\x83\x8a\x83\x93", DAT_10daac2d);
        DAT_10db6ee8.M_10162420(x, y, z);
        FUN_100154c0(DAT_10daabb4);
        DAT_104bfd50 = 1;
        FUN_10015910();
        FUN_1000e750();
        FUN_1000e810();
        return 1;
    }
    return 0;
}
