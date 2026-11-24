// EN script parser: expression evaluation and library setup. JP twin: decomp/src/listbank.cpp.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listbank.h"

// Compiles `expression` (its value lands in *result, of type resultType) and converts the value to `type`:
// 0 = int (a float is truncated, text is read with atol), 1 = float (text with atof), 2 = quoted text ("%d" or
// "%f" in quotes). An unknown result type gives 0, or "NULL" in quotes for text. Returns 1. Called by the script
// runner whenever it needs a value of a known type.
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

// Number of keyword slots the init clears (FindKeyword only searches the first 16).
#define KEYWORD_SLOTS 0x20

// Resets the parser (no text, no blocks, no names, no keywords, empty groups; results go to the scratch value
// resultBuffer) and registers the library builtins sin ... strcmp. The names declared so far count as global
// ones. Returns 0. Called once at startup by the game's script setup.
int ListBank::InitLibrary()
{
    result = resultBuffer;
    self = this;
    callback = 0;
    flow = 0;
    blockCount = 0;
    compiling = 0;
    textLength = 0;
    depth = 0;
    failed = 0;
    builtinCount = 0;
    for (int i = 0; i < KEYWORD_SLOTS; i++)
        m_36334[i][0] = 0;
    for (int group = 0; group < LISTBANK_GROUP_COUNT; group++)
    {
        variableCounts[group] = 0;
        for (int k = 0; k < LISTBANK_GROUP_ENTRIES; k++)
            values[group][k].storage = 0;
    }
    RegisterBuiltin("float sin(float)", BUILTIN_SIN, RunLibraryBuiltin);
    RegisterBuiltin("float cos(float)", BUILTIN_COS, RunLibraryBuiltin);
    RegisterBuiltin("float tan(float)", BUILTIN_TAN, RunLibraryBuiltin);
    RegisterBuiltin("float atan(float)", BUILTIN_ATAN, RunLibraryBuiltin);
    RegisterBuiltin("float atan2(float,float)", BUILTIN_ATAN2, RunLibraryBuiltin);
    RegisterBuiltin("float sqrt(float)", BUILTIN_SQRT, RunLibraryBuiltin);
    RegisterBuiltin("int rand()", BUILTIN_RAND, RunLibraryBuiltin);
    RegisterBuiltin("int abs(int)", BUILTIN_ABS, RunLibraryBuiltin);
    RegisterBuiltin("float fabs(float)", BUILTIN_FABS, RunLibraryBuiltin);
    RegisterBuiltin("float itof(int)", BUILTIN_ITOF, RunLibraryBuiltin);
    RegisterBuiltin("int ftoi(float)", BUILTIN_FTOI, RunLibraryBuiltin);
    RegisterBuiltin("float asin(float)", BUILTIN_ASIN, RunLibraryBuiltin);
    RegisterBuiltin("float acos(float)", BUILTIN_ACOS, RunLibraryBuiltin);
    RegisterBuiltin("float sinh(float)", BUILTIN_SINH, RunLibraryBuiltin);
    RegisterBuiltin("float cosh(float)", BUILTIN_COSH, RunLibraryBuiltin);
    RegisterBuiltin("float tanh(float)", BUILTIN_TANH, RunLibraryBuiltin);
    RegisterBuiltin("float pow(float,float)", BUILTIN_POW, RunLibraryBuiltin);
    RegisterBuiltin("float ceil(float)", BUILTIN_CEIL, RunLibraryBuiltin);
    RegisterBuiltin("float floor(float)", BUILTIN_FLOOR, RunLibraryBuiltin);
    RegisterBuiltin("float exp(float)", BUILTIN_EXP, RunLibraryBuiltin);
    RegisterBuiltin("float log(float)", BUILTIN_LOG, RunLibraryBuiltin);
    RegisterBuiltin("float log10(float)", BUILTIN_LOG10, RunLibraryBuiltin);
    RegisterBuiltin("float _max(float,float)", BUILTIN_MAX, RunLibraryBuiltin);
    RegisterBuiltin("float _min(float,float)", BUILTIN_MIN, RunLibraryBuiltin);
    RegisterBuiltin("int strcmp(string,string)", BUILTIN_STRCMP, RunLibraryBuiltin);
    globalBuiltinCount = builtinCount;
    return 0;
}
