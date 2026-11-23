// Save game helpers (gdataNN.sav in the save folder, see docs/SAVE_FORMAT.md).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Save game helpers (gdataNN.sav in the save folder).

#include "game_file.h"

#include "vec4.h"

#include "char_vars.h"
#include "game_vars.h"

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

#include "anim_model.h"
#include "char_slot.h"

extern "C" int DAT_10202464[4];
extern "C" float DAT_108029b8;                   // where the player starts (x, y, z)
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
            memcpy(&DAT_10daaae8[0], p, sizeof(CharSlot));
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
        FUN_1013e6f0(0, "\x83p\x83\x8a\x83\x93", DAT_10daaae8[0].name);
        DAT_10db6ee8[0].Teleport(x, y, z);
        FUN_100154c0(DAT_10daaae8[0].helmet);
        DAT_104bfd50 = 1;
        FUN_10015910();
        FUN_1000e750();
        FUN_1000e810();
        return 1;
    }
    return 0;
}

// "ポポン" (Popon), Shift-JIS: the name shown for a save made in costume 6.
#define POPON_NAME "\x83\x7c\x83\x7c\x83\x93"

// 0x10010d10 (Save_ReadSlotInfo). Reads what the save and load menus show for save `name`: the map, the play time,
// the player's name and the costume (with the difficulty variants 3 and 5, and 4 on map 1000). Handles the scrambled
// format (header 9999) and the old plain one (header 0x36b1). In costume 6 the name is "ポポン". Returns 1, or 0
// when the file is missing, has another header or does not descramble.
int FUN_10010d10(const char *name, int *map, int *playTime, char *playerName, int *costume)
{
    *playTime = 0;
    *map = 0;
    if (costume != 0)
        *costume = 0;
    playerName[0] = 0;
    int popon = 0;
    GameFile file;
    char path[0x104];
    sprintf(path, "%s%s", DAT_10f60590, name);
    file.Open(path, "rb");
    if (file.handle != 0)
    {
        Vec4 unused;
        int header;
        int mapNumber;
        int x;
        int y;
        int z;
        int w;
        char record[0x188];
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
                mapNumber = *(int *)(body + 0x30000);
                x = *(int *)(body + 0x30004);
                y = *(int *)(body + 0x30008);
                z = *(int *)(body + 0x3000c);
                w = *(int *)(body + 0x30010);
                memcpy(record, body + 0x3016c, 0x188);
                offset += 0x302f4;
                int *flags = (int *)operator new(0x4000);
                memcpy(flags, body + 0x302f4, 0x4000);
                offset += 0x4000;
                int look = flags[0x1c7];
                int difficulty = flags[0xf79];
                if (flags[0x9c5] == 6)
                    popon = 1;
                free(flags);
                *playTime = *(int *)(body + 0x34374);
                if (costume != 0)
                {
                    *costume = look;
                    if (difficulty == 1)
                        *costume = 3;
                    else if (difficulty == 2)
                        *costume = 5;
                }
                offset += 0x88;
                *map = mapNumber;
                if (mapNumber == 1000)
                    *costume = 4;
                free(body);
                goto copyName;
            }
            free(body);
        }
        else if (header == 0x36b1)
        {
            file.Seek(0x30000, 1);
            file.Read(&mapNumber, 1, 4);
            file.Read(&x, 1, 4);
            file.Read(&y, 1, 4);
            file.Read(&z, 1, 4);
            file.Read(&w, 1, 4);
            file.Seek(4, 1);
            file.Seek(0x140, 1);
            file.Seek(0x14, 1);
            file.Read(record, 1, 0x188);
            int *flags = (int *)operator new(0x4000);
            file.Read(flags, 1, 0x4000);
            int look = flags[0x1c7];
            int difficulty = flags[0xf79];
            if (flags[0x9c5] == 6)
                popon = 1;
            free(flags);
            file.Seek(0x80, 1);
            file.Read(playTime, 1, 4);
            if (costume != 0)
            {
                *costume = look;
                if (difficulty == 1)
                    *costume = 3;
                else if (difficulty == 2)
                    *costume = 5;
            }
            int unusedValue;
            file.Read(&unusedValue, 1, 4);
            *map = mapNumber;
            file.Close();
            goto copyName;
        }
        else
            file.Close();
        return 0;
    copyName:
        strcpy(playerName, &record[0x145]);
        if (popon != 0)
            strcpy(playerName, POPON_NAME);
        return 1;
    }
    return 0;
}
