// EN script text readers (ListBank): statements, words, arguments, terms. JP twin: decomp/src/listbank.cpp.
// Kept apart from listbank.cpp because EN compiled them in another unit (CompileText calls ReadStatement).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listbank.h"

// Reads the next statement into `statement` (at most 255 characters, followed by two zero bytes). A '{', '}' or
// ';' at the cursor is a statement of its own. Otherwise separators are skipped and the statement runs to a ';',
// brace or the end of the text outside quotes, brackets and parentheses (the ';' is consumed, a brace is left for
// the next call), or up to and including a ':' (a label). A closing parenthesis of the enclosing call ends it too,
// and after a ')' a letter or '_' (past any white space) starts the next statement. Tabs and line ends outside
// quotes are dropped. Returns 1, or 0 at the end of the text or after an error. Called by CompileText and the
// statement runner.
int ListBank::ReadStatement(char **text, char *statement)
{
    char *p = *text;
    statement[0] = 0;
    int length = 0;
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
    do
    {
        if (failed != 0)
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
    } while (length < READ_BUFFER_LIMIT);
    ReportError("\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B");
    *text = p;
    return 1;
}

// Reads the next word of a statement into `word` (at most 255 characters, followed by two zero bytes):
// separators (space, comma, tab, line ends, ';') before it are skipped, and the word ends at a separator, a
// parenthesis or the end of the text outside double quotes and outside `[ ]` brackets. *text is moved past the
// word and its separator (a parenthesis or the end of the text is left in place). Returns 1, or 0 at the end of
// the text or after an error. A word that is too long reports "the type is invalid" and returns 1.
// Called by the declaration and statement compilers.
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
        if (failed != 0)
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

// Like ReadWord, but '&' also ends a word, and a '&' at the start of a word is read as a word of its own (so
// `a&b` reads as "a", "&", "b"). Called by the expression readers.
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
        if (failed != 0)
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

// Copies the `n`-th (from 0) bracketed part of `name` into `part`: for "a[1][x+2]", part 1 is "x+2" (inner
// brackets and quoted text are kept, at most 255 characters, followed by two zero bytes). Returns 1 once the part
// is complete, 0 when `name` ends first (at its end, a space, comma or ';' outside the brackets), at an unmatched
// ')' or after an error. Called when an array element is read or assigned.
int ListBank::GetIndexPart(const char *name, char *part, int n)
{
    part[0] = 0;
    int parens = 0;
    int length = 0;
    int seenBracket = 0;
    int inQuotes = 0;
    int index = 0;
    int brackets = 0;
    if (*name == 0)
        return 0;
    while (*name == ' ' || *name == ',' || *name == '\t' || *name == '\r' || *name == '\n' || *name == ';')
        name++;
    do
    {
        if (failed != 0)
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

// Reads the next token of an expression into `token` (at most 255 characters, followed by two zero bytes): an
// operand (a name, number or quoted text, with its `( )` and `[ ]` parts) or an operator. Separators before it are
// skipped. An operand ends at an operator, a space, ';' or the end of the text outside quotes and brackets; an
// operator is one character, or two for `&& || -- ++ /= *= += -= %= != <= >= ==`. token[TOKEN_IS_OPERATOR] is set
// to 1 when the token is an operator. A '(' or ')' right after an operator ends the operator. Returns 1, or 0 at
// the end of the text or after an error. Called by the expression evaluator.
int ListBank::ReadToken(char **text, char *token)
{
    char *p = *text;
    int length = 0;
    token[TOKEN_IS_OPERATOR] = 0;
    token[0] = 0;
    int inQuotes = 0;
    int brackets = 0;
    int parens = 0;
    int last = TOKEN_START;
    if (*p == 0)
        return 0;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' || *p == ';')
        p++;
    do
    {
        if (failed != 0)
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
            while (*p == ' ' || *p == '\t')
                p++;
            if (*p == '(')
            {
                if (last == TOKEN_OPERATOR)
                {
                    *text = p;
                    return 1;
                }
                parens++;
            }
            else if (*p == ')')
            {
                if (last == TOKEN_OPERATOR)
                {
                    *text = p;
                    return 1;
                }
                parens--;
            }
            if (*p == '[')
                brackets++;
            else if (*p == ']')
                brackets--;
            if (parens == 0 && brackets == 0)
            {
                if (*p == ',' || *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '=' || *p == '%' ||
                    *p == '<' || *p == '>' || *p == '&' || *p == '!' || *p == '|')
                {
                    // an operator: it ends the operand before it
                    if (last == TOKEN_OPERAND)
                    {
                        *text = p;
                        return 1;
                    }
                    token[length] = *p;
                    token[length + 1] = 0;
                    p++;
                    last = TOKEN_OPERATOR;
                    // 1 = the next character cannot continue the operator
                    int ends = 1;
                    if (*p == ',' || *p == '+' || *p == '*' || *p == '/' || *p == '=' || *p == '%' || *p == '<' ||
                        *p == '>' || *p == '&' || *p == '!' || *p == '|')
                        ends = 0;
                    token[TOKEN_IS_OPERATOR] = 1;
                    char op = token[length];
                    // a two-character operator (each pair is tested on its own)
                    if (op == '&' && *p == '&')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '|' && *p == '|')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '-' && *p == '-')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '+' && *p == '+')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '/' && *p == '=')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '*' && *p == '=')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '+' && *p == '=')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '-' && *p == '=')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '%' && *p == '=')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '!' && *p == '=')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '<' && *p == '=')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '>' && *p == '=')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    if (op == '=' && *p == '=')
                    {
                        length++;
                        token[length] = *p;
                        token[length + 1] = 0;
                        *text = p + 1;
                        return 1;
                    }
                    // a sign or negation after the operator starts the next operand
                    if (*p == '!' || *p == '+' || *p == '-')
                        ends = 1;
                    // a single-character operator
                    if (op == '*' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                    if (op == '/' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                    if (op == '%' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                    if (op == '<' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                    if (op == '>' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                    if (op == '!' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                    if (op == '=' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                    if (op == '-' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                    if (op == '+' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                    if (op == ',' && ends)
                    {
                        *text = p;
                        return 1;
                    }
                }
                else
                {
                    // an operand character: it ends the operator before it
                    if (last == TOKEN_OPERATOR)
                    {
                        *text = p;
                        return 1;
                    }
                    last = TOKEN_OPERAND;
                }
                if (*p == 0 || *p == ' ' || *p == ';')
                {
                    if (*p == 0)
                        p[1] = 0;
                    *text = p + 1;
                    return 1;
                }
            }
        }
        token[length] = *p;
        length++;
        p++;
        token[length] = 0;
        token[length + 1] = 0;
    } while (length < 0x100);
    ReportError("\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B");
    *text = p;
    return 1;
}
