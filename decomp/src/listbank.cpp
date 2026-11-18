// Script parser buffers (ListBank = ResourceBanks = TokenReader: one object). It holds 8 groups of up to 0x80
// named entries (text, value, flag), the script text being parsed and an error flag (m_3632c) that makes every
// reader fail once an error was reported.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Callback the host installs to receive parser messages: (1, message text) for errors, (5000, text offset) for
// the position of the error in the script text.
typedef void (*ParserCallback)(int code, int value);

// A top-level `{ ... }` block of a script (its start in the text).
struct ScriptBlock
{
    char *start;              // first character after the opening brace
    char *end;                // first character after the closing brace
};

struct ListBank
{
    int *result;              // 0x00: where the last evaluated expression left its value
    int pad_0004;
    ParserCallback callback;  // 0x08: message callback (may be 0)
    char m_000c[8][0x80][0x10];
    char m_400c[8][0x80];
    int m_440c[8][0x100];
    char pad_640c[0x8400];
    char m_e80c[8][0x80];
    char names[0xdce][0x20];  // 0xec0c: names of the declared script variables
    int pad_2a5cc;
    int compiling;            // 0x2a5d0: 1 while CompileText runs
    int nameCount;            // 0x2a5d4: number of entries in `names`
    int globalNameCount;      // 0x2a5d8: names declared outside any script (kept between compiles)
    int m_2a5dc[8];
    ScriptBlock blocks[0x800];// 0x2a5fc: top-level `{ }` blocks of the text
    int blockCount;           // 0x2e5fc
    char text[0x7d00];        // 0x2e600: script text being parsed
    int textLength;           // 0x36300
    char *cursor[8];          // 0x36304: read position in `text`, one per nesting level
    int depth;                // 0x36324: current nesting level
    int pad_36328;
    int m_3632c;
    ListBank *self;           // 0x36330: set to this object by LoadText
    char m_36334[0x10][0x20];

    int M_10004680(int group);
    int M_100046d0(const char *text, int group, int value, char flag);
    int M_100048f0(const char *name, int *index);
    int M_10004950(char *buffer, void *value, char type);
    int M_100049e0(const char *text);
    int M_10004a30(const char *text, int *value);
    int M_10004aa0(const char *text, float *value);
    int ReportError(const char *message);
    int FindAll(const char *name, int *out);
    int FreeAll();
    int FreeBank(int group);                                   // 0x10007ce0 (resource_banks.cpp)
    int ReadStatement(char **text, char *statement);           // 0x10035b10
    int SkipBlock(char **text, int *level, int block);         // 0x10035a30
    int CompileStatement(const char *statement);               // 0x100ed9b0
    int CompileText();
    int LoadText(const char *source, int length);
    int GetIndexPart(const char *name, char *part, int n);    // 0x10036560: the n-th [ ] part of `name`
    int Evaluate(const char *expression, int flags);         // 0x100caf90: leaves the value in *result
    int FirstIndexValue(const char *name, int *value);
};

// Clears group `group`: entries that are not in use lose their values. Returns 1.
int ListBank::M_10004680(int group)
{
    m_2a5dc[group] = 0;
    for (int i = 0; i < 0x80; i++)
    {
        if (m_e80c[group][i] == 0)
        {
            m_440c[group][i] = 0;
            m_e80c[group][i] = 0;
        }
    }
    return 1;
}

// Appends entry (`text`, `value`, `flag`) to group `group` and marks it in use. Returns 1.
int ListBank::M_100046d0(const char *text, int group, int value, char flag)
{
    m_e80c[group][m_2a5dc[group]] = 1;
    strcpy(m_000c[group][m_2a5dc[group]], text);
    m_440c[group][m_2a5dc[group]] = value;
    m_400c[group][m_2a5dc[group]] = flag;
    m_2a5dc[group]++;
    return 1;
}

// Looks up `name` among the 16 registered keywords (prefix match); stores its index and returns 1, else 0.
int ListBank::M_100048f0(const char *name, int *index)
{
    for (int i = 0; i < 0x10; i++)
    {
        if (m_36334[i][0] != 0 && strncmp(name, m_36334[i], strlen(m_36334[i])) == 0)
        {
            *index = i;
            return 1;
        }
    }
    return 0;
}

// Formats a script value into `buffer` (256 bytes): type 0 int (%d), 1 float (%f), 2 string. Returns 1.
int ListBank::M_10004950(char *buffer, void *value, char type)
{
    if (type == 0)
        sprintf(buffer, "%d", *(int *)value);
    else if (type == 1)
        sprintf(buffer, "%f", *(float *)value);
    else if (type == 2)
        sprintf(buffer, "%s", (char *)value);
    buffer[0xff] = 0;
    return 1;
}

// 1 when `text` contains a parenthesis (a function call), 0 otherwise or after an error.
int ListBank::M_100049e0(const char *text)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        if (m_3632c != 0)
            return 0;
        if (text[i] == '(' || text[i] == ')')
            return 1;
    }
    return 0;
}

// Parses `text` as an integer when it only holds digits and signs; returns 1 on success.
int ListBank::M_10004a30(const char *text, int *value)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        if (m_3632c != 0)
            return 0;
        if (text[i] != '+' && text[i] != '-' && (text[i] < '0' || text[i] > '9'))
            return 0;
    }
    *value = atol(text);
    return 1;
}

// Parses `text` as a float when it only holds digits, signs and a decimal point; returns 1 on success.
int ListBank::M_10004aa0(const char *text, float *value)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        if (m_3632c != 0)
            return 0;
        if (text[i] != '+' && text[i] != '-' && text[i] != '.' && (text[i] < '0' || text[i] > '9'))
            return 0;
    }
    *value = (float)atof(text);
    return 1;
}

// Reports a script parse error: sets the error flag (m_3632c, which makes the readers above fail from now on),
// prints the message, passes it and the error's offset in the script text to the host callback, and frees all
// script buffers (ScriptBuffers_FreeAll 0x10007d50). Returns 0.
int ListBank::ReportError(const char *message)
{
    m_3632c = 1;
    printf("%s\n", message);
    if (callback != 0)
        callback(1, (int)message);
    if (callback != 0)
        callback(5000, cursor[depth] - text);
    FreeAll();
    return 0;
}

// Collects the index of every declared name equal to `name` (case-sensitive) into `out` and returns how many were
// found; stops with 0 once a parse error was reported.
int ListBank::FindAll(const char *name, int *out)
{
    int found = 0;
    for (int i = 0; i < nameCount; i++)
    {
        if (m_3632c != 0)
            return 0;
        if (strcmp(name, names[i]) == 0)
            out[found++] = i;
    }
    return found;
}

// Compiles the whole script text: top-level statements are compiled one by one, and each `{ ... }` block is recorded
// in `blocks` and skipped (blocks are compiled when they run). Script variables start again from the global ones.
// Returns 1, or 0 when an error stopped the compile.
int ListBank::CompileText()
{
    char *p;
    int level;
    char statement[256];
    FreeBank(0);
    int savedDepth = depth;
    nameCount = globalNameCount;
    compiling = 1;
    blockCount = 0;
    p = text;
    depth = 0;
    while (p < text + textLength)
    {
        if (m_3632c != 0)
            return 0;
        ReadStatement(&p, statement);
        cursor[0] = p;
        if (statement[0] == 0)
            p++;
        else if (statement[0] == '{')
        {
            level = 1;
            blocks[blockCount].start = p;
            SkipBlock(&p, &level, blockCount);
            blockCount++;
        }
        else
            CompileStatement(statement);
    }
    depth = savedDepth;
    compiling = 0;
    return 1;
}

// Skips a `{ ... }` block whose opening brace was just read: nested blocks are recorded in `blocks` (and their
// ends), braces inside double quotes do not count. On the closing brace *text points past it, *level is lowered and
// the end of block `block` is recorded; returns 1. Reaching the end of the text reports "}が足りない。"
// ("missing }") and returns 0. Not byte-identical yet: the original gives inQuotes its own stack slot, ours
// reuses the dead `text` parameter slot (see open.tsv).
int ListBank::SkipBlock(char **text, int *level, int block)
{
    char *p = *text;
    int inQuotes = 0;
    while (p < this->text + textLength)
    {
        if (m_3632c != 0)
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

// 0x100fb6d0. Copies the script `source` (`length` bytes, plus the byte after them) into `text` and compiles it.
// "//" comments outside double quotes are blanked out on the way: from the second slash to the end of the line
// every character becomes '\r', and the first slash is dropped. Returns 1.
int ListBank::LoadText(const char *source, int length)
{
    int inQuotes = 0;
    int inComment = 0;
    int out = 0;
    self = this;
    textLength = 0;
    memset(text, 0, sizeof(text));
    for (int in = 0; in <= length;)
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
    }
    CompileText();
    return 1;
}

// 0x10037470. For an array reference such as "name[expr]..." (a '[' after the first character): evaluates every
// bracketed index expression in turn and stores the value of the first one in `value` (0 when there is no '[').
// Up to eight index values are kept, though only the first is used. Returns 0.
int ListBank::FirstIndexValue(const char *name, int *value)
{
    int values[8];
    char part[256];
    char copy[256];
    *value = 0;
    int bracket = 0;
    for (int i = (int)strlen(name) - 1; i > 0; i--)
    {
        if (name[i] == '[')
            bracket = 1;
    }
    if (bracket != 0)
    {
        strcpy(copy, name);
        int n = 0;
        while (GetIndexPart(copy, part, n) != 0)
        {
            Evaluate(part, 0);
            values[n] = *result;
            n++;
        }
        *value = values[0];
    }
    return 0;
}

