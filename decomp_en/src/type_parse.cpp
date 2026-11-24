// Script type names (EN build). JP twin: type_parse.cpp.
#include <string.h>

#include "listbank.h"

// Error texts (Shift-JIS): "the type is missing / could not be read" and "the type is wrong".
#define TYPE_MISSING_TEXT "\x82\xa4\x82\xdc\x82\xad\x93\xc7\x82\xdf\x82\xdc\x82\xb9\x82\xf1\x81" "B"
#define TYPE_UNKNOWN_TEXT "\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B"

// Values ReadType returns.
const char TYPE_INT = 0;
const char TYPE_FLOAT = 1;
const char TYPE_STRING = 2;

// Reads a type name token: returns 0 for "int", 1 for "float", 2 for "string"; reports an error (and returns 0) when
// the token is missing or unknown. Called while parsing variable and function declarations.
char ListBank::ReadType(char **text)
{
    char word[0x100];
    if (ReadArgument(text, word) == 0)
    {
        ReportError(TYPE_MISSING_TEXT);
        return TYPE_INT;
    }
    if (strcmp(word, "int") == 0)
        return TYPE_INT;
    if (strcmp(word, "float") == 0)
        return TYPE_FLOAT;
    if (strcmp(word, "string") == 0)
        return TYPE_STRING;
    ReportError(TYPE_UNKNOWN_TEXT);
    return TYPE_INT;
}
