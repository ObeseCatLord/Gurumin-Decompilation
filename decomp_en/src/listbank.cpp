// EN script parser (ListBank). JP twin: decomp/src/listbank.cpp.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listbank.h"

// Number of registered keywords that FindKeyword searches.
#define KEYWORD_SEARCH_COUNT 0x10

// Clears group `group`: entries that are not in use lose their values. Returns 1. Called by CompileText.
int ListBank::ClearGroup(int group)
{
    variableCounts[group] = 0;
    for (int i = 0; i < LISTBANK_GROUP_ENTRIES; i++)
    {
        if (borrowed[group][i] == 0)
        {
            values[group][i].storage = 0;
            borrowed[group][i] = 0;
        }
    }
    return 1;
}

// Looks up `name` among the registered keywords (prefix match); stores its index and returns 1, else 0.
int ListBank::FindKeyword(const char *name, int *index)
{
    for (int i = 0; i < KEYWORD_SEARCH_COUNT; i++)
    {
        if (m_36334[i][0] != 0 && strncmp(name, m_36334[i], strlen(m_36334[i])) == 0)
        {
            *index = i;
            return 1;
        }
    }
    return 0;
}

// 1 when `text` contains a parenthesis (a function call), 0 otherwise or after an error.
int ListBank::HasParenthesis(const char *text)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        if (failed != 0)
            return 0;
        if (text[i] == '(' || text[i] == ')')
            return 1;
    }
    return 0;
}

// Reports a script parse error: sets the error flag (which makes the readers fail from now on), prints the
// message, passes it and the error's offset in the script text to the host callback, and frees all script
// buffers (FreeAll). Returns 0. Called by the readers on a syntax error.
int ListBank::ReportError(const char *message)
{
    failed = 1;
    printf("%s\n", message);
    if (callback != 0)
        callback(1, (int)message);
    if (callback != 0)
        callback(5000, cursor[depth] - text);
    FreeAll();
    return 0;
}

// Collects the index of every registered function named `name` (case-sensitive) into `out` and returns how many
// were found; stops with 0 once a parse error was reported. Called when a call expression is compiled.
int ListBank::FindAll(const char *name, int *out)
{
    int found = 0;
    for (int i = 0; i < builtinCount; i++)
    {
        if (failed != 0)
            return 0;
        if (strcmp(name, builtinNames[i]) == 0)
            out[found++] = i;
    }
    return found;
}

// Appends entry (`text`, `value`, `flag`) to group `group` and marks it in use. Returns 1. Called when constants
// are registered.
int ListBank::AppendEntry(const char *text, int group, int value, char flag)
{
    borrowed[group][variableCounts[group]] = 1;
    strcpy(variableNames[group][variableCounts[group]], text);
    values[group][variableCounts[group]].value = value;
    variableTypes[group][variableCounts[group]] = flag;
    variableCounts[group]++;
    return 1;
}

// Compiles the whole script text: top-level statements are compiled one by one, and each `{ ... }` block is
// recorded in `blocks` and skipped (blocks are compiled when they run). Script variables start again from the
// global ones. Returns 1, or 0 when an error stopped the compile. Called by LoadText.
int ListBank::CompileText()
{
    char *p;
    int level;
    char statement[256];
    FreeBank(0);
    int savedDepth = depth;
    builtinCount = globalBuiltinCount;
    compiling = 1;
    blockCount = 0;
    p = text;
    depth = 0;
    while (p < text + textLength)
    {
        if (failed != 0)
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

// Error reported when the script text ends in the middle of a statement ("the end was reached").
#define END_REACHED_TEXT "\x8f" "I\x82\xed\x82\xe8\x82\xdc\x82\xc5\x97\x88\x82\xbd\x81" "B"

// Looks for the label `case <value>` of a switch block, starting at `text` (just before the block's opening
// brace) and going on to the end of the script text. Only labels directly inside the block count: outside quotes
// and parentheses and at brace depth 1. The label's number may be followed by spaces, tabs or the colon. On a
// match `found` is set to the character after "case" and 1 is returned; 0 when the block ends (or the text does)
// without one, or on a parse error. Called when a switch statement runs.
int ListBank::FindCase(const char *value, char *text, char **found)
{
    char word[0x100];
    int quoted = 0;
    int parens = 0;
    int braces = 0;
    int opened = 0;
    int count = 0;
    for (; text < &this->text[textLength]; text++)
    {
        if (failed != 0)
            return 0;
        if (quoted == 0 && parens == 0 && braces == 1 && count >= 3 && text[-3] == 'c' && text[-2] == 'a' &&
            text[-1] == 's' && text[0] == 'e')
        {
            // `text` moves past "case" (to the character after it)
            text++;
            char *label = text;
            ReadWord(&label, word);
            for (int i = (int)strlen(word) - 1; i > 0; i--)
            {
                if (word[i] == ' ' || word[i] == '\t' || word[i] == ':')
                    word[i] = 0;
            }
            if (atol(value) == atol(word))
            {
                *found = text;
                return 1;
            }
            text += 2;
        }
        if (*text == '(')
            parens++;
        else if (*text == ')')
            parens--;
        if (*text == '"')
        {
            if (quoted == 1)
                quoted = 0;
            else if (quoted == 0)
                quoted = 1;
        }
        if (quoted == 0 && parens == 0)
        {
            if (*text == '{')
            {
                braces++;
                opened = 1;
            }
            if (*text == '}')
                braces--;
        }
        if (opened != 0 && braces == 0)
            return 0;
        count++;
    }
    return 0;
}

// Looks for the label `name:` (for goto) directly inside the block that starts at `text`, the same way FindCase
// looks for `case <value>`: outside quotes and parentheses and at brace depth 1, with the colon right after the
// name. The word read there may end in spaces, tabs or the colon. On a match `found` is set to the label's first
// character and 1 is returned; 0 when the block or the text ends, or on a parse error. Called when a goto runs.
int ListBank::FindLabel(const char *name, char *text, char **found)
{
    char word[0x100];
    int quoted = 0;
    int parens = 0;
    int braces = 0;
    int opened = 0;
    int count = 0;
    int length = strlen(name);
    for (; text < &this->text[textLength]; text++)
    {
        if (failed != 0)
            return 0;
        if (quoted == 0 && parens == 0 && braces == 1 && count >= length - 1 && text[length + 1] == ':')
        {
            text++;
            char *label = text;
            ReadWord(&label, word);
            for (int i = (int)strlen(word) - 1; i > 0; i--)
            {
                if (word[i] == ' ' || word[i] == '\t' || word[i] == ':')
                    word[i] = 0;
            }
            if (strcmp(name, word) == 0)
            {
                *found = text;
                return 1;
            }
            text += 2;
        }
        if (*text == '(')
            parens++;
        else if (*text == ')')
            parens--;
        if (*text == '"')
        {
            if (quoted == 1)
                quoted = 0;
            else if (quoted == 0)
                quoted = 1;
        }
        if (quoted == 0 && parens == 0)
        {
            if (*text == '{')
            {
                braces++;
                opened = 1;
            }
            if (*text == '}')
                braces--;
        }
        if (opened != 0 && braces == 0)
            return 0;
        count++;
    }
    return 0;
}

// Moves `*text` past one statement: an `if` (with its `else` branch, both skipped recursively), a `for`, `while`
// or `switch` statement, a `{ }` block or a plain statement ending in ';' (outside quotes, parentheses and
// braces). Returns 1; reports an error and returns 0 when the script text ends first (0 without a message after
// an earlier error). Called when a branch is not taken.
int ListBank::SkipStatement(char **text)
{
    char word[0x100];
    char *p = *text;
    int quoted = 0;
    int braces = 0;
    int count = 0;
    int opened = 0;
    int parens = 0;
    char *end = &this->text[textLength];
    for (; p < end; p++, count++)
    {
        if (failed != 0)
            return 0;
        if (quoted == 0 && parens == 0 && braces == 0)
        {
            if (count >= 1 && p[-1] == 'i' && p[0] == 'f')
            {
                // skip the condition and the statement, then an `else` branch if one follows
                *text = p;
                SkipStatement(text);
                p = *text;
                char *next = p;
                ReadWord(&next, word);
                if (strcmp(word, "else") == 0)
                {
                    *text = next;
                    SkipStatement(text);
                    p = *text;
                }
                *text = p;
                return 1;
            }
            if ((count >= 2 && p[-2] == 'f' && p[-1] == 'o' && p[0] == 'r') ||
                (count >= 4 && p[-4] == 'w' && p[-3] == 'h' && p[-2] == 'i' && p[-1] == 'l' && p[0] == 'e') ||
                (count >= 5 && p[-5] == 's' && p[-4] == 'w' && p[-3] == 'i' && p[-2] == 't' && p[-1] == 'c' &&
                 p[0] == 'h'))
            {
                *text = p;
                SkipStatement(text);
                return 1;
            }
        }
        if (*p == '(')
            parens++;
        else if (*p == ')')
            parens--;
        if (*p == '"')
        {
            if (quoted == 1)
                quoted = 0;
            else if (quoted == 0)
                quoted = 1;
        }
        if (quoted == 0 && parens == 0)
        {
            if (*p == '{')
            {
                braces++;
                opened = 1;
            }
            if (*p == '}')
                braces--;
            if (*p == ';' && braces == 0)
            {
                *text = p + 1;
                return 1;
            }
        }
        if (opened != 0 && braces == 0)
        {
            *text = p + 1;
            return 1;
        }
    }
    ReportError(END_REACHED_TEXT);
    *text = p;
    return 0;
}
