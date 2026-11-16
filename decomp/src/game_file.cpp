#include <stdio.h>

struct GameFile
{
    FILE *handle;
    char name[0x100];
    GameFile();
};

GameFile::GameFile()
{
    handle = 0;
}
