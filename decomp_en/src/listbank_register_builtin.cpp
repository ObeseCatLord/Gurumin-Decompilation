// Registering builtin script functions (EN build). Ported from the JP source decomp/src/listbank_vars.cpp
// (APP.dll 0x10036ca0, an open match there) and the EN bytes. This unit sees ListBank::ReportError as the inline member
// it was in the original.
#include <stdio.h>
#include <string.h>

#include "listbank.h"

// Error: "the type is invalid" (Shift-JIS).
#define BAD_PARAMETER_TYPE_TEXT "\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B"
// Error: "the function name is duplicated" (Shift-JIS).
#define DUPLICATE_FUNCTION_TEXT "\x8a\xd6\x90\x94\x96\xbc\x82\xaa\x8f" "d\x95\xa1\x82\xb5\x82\xc4\x82\xdc\x82\xb7\x81" "B"
// Error: "there is no matching bracket" (Shift-JIS).
#define NO_MATCHING_BRACKET_TEXT "\x91\xce\x89\x9e\x82\xb7\x82\xe9\x8a\x87\x8c\xca\x82\xaa\x82\xa0\x82\xe8\x82\xdc\x82\xb9\x82\xf1\x81" "B"
// Parameters a builtin can declare.
#define MAX_BUILTIN_PARAMS 0x10
// Added to a parameter type for "&" (passed by reference).
#define BY_REFERENCE_OFFSET 10
// Marks the end of a builtin's parameter type list.
#define PARAM_LIST_END ((char)0xff)

// Registers a builtin script function from its C-like signature, such as "int name(int, &float, string)": the return
// type, the name and up to 16 parameter types ("&" before a type adds BY_REFERENCE_OFFSET: passed by reference) with
// the handler and its id. A name already registered with the same parameter types, an unknown type or missing brackets
// is reported (the latter also clears the last result). Returns 1 when registered, 0 otherwise. Called by the library
// and game builtin set-up (InitLibrary, ListBank_Init).
int ListBank::RegisterBuiltin(const char *signature, short id, BuiltinHandler handler)
{
    char params[0x100];
    char word[0x100];
    char *cursor = (char *)signature;
    builtinHandlers[builtinCount].handler = handler;
    builtinIds[builtinCount] = id;
    builtinReturnTypes[builtinCount] = ReadType(&cursor);
    ReadWord(&cursor, builtinNames[builtinCount]);
    for (int j = 0; j < MAX_BUILTIN_PARAMS; j++)
        builtinParamTypes[builtinCount][j] = PARAM_LIST_END;
    strcpy(params, cursor);
    char *paramCursor = params;
    if (params[0] != '(')
    {
        ReportError(NO_MATCHING_BRACKET_TEXT);
        self->resultType = 0;
        *self->result = 0;
        return 0;
    }
    params[0] = ' ';
    if (params[strlen(params) - 1] != ')')
    {
        ReportError(NO_MATCHING_BRACKET_TEXT);
        self->resultType = 0;
        *self->result = 0;
        return 0;
    }
    params[strlen(params) - 1] = ' ';
    int i = 0;
    builtinParamTypes[builtinCount][0] = PARAM_LIST_END;
    do
    {
        if (failed != 0)
            return 0;
        ReadTerm(&paramCursor, word);
        char firstChar = word[0];
        builtinParamTypes[builtinCount][i] = 0;
        if (firstChar == '&')
        {
            builtinParamTypes[builtinCount][i - 1] += BY_REFERENCE_OFFSET;
            ReadTerm(&paramCursor, word);
            firstChar = word[0];
        }
        builtinParamCounts[builtinCount] = (char)i;
        if (firstChar == 0)
            break;
        if (strcmp(word, "int") != 0)
        {
            if (strcmp(word, "float") == 0)
                builtinParamTypes[builtinCount][i] += SCRIPT_TYPE_FLOAT;
            else if (strcmp(word, "string") == 0)
                builtinParamTypes[builtinCount][i] += SCRIPT_TYPE_TEXT;
            else
            {
                builtinParamTypes[builtinCount][i] = 0;
                ReportError(BAD_PARAMETER_TYPE_TEXT);
            }
        }
        i++;
        builtinParamCounts[builtinCount] = (char)i;
        builtinParamTypes[builtinCount][i] = PARAM_LIST_END;
    } while (i < MAX_BUILTIN_PARAMS);
    for (int k = 0; k < builtinCount; k++)
    {
        if (strcmp(builtinNames[k], builtinNames[builtinCount]) == 0 &&
            memcmp(builtinParamTypes[builtinCount], builtinParamTypes[k], MAX_BUILTIN_PARAMS) == 0)
            return ReportError(DUPLICATE_FUNCTION_TEXT);
    }
    builtinCount++;
    return 1;
}
