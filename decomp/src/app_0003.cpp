// Script flags and a rectangle test.

#include "game.h"

// Tests script flag `index` (4000 flags in m_3ee694, saved with the game). A negative index tests that the flag
// is clear. Flag 0 is always set; out-of-range indexes read as clear (or as set for the negated test).
int Game::M_100022b0(int index)
{
    if (index < 0)
    {
        index = -index;
        if (index > 3999)
            return 1;
        return m_3ee694[index] == 0;
    }
    if (index > 3999)
        return 0;
    if (index == 0)
        return 1;
    return m_3ee694[index];
}

// Sets script flag `index` to `value` (flag 0 and out-of-range indexes are ignored).
int Game::M_10002300(int index, int value)
{
    if (index > 3999)
        return 0;
    if (index != 0)
        m_3ee694[index] = value;
    return 1;
}

// 1 when the point (x, y) lies inside the rectangle (edges included), else 0.
extern "C" int __stdcall FUN_10008430(int x, int y, int left, int top, int right, int bottom)
{
    if (x < left || y < top || x > right || y > bottom)
        return 0;
    return 1;
}


// Reads the next word of the script text into `token`. Words are separated by line breaks, tabs and spaces;
// text between double quotes may contain them (the quotes themselves are dropped). A word starting with '/' is
// a comment: the rest of its line is skipped. Returns 1 when a word was read, 0 at the end of the text (with any
// partial word left in `token`).
int Game::ReadWord()
{
    int length = 0;
    int quotes = 0;
    token[0] = 0;
    for (;;)
    {
        // Skip separators before the word.
        for (;;)
        {
            int at = pos;
            if (at >= end)
                return 0;
            char c = text[at];
            if (c == '\r')
                pos = at + 1;
            else if (c == '\n')
                pos = at + 1;
            else if (c == '\t')
                pos = at + 1;
            else if (c == ' ')
                pos = at + 1;
            else
                break;
        }
        // Collect the word.
        for (;;)
        {
            if (pos >= end)
                return 0;
            char c = text[pos];
            if (c == '"')
            {
                quotes++;
                pos++;
                continue;
            }
            // Outside quotes (an even number seen so far) a separator ends the word.
            if (((quotes & 1) == 0 && c == '\r') || ((quotes & 1) == 0 && c == '\n') ||
                ((quotes & 1) == 0 && c == '\t') || ((quotes & 1) == 0 && c == ' '))
                return 1;
            if (length == 0 && c == '/')
                break;
            token[length] = c;
            pos++;
            length++;
            token[length] = 0;
        }
        // Comment: skip to the end of the line, then look for a word again.
        for (;;)
        {
            if (pos >= end)
                return 0;
            if (text[pos] == '\r')
                break;
            pos++;
        }
    }
}

// Like ReadWord, but stays on the current line: reaching a line break before the word starts returns 0 (no
// further word on this line). Used for commands whose arguments must be on the same line.
int Game::ReadWordInLine()
{
    int quotes = 0;
    token[0] = 0;
    int length = 0;
    for (;;)
    {
        // Skip tabs and spaces before the word; a line break ends the search.
        for (;;)
        {
            int at = pos;
            if (at >= end)
                return 0;
            char c = text[at];
            if (c == '\r')
                return 0;
            if (c == '\n')
                return 0;
            if (c == '\t')
                pos = at + 1;
            else if (c == ' ')
                pos = at + 1;
            else
                break;
        }
        // Collect the word.
        for (;;)
        {
            if (pos >= end)
                return 0;
            char c = text[pos];
            if (c == '"')
            {
                quotes++;
                pos++;
                continue;
            }
            // Outside quotes (an even number seen so far) a separator ends the word.
            if (((quotes & 1) == 0 && c == '\r') || ((quotes & 1) == 0 && c == '\n') ||
                ((quotes & 1) == 0 && c == '\t') || ((quotes & 1) == 0 && c == ' '))
                return 1;
            if (length == 0 && c == '/')
                break;
            token[length] = c;
            pos++;
            length++;
            token[length] = 0;
        }
        // Comment: skip to the end of the line, then look for a word again.
        for (;;)
        {
            if (pos >= end)
                return 0;
            if (text[pos] == '\r')
                break;
            pos++;
        }
    }
}
