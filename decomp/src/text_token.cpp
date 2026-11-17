// Text_ReadToken: skips spaces, tabs and line breaks at text[*pos], then copies the next word into `out`
// (at most 127 characters) and advances *pos past it. Returns 1 for a word, 0 at the end of the text or when
// the word is too long. Not byte-identical yet (see open.tsv).
extern "C" int __stdcall FUN_1019b280(char *out, const char *text, int *pos)
{
    *out = 0;
    for (;;)
    {
        char c = text[*pos];
        if (c == 0)
            return 0;
        if (c != '\t' && c != '\n' && c != '\r' && c != ' ')
            break;
        (*pos)++;
    }
    char *start = out;
    for (;;)
    {
        char c = text[*pos];
        if (c == 0 || c == '\t' || c == '\n' || c == '\r' || c == ' ')
            return 1;
        *out++ = c;
        (*pos)++;
        *out = 0;
        if (out - start + 1 >= 0x80)
            return 0;
    }
}
