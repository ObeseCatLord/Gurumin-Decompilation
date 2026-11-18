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
    char resultType;          // 0x04: type of *result: 0 = int, 1 = float, 2 = quoted text
    char pad_0005[3];
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
    int ReadArgument(char **text, char *word);
    int ReadWord(char **text, char *word);
    int ReadTerm(char **text, char *word);
    int CompileStatement(const char *statement);               // 0x100ed9b0
    int CompileText();
    int LoadText(const char *source, int length);
    int GetIndexPart(const char *name, char *part, int n);    // 0x10036560: the n-th [ ] part of `name`
    int Evaluate(const char *expression, char type);
    int CompileCall(const char *text, int flags);              // 0x10089a10 (Script_CompileCall)
    int FirstIndexValue(const char *name, int *value);
    int DeclareVariable(const char *name, int group, char type);
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

// 0x10035b10. Reads the next statement into `statement` (at most 255 characters, followed by two zero bytes). A
// '{', '}' or ';' at the cursor is a statement of its own. Otherwise separators are skipped and the statement runs to a
// ';', brace or the end of the text outside quotes, brackets and parentheses (the ';' is consumed, a brace is left
// for the next call), or up to and including a ':' (a label). A closing parenthesis of the enclosing call ends it
// too, and after a ')' a letter or '_' (past any white space) starts the next statement. Tabs and line ends outside
// quotes are dropped. Returns 1, or 0 at the end of the text or after an error.
int ListBank::ReadStatement(char **text, char *statement)
{
    char *p = *text;
    statement[0] = 0;
    int inQuotes = 0;
    int parens = 0;
    int brackets = 0;
    if (*p == 0)
        return 0;
    if (*p == '{' || *p == '}' || *p == ';')
    {
        statement[0] = *p;
        statement[1] = 0;
        *text = p + 1;
        return 1;
    }
    while (*p == ' ' || *p == ',' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    int afterParen = 0;
    int length = 0;
    do
    {
        if (m_3632c != 0)
            return 0;
        if (inQuotes == 0)
        {
            while (*p == '\t' || *p == '\n' || *p == '\r')
                p++;
        }
        if (afterParen && ((*p >= 'a' && *p <= 'z') || *p == '_' || (*p >= 'A' && *p <= 'Z')))
        {
            *text = p;
            statement[length] = 0;
            statement[length + 1] = 0;
            return 1;
        }
        if (*p == '"')
        {
            if (inQuotes == 0)
                inQuotes = 1;
            else if (inQuotes == 1)
                inQuotes = 0;
        }
        if (*p != ' ' && *p != '\n' && *p != '\r' && *p != '\t')
            afterParen = 0;
        if (inQuotes == 0)
        {
            if (*p == '(')
                parens++;
            else if (*p == ')')
            {
                parens--;
                afterParen = 1;
            }
            if (parens < 0)
            {
                *text = p + 1;
                return 1;
            }
            if (*p == '[')
                brackets++;
            else if (*p == ']')
                brackets--;
            if (parens == 0 && brackets == 0 && (*p == ':' || *p == 0 || *p == ';' || *p == '{' || *p == '}'))
            {
                if (*p == ':')
                {
                    statement[length] = *p;
                    length++;
                    p++;
                    *text = p;
                    statement[length] = 0;
                    statement[length + 1] = 0;
                    return 1;
                }
                if (*p != '{' && *p != '}' && *p != ';')
                    p++;
                *text = p;
                statement[length] = 0;
                statement[length + 1] = 0;
                return 1;
            }
        }
        statement[length] = *p;
        length++;
        p++;
        statement[length] = 0;
        statement[length + 1] = 0;
    } while (length < 0x100);
    ReportError("\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B");
    *text = p;
    return 1;
}

// 0x10035d20. Reads the next argument of a call into `word`, like ReadWord, but parentheses nest too: the argument
// ends at a space, comma, ';' or the end of the text outside quotes, brackets and parentheses, or at the closing
// parenthesis of the call (not copied). *text is moved past the character that ended it.
int ListBank::ReadArgument(char **text, char *word)
{
    char *p = *text;
    word[0] = 0;
    int length = 0;
    int inQuotes = 0;
    int parens = 0;
    int brackets = 0;
    if (*p == 0)
        return 0;
    while (*p == ' ' || *p == ',' || *p == '\t' || *p == '\r' || *p == '\n' || *p == ';')
        p++;
    do
    {
        if (m_3632c != 0)
            return 0;
        if (*p == '"')
        {
            if (inQuotes == 0)
                inQuotes = 1;
            else if (inQuotes == 1)
                inQuotes = 0;
        }
        if (inQuotes == 0)
        {
            if (*p == '(')
                parens++;
            else if (*p == ')')
                parens--;
            if (parens < 0)
            {
                *text = p + 1;
                return 1;
            }
            if (*p == '[')
                brackets++;
            else if (*p == ']')
                brackets--;
            if (parens == 0 && brackets == 0 && (*p == 0 || *p == ' ' || *p == ',' || *p == ';'))
            {
                *text = p + 1;
                return 1;
            }
        }
        word[length] = *p;
        length++;
        p++;
        word[length] = 0;
        word[length + 1] = 0;
    } while (length < 0x100);
    ReportError("\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B");
    *text = p;
    return 1;
}


// 0x10035e80. Reads the next word of a statement into `word` (at most 255 characters, followed by two zero bytes):
// separators (space, comma, tab, line ends, ';') before it are skipped, and the word ends at a separator, a
// parenthesis or the end of the text outside double quotes and outside `[ ]` brackets. *text is moved past the word
// and its separator (a parenthesis or the end of the text is left in place). Returns 1, or 0 at the end of the text
// or after an error. A word that is too long reports "型が異常です。" ("the type is invalid") and returns 1.
int ListBank::ReadWord(char **text, char *word)
{
    char *p = *text;
    word[0] = 0;
    int length = 0;
    int inQuotes = 0;
    int brackets = 0;
    if (*p == 0)
        return 0;
    while (*p == ' ' || *p == ',' || *p == '\t' || *p == '\r' || *p == '\n' || *p == ';')
        p++;
    do
    {
        if (m_3632c != 0)
            return 0;
        if (*p == '"')
        {
            if (inQuotes == 0)
                inQuotes = 1;
            else if (inQuotes == 1)
                inQuotes = 0;
        }
        if (inQuotes == 0)
        {
            if (*p == '[')
                brackets++;
            else if (*p == ']')
                brackets--;
            if (brackets == 0 && (*p == '\n' || *p == '\r' || *p == 0 || *p == ')' || *p == '(' || *p == ' ' ||
                                  *p == ',' || *p == ';'))
            {
                if (*p != 0 && *p != ')' && *p != '(')
                    p++;
                *text = p;
                return 1;
            }
        }
        word[length] = *p;
        length++;
        p++;
        word[length] = 0;
        word[length + 1] = 0;
    } while (length < 0x100);
    ReportError("\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B");
    *text = p;
    return 1;
}

// 0x10035fa0. Like ReadWord, but '&' also ends a word, and a '&' at the start of a word is read as a word of its
// own (so `a&b` reads as "a", "&", "b").
int ListBank::ReadTerm(char **text, char *word)
{
    char *p = *text;
    word[0] = 0;
    int length = 0;
    int inQuotes = 0;
    int brackets = 0;
    if (*p == 0)
        return 0;
    while (*p == ' ' || *p == ',' || *p == '\t' || *p == '\r' || *p == '\n' || *p == ';')
        p++;
    do
    {
        if (m_3632c != 0)
            return 0;
        if (*p == '"')
        {
            if (inQuotes == 0)
                inQuotes = 1;
            else if (inQuotes == 1)
                inQuotes = 0;
        }
        if (inQuotes == 0)
        {
            if (*p == '[')
                brackets++;
            else if (*p == ']')
                brackets--;
            if (brackets == 0 && (*p == '&' || *p == '\n' || *p == '\r' || *p == 0 || *p == ')' || *p == '(' ||
                                  *p == ' ' || *p == ',' || *p == ';'))
            {
                if (*p == '&' && length == 0)
                {
                    word[0] = *p;
                    word[1] = 0;
                    word[2] = 0;
                    *text = p + 1;
                    return 1;
                }
                if (*p != '&' && *p != 0 && *p != ')' && *p != '(')
                    p++;
                *text = p;
                return 1;
            }
        }
        word[length] = *p;
        length++;
        p++;
        word[length] = 0;
        word[length + 1] = 0;
    } while (length < 0x100);
    ReportError("\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B");
    *text = p;
    return 1;
}

// 0x10036560. Copies the `n`-th (from 0) bracketed part of `name` into `part`: for "a[1][x+2]", part 1 is "x+2"
// (inner brackets and quoted text are kept, at most 255 characters, followed by two zero bytes). Returns 1 once the
// part is complete, 0 when `name` ends first (at its end, a space, comma or ';' outside the brackets), at an unmatched
// ')' or after an error.
int ListBank::GetIndexPart(const char *name, char *part, int n)
{
    part[0] = 0;
    int length = 0;
    int inQuotes = 0;
    int brackets = 0;
    int seenBracket = 0;
    int index = 0;
    int parens = 0;
    if (*name == 0)
        return 0;
    while (*name == ' ' || *name == ',' || *name == '\t' || *name == '\r' || *name == '\n' || *name == ';')
        name++;
    do
    {
        if (m_3632c != 0)
            return 0;
        if (*name == 0)
            return 0;
        if (*name == '"')
        {
            if (inQuotes == 0)
                inQuotes = 1;
            else if (inQuotes == 1)
                inQuotes = 0;
        }
        if (inQuotes == 0)
        {
            if (*name == '(')
                parens++;
            else if (*name == ')')
                parens--;
            if (parens < 0)
                return 0;
            if (*name == '[')
            {
                brackets++;
                seenBracket = 1;
                if (brackets == 1)
                    name++;
            }
            else if (*name == ']')
                brackets--;
            if (brackets == 0 && seenBracket == 1)
            {
                if (n <= index)
                    return 1;
                length = 0;
                part[0] = 0;
                part[1] = 0;
                index++;
            }
            if (parens == 0 && brackets == 0)
            {
                if (*name == 0 || *name == ' ' || *name == ',' || *name == ';')
                    return 0;
                name++;
                continue;
            }
        }
        if (brackets >= 1)
        {
            part[length] = *name;
            length++;
            part[length] = 0;
            part[length + 1] = 0;
        }
        name++;
    } while (length < 0x100);
    ReportError("\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B");
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

// 0x10004760. Declares variable `name` as the next entry of group `group` with storage of its own: types 0 and 1
// (numbers) get a new int set to 0, type 2 (text) a new 256-byte string set to "NULL". The entry is marked as not
// in use (m_e80c = 0) and keeps a pointer to the storage in m_440c. Returns 1.
int ListBank::DeclareVariable(const char *name, int group, char type)
{
    m_e80c[group][m_2a5dc[group]] = 0;
    strcpy(m_000c[group][m_2a5dc[group]], name);
    if (type == 0 || type == 1)
    {
        m_440c[group][m_2a5dc[group]] = (int)new int;
        *(int *)m_440c[group][m_2a5dc[group]] = 0;
    }
    else if (type == 2)
    {
        m_440c[group][m_2a5dc[group]] = (int)new char[0x100];
        strcpy((char *)m_440c[group][m_2a5dc[group]], "NULL");
    }
    m_400c[group][m_2a5dc[group]] = type;
    m_2a5dc[group]++;
    return 1;
}

// 0x100caf90. Compiles `expression` (its value lands in *result, of type resultType) and converts the value to
// `type`: 0 = int (a float is truncated, text is read with atol), 1 = float (text with atof), 2 = quoted text
// ("%d" or "%f" in quotes). An unknown result type gives 0, or "NULL" in quotes for text. Returns 1.
int ListBank::Evaluate(const char *expression, char type)
{
    CompileCall(expression, 0);
    if (type == 0)
    {
        if (resultType == 0)
        {
            // already an int
        }
        else if (resultType == 1)
            *result = (int)*(float *)result;
        else if (resultType == 2)
            *result = atol((char *)result + 1);
        else
            *result = 0;
    }
    else if (type == 1)
    {
        if (resultType == 0)
            *(float *)result = (float)*result;
        else if (resultType == 1)
        {
            // already a float
        }
        else if (resultType == 2)
            *(float *)result = (float)atof((char *)result + 1);
        else
            *result = 0;
    }
    else if (type == 2)
    {
        if (resultType == 0)
            sprintf((char *)result, "\"%d\"", *result);
        else if (resultType == 1)
            sprintf((char *)result, "\"%f\"", *(float *)result);
        else if (resultType != 2)
            strcpy((char *)result, "\"NULL\"");
    }
    return 1;
}
