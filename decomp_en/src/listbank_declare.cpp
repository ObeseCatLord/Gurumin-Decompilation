// EN script parser declarations: variable lists and script functions. JP twin: decomp/src/listbank.cpp.

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listbank.h"

// Error: "the variable name is duplicated".
#define DUPLICATE_NAME_TEXT "\x95\xcf\x90\x94\x96\xbc\x82\xaa\x8f" "d\x95\xa1\x82\xb5\x82\xc4\x82\xdc\x82\xb7\x81" "B"
// Message box text: "too many functions".
#define TOO_MANY_FUNCTIONS_TEXT "\x8a\xd6\x90\x94\x82\xaa\x91\xbd\x82\xb7\x82\xac"
// Error: "there is no matching bracket".
#define NO_MATCHING_BRACKET_TEXT "\x91\xce\x89\x9e\x82\xb7\x82\xe9\x8a\x87\x8c\xca\x82\xaa\x82\xa0\x82\xe8\x82\xdc\x82\xb9\x82\xf1\x81" "B"
// Error: "the argument has an abnormal description".
#define BAD_ARGUMENT_TEXT "\x88\xf8\x90\x94\x82\xc9\x88\xd9\x8f\xed\x82\xc8\x8b" "L\x8f" "q\x82\xaa\x82\xa0\x82\xe8\x82\xdc\x82\xb7\x81" "B"
// Error: "the type is invalid".
#define BAD_PARAMETER_TYPE_TEXT "\x8c^\x82\xaa\x88\xd9\x8f\xed\x82\xc5\x82\xb7\x81" "B"
// Error: "the function name is duplicated".
#define DUPLICATE_FUNCTION_TEXT "\x8a\xd6\x90\x94\x96\xbc\x82\xaa\x8f" "d\x95\xa1\x82\xb5\x82\xc4\x82\xdc\x82\xb7\x81" "B"
// Most functions the table holds (the check is "more than this").
#define FUNCTION_LIMIT 0xff
// Parameters of one function (the type array holds this many entries).
#define PARAMETER_LIMIT 0x10
// builtinIds value of a function declared in the script (it has a body instead of a handler).
#define SCRIPT_FUNCTION_ID -1
// Name given to a parameter that has none.
#define UNNAMED_PARAMETER "tmp"
// Added to a parameter's type when it is passed by reference ("&").
#define BY_REFERENCE 10

// Declares the variables listed in `declaration` in group `group`: each word is a variable name with optional
// "[size]" parts (every size is compiled; their product is the element count). The type comes from ReadType. A
// name that is already declared in the group is an error. Each variable gets storage for all its elements (zeroed
// numbers, or text elements preset to ""), and its sizes, dimension count and type are recorded. Returns 1 when
// the list is used up, 0 after an error. Called when the script declares variables.
int ListBank::DeclareFromText(char *declaration, int group)
{
    char *source = declaration;
    char type = ReadType(&source);
    int elements = 1;
    char word[LISTBANK_TEXT_SIZE];
    while (failed == 0)
    {
        char part[LISTBANK_TEXT_SIZE];
        ReadWord(&source, word);
        if (word[0] == 0)
            return 1;
        borrowed[group][variableCounts[group]] = 0;
        if (GetIndexPart(word, part, 0) != 0)
        {
            int dimension = 0;
            while (GetIndexPart(word, part, dimension) != 0)
            {
                CompileCall(part, 0);
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
                sizes[group][variableCounts[group]][dimension] = *result;
                elements = sizes[group][variableCounts[group]][dimension] * elements;
                dimension++;
                sizes[group][variableCounts[group]][dimension] = 0;
            }
            dimensions[group][variableCounts[group]] = dimension;
            int length = 0;
            while (word[length] != '[')
                length++;
            word[length] = 0;
            strcpy(variableNames[group][variableCounts[group]], word);
        }
        else
        {
            strcpy(variableNames[group][variableCounts[group]], word);
            sizes[group][variableCounts[group]][0] = 0;
            dimensions[group][variableCounts[group]] = 0;
        }
        for (int i = 0; i < variableCounts[group]; i++)
        {
            if (strcmp(variableNames[group][i], word) == 0)
                return ReportError(DUPLICATE_NAME_TEXT);
        }
        if (type == 0)
        {
            values[group][variableCounts[group]].ints = new int[elements];
            values[group][variableCounts[group]].ints[0] = 0;
            for (int i = 0; i < elements; i++)
                values[group][variableCounts[group]].ints[i] = 0;
        }
        else if (type == 1)
        {
            values[group][variableCounts[group]].floats = new float[elements];
            values[group][variableCounts[group]].floats[0] = 0;
            for (int i = 0; i < elements; i++)
                values[group][variableCounts[group]].floats[i] = 0;
        }
        else if (type == 2)
        {
            values[group][variableCounts[group]].storage = new char[elements * LISTBANK_TEXT_SIZE];
            for (int i = 0; i < elements; i++)
                strcpy(values[group][variableCounts[group]].texts[i], "\"\"");
        }
        variableTypes[group][variableCounts[group]] = type;
        variableCounts[group]++;
    }
    return 0;
}

// Declares a function written in the script, `statement` being its header "type name(type name, &type name, ...)":
// the function table gets the body text (the read position of level 0), the return type, the name and the
// parameter types and names (a "&" before a parameter name marks it as passed by reference). A parameter without
// a name gets "tmp", a name equal to a type word reports a wrong argument, and a name already declared with the
// same parameter types is a duplicate. Returns 1 when declared, 0 after an error (the missing-bracket errors
// also clear the last result). Called while the text is compiled.
int ListBank::DeclareFunction(char *statement)
{
    // The scope of the header parsing: its buffers share their stack space with the loop below.
    {
        char *source = statement;
        char params[LISTBANK_TEXT_SIZE];
        char word[LISTBANK_TEXT_SIZE];
        if (builtinCount > FUNCTION_LIMIT)
            MessageBoxA(0, TOO_MANY_FUNCTIONS_TEXT, "err", 0);
        builtinHandlers[builtinCount].body = cursor[0];
        builtinIds[builtinCount] = SCRIPT_FUNCTION_ID;
        builtinReturnTypes[builtinCount] = ReadType(&source);
        ReadWord(&source, builtinNames[builtinCount]);
        for (int j = 0; j < PARAMETER_LIMIT; j++)
            builtinParamTypes[builtinCount][j] = (char)0xff;
        strcpy(params, source);
        char *rest = params;
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
        builtinParamTypes[builtinCount][0] = (char)0xff;
        while (i < PARAMETER_LIMIT)
        {
            if (failed != 0)
                return 0;
            ReadTerm(&rest, word);
            builtinParamCounts[builtinCount] = (char)i;
            if (word[0] == 0)
                break;
            if (strcmp(word, "int") == 0)
                builtinParamTypes[builtinCount][i] = 0;
            else if (strcmp(word, "float") == 0)
                builtinParamTypes[builtinCount][i] = 1;
            else if (strcmp(word, "string") == 0)
                builtinParamTypes[builtinCount][i] = 2;
            else
            {
                builtinParamTypes[builtinCount][i] = 0;
                ReportError(BAD_PARAMETER_TYPE_TEXT);
            }
            for (;;)
            {
                ReadTerm(&rest, builtinParamNames[builtinCount][i]);
                if (builtinParamNames[builtinCount][i][0] == '&')
                {
                    builtinParamTypes[builtinCount][i] += BY_REFERENCE;
                    ReadTerm(&rest, builtinParamNames[builtinCount][i]);
                }
                if (builtinParamNames[builtinCount][i][0] == 0)
                {
                    ReportError(BAD_ARGUMENT_TEXT);
                    strcpy(builtinParamNames[builtinCount][i], UNNAMED_PARAMETER);
                    break;
                }
                if (strcmp(builtinParamNames[builtinCount][i], "int") != 0 &&
                    strcmp(builtinParamNames[builtinCount][i], "float") != 0 &&
                    strcmp(builtinParamNames[builtinCount][i], "string") != 0)
                    break;
                ReportError(BAD_ARGUMENT_TEXT);
                i++;
                if (strcmp(builtinParamNames[builtinCount][i], "int") == 0)
                    builtinParamTypes[builtinCount][i] = 0;
                else if (strcmp(builtinParamNames[builtinCount][i], "float") == 0)
                    builtinParamTypes[builtinCount][i] = 1;
                else if (strcmp(builtinParamNames[builtinCount][i], "string") == 0)
                    builtinParamTypes[builtinCount][i] = 2;
                strcpy(builtinParamNames[builtinCount][i - 1], UNNAMED_PARAMETER);
            }
            i++;
            builtinParamCounts[builtinCount] = (char)i;
            builtinParamTypes[builtinCount][i] = (char)0xff;
        }
    }
    for (int k = 0; k < builtinCount; k++)
    {
        if (strcmp(builtinNames[k], builtinNames[builtinCount]) == 0 &&
            memcmp(builtinParamTypes[builtinCount], builtinParamTypes[k], PARAMETER_LIMIT) == 0)
        {
            ReportError(DUPLICATE_FUNCTION_TEXT);
            return 0;
        }
    }
    builtinCount++;
    return 1;
}
