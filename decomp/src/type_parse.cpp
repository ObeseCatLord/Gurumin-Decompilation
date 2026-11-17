#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct TokenReader
{
    int M_100359c0(const char *message);
    int M_10035d20(int source, char *word);
    char M_100364a0(int source);
};

extern "C" const char DAT_101d7f08[];
extern "C" const char DAT_101d7ee4[];

char TokenReader::M_100364a0(int source)
{
    char word[256];
    if (M_10035d20(source, word) == 0)
    {
        M_100359c0(DAT_101d7f08);
        return 0;
    }
    if (strcmp(word, "int") == 0)
        return 0;
    if (strcmp(word, "float") == 0)
        return 1;
    if (strcmp(word, "string") == 0)
        return 2;
    M_100359c0(DAT_101d7ee4);
    return 0;
}

struct GameFile
{
    FILE *handle;
    char name[0x24];
    GameFile();
    ~GameFile();
    int Open(const char *path, const char *mode);
    int Read(void *buffer, int size, int count);
    void Close();
};

extern "C" short DAT_10f65558;
extern "C" int DAT_10f20590[0x10000];
extern "C" const char *FUN_1013f8a0(const char *format, ...);

int FUN_1014b1c0(short value)
{
    DAT_10f65558 = value;
    GameFile file;
    if (file.Open(FUN_1013f8a0("fontr.dat", 0), "rb") != 0)
    {
        file.Read(DAT_10f20590, 1, 0x40000);
        file.Close();
    }
    else
    {
        for (int i = 0; i < 0x10000; i++)
            DAT_10f20590[i] = rand();
    }
    return 0;
}
