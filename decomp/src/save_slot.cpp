#include <stdio.h>

struct GameFile
{
    FILE *handle;
    char name[0x24];
    GameFile();
    ~GameFile();
};

extern "C" char DAT_10f60590[];
int FUN_1003b2c0(const char *path, int mode);

int FUN_1005cdc0(const char *name, int mode)
{
    GameFile file;
    char path[256];
    sprintf(path, "%s%s", DAT_10f60590, name);
    return FUN_1003b2c0(path, mode);
}
