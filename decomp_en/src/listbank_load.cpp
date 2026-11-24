// Script loading and variable declaration of the script parser (EN build). JP twin: decomp/src/listbank.cpp.
#include <stdlib.h>
#include <string.h>

#include "listbank.h"

// Copies the script `source` (`length` bytes, plus the byte after them) into `text` and compiles it.
// "//" comments outside double quotes are blanked out on the way: from the second slash to the end of the line
// every character becomes '\r', and the first slash is dropped. Returns 1.
int ListBank::LoadText(const char *source, int length)
{
    int in = 0;
    self = this;
    textLength = 0;
    memset(text, 0, sizeof(text));
    int out = 0;
    int inQuotes = 0;
    int inComment = 0;
    if (length >= 0)
    {
        do
        {
            char c = source[in];
            in++;
            text[out] = c;
            if (inComment == 0)
            {
                if (c == '"' && inQuotes == 0)
                    inQuotes = 1;
                else if (c == '"' && inQuotes == 1)
                    inQuotes = 0;
                if (out > 0 && inQuotes == 0 && c == '/' && text[out - 1] == '/')
                {
                    // Start of a comment: drop the first slash and blank out the rest of the line.
                    out--;
                    textLength--;
                    inComment = 1;
                }
            }
            if (inComment == 1)
            {
                if (text[out] == '\r' || text[out] == '\n')
                {
                    inComment = 0;
                }
                else
                {
                    text[out] = '\r';
                    out--;
                    textLength--;
                }
            }
            textLength++;
            out++;
        } while (in <= length);
    }
    CompileText();
    return 1;
}

// Declares variable `name` as the next entry of group `group` with storage of its own: types 0 and 1
// (numbers) get a new int set to 0, type 2 (text) a new 256-byte string set to "NULL". The entry is marked as not
// in use (m_e80c = 0) and keeps a pointer to the storage in m_440c. Returns 1.
int ListBank::DeclareVariable(const char *name, int group, char type)
{
    borrowed[group][variableCounts[group]] = 0;
    strcpy(variableNames[group][variableCounts[group]], name);
    if (type == 0 || type == 1)
    {
        values[group][variableCounts[group]].ints = new int;
        *values[group][variableCounts[group]].ints = 0;
    }
    else if (type == 2)
    {
        values[group][variableCounts[group]].texts = new char[1][LISTBANK_TEXT_SIZE];
        strcpy(values[group][variableCounts[group]].texts[0], "NULL");
    }
    variableTypes[group][variableCounts[group]] = type;
    variableCounts[group]++;
    return 1;
}

