// Skipping of script blocks (EN build): kept apart from listbank.cpp because EN compiled it in another unit
// (CompileText calls SkipBlock).
#include "listbank.h"

// ListBank::SkipBlock: skips a `{ ... }` block whose opening brace was just read: nested blocks are recorded in
// `blocks` (and their ends), braces inside double quotes do not count. On the closing brace *text points past it,
// *level is lowered and the end of block `block` is recorded; returns 1. Reaching the end of the text reports a
// "missing }" error and returns 0. Called by CompileText and recursively.
int ListBank::SkipBlock(char **text, int *level, int block)
{
    char *p = *text;
    int inQuotes = 0;
    while (p < this->text + textLength)
    {
        if (failed != 0)
            return 0;
        if (*p == '"')
        {
            if (inQuotes == 1)
                inQuotes = 0;
            else if (inQuotes == 0)
                inQuotes = 1;
        }
        if (inQuotes == 0)
        {
            if (*p == '{')
            {
                (*level)++;
                p++;
                *text = p;
                blockCount++;
                blocks[block].start = p;
                SkipBlock(text, level, blockCount);
                p = *text;
            }
            if (*p == '}')
            {
                p++;
                blocks[block].end = p;
                (*level)--;
                *text = p;
                return 1;
            }
        }
        p++;
    }
    ReportError("}\x82\xaa\x91\xab\x82\xe8\x82\xc8\x82\xa2\x81" "B");
    *text = p;
    return 0;
}
