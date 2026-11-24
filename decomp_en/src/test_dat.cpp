// test.dat word groups (EN build). Ported from the matched JP source decomp/src/test_dat.cpp.
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "test_dat.h"

// Characters of a line of test.dat, and the first character of a comment line.
const int TEST_DAT_LINE_SIZE = 128;
const char TEST_DAT_COMMENT = ';';
// Most words in a group (the index of the last one + 1 is the limit).
const int TEST_DAT_LAST_WORD = 255;
const int TEST_DAT_LAST_GROUP = 127;
// Marks a word whose replacement has not been chosen yet.
const unsigned char TEST_WORD_NOT_CHOSEN = 0xff;

// TestDat_Load: reads test.dat from the data folder into the word groups. A group of more than 256 words or more than
// 128 groups crash on purpose through fread(0, 1, 1, 0). Returns 1, or 0 when the file is missing. Called at start-up.
extern "C" int TestDat_Load(void)
{
    char line[TEST_DAT_LINE_SIZE];
    FILE *file = fopen(Path_Data("test.dat", 0), "rt");
    if (file == 0)
        return 0;
    g_testGroupCount = 0;
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
            if (line[0] == 0 || line[0] == TEST_DAT_COMMENT)
                break;
            g_testWordChoice[g_testGroupCount][count] = TEST_WORD_NOT_CHOSEN;
            strcpy(g_testWords[g_testGroupCount][count], line);
            g_testWordLength[g_testGroupCount][count] = strlen(line);
            count++;
            if (count > TEST_DAT_LAST_WORD)
                fread(0, 1, 1, 0);
        }
        g_testGroupWordCount[g_testGroupCount] = count;
        count = 0;
        g_testGroupCount++;
        if (g_testGroupCount > TEST_DAT_LAST_GROUP)
            fread(0, 1, 1, 0);
    } while (line[0] != 0);
    fclose(file);
    return 1;
}
