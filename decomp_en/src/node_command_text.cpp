// Word scanner of the node command text (EN build, EN-only: no JP twin found).
#include "node.h"

// Whether `c` separates the words of a command text.
static inline bool IsCommandSpace(char c)
{
    return c == '\t' || c == '\n' || c == '\r' || c == ' ';
}

// Copies the next word of `text`, starting at index *position, into `word` (a buffer of COMMAND_WORD_SIZE bytes) and
// leaves *position just after it. White space in front of the word is skipped. Returns 1 when a word was read, 0 when
// the text ends first or the word does not fit. Called by Node::ApplyCommandText.
extern "C" int __stdcall CommandText_ReadWord(char *word, const char *text, int *position)
{
    word[0] = 0;
    for (;;)
    {
        char c = text[*position];
        if (c == 0)
            return 0;
        if (!IsCommandSpace(c))
            break;
        *position = *position + 1;
    }
    char *out = word;
    while (text[*position] != 0 && !IsCommandSpace(text[*position]))
    {
        *out = text[*position];
        out++;
        ++*position;
        *out = 0;
        if (out - word + 1 >= COMMAND_WORD_SIZE)
            return 0;
    }
    return 1;
}
