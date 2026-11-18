// test.dat: groups of words (each up to 31 characters) read at startup; FUN_1000c020 replaces them in message text.
// A group starts after one skipped line and ends at an empty line or a ';' line.

#include <stdio.h>
#include <string.h>

extern "C" int DAT_104c02b0;                    // number of groups read
extern "C" int DAT_103084a8[0x80];              // number of words in each group
// Word tables, indexed by group * 256 + word.
extern "C" char DAT_1032d870[0x80 * 0x100][0x20];   // the words
extern "C" char DAT_104487e8[0x80 * 0x100];         // length of each word
extern "C" unsigned char DAT_1030ceb8[0x80 * 0x100];   // per word, set to 0xff when read
extern "C" const char *FUN_1013f8a0(const char *name, int folder);   // Path_Data

// 0x1000beb0 (TestDat_Load). Reads test.dat from the data folder into the word groups (more than 256 words in a group
// or 128 groups crash on purpose through fread(0, 1, 1, 0)). Returns 1, or 0 when the file is missing.
extern "C" int TestDat_Load(void)
{
    char line[128];
    FILE *file = fopen(FUN_1013f8a0("test.dat", 0), "rt");
    if (file == 0)
        return 0;
    DAT_104c02b0 = 0;
    int count = 0;
    do
    {
        while (1)
        {
            line[0] = 0;
            fgets(line, sizeof(line), file);
            if (count == 0)
                fgets(line, sizeof(line), file);
            int length = strlen(line);
            if (length > 0)
                line[length - 1] = 0;
            if (line[0] == 0 || line[0] == ';')
                break;
            int slot = DAT_104c02b0 * 0x100 + count;
            DAT_1030ceb8[slot] = 0xff;
            strcpy(DAT_1032d870[slot], line);
            DAT_104487e8[slot] = strlen(line);
            count++;
            if (count > 255)
                fread(0, 1, 1, 0);
        }
        DAT_103084a8[DAT_104c02b0] = count;
        count = 0;
        DAT_104c02b0++;
        if (DAT_104c02b0 > 127)
            fread(0, 1, 1, 0);
    } while (line[0] != 0);
    fclose(file);
    return 1;
}
