// Script data parser helpers and the font randomisation table.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listbank.h"
#include "script_vars.h"

// Reads a type name token: returns 0 for "int", 1 for "float", 2 for "string"; reports an error (and returns 0)
// when the token is missing or unknown.
char ListBank::ReadType(char **text)
{
    char word[256];
    if (ReadArgument(text, word) == 0)
    {
        ReportError(DAT_101d7f08);
        return 0;
    }
    if (strcmp(word, "int") == 0)
        return 0;
    if (strcmp(word, "float") == 0)
        return 1;
    if (strcmp(word, "string") == 0)
        return 2;
    ReportError(DAT_101d7ee4);
    return 0;
}

#include "game_file.h"

extern "C" const char *FUN_1013f8a0(const char *format, ...);

// Loads the 256 KB table fontr.dat into DAT_10f20590, or fills it with rand() when the file is missing.
// Also stores `value` in DAT_10f65558. C++ linkage: the GameFile local needs an exception frame.
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

// 0x1013f880 (Random). Returns the next entry of the random table loaded above; the position wraps at 0x10000.
extern "C" int FUN_1013f880(void)
{
    return DAT_10f20590[++DAT_10f65558];
}
