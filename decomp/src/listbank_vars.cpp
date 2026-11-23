// Looking up declared script variables (array elements included). This unit sees ListBank::ReportError as the
// inline member it was in the original: one call site here is expanded, the other calls the out-of-line copy
// (0x100359c0, listbank.cpp).

#include <stdio.h>
#include <string.h>

#include "listbank.h"
// "添え字の範囲を超えています。" ("The index is out of range."), Shift-JIS.
#define INDEX_RANGE_TEXT "\x93\x59\x82\xa6\x8e\x9a\x82\xcc\x94\xcd\x88\xcd\x82\xf0\x92\xb4\x82\xa6\x82\xc4\x82\xa2\x82\xdc\x82\xb7\x81\x42"
// "添え字の次元が違います。" ("The number of indexes is wrong."), Shift-JIS.
#define INDEX_DIMENSION_TEXT "\x93\x59\x82\xa6\x8e\x9a\x82\xcc\x8e\x9f\x8c\xb3\x82\xaa\x88\xe1\x82\xa2\x82\xdc\x82\xb7\x81\x42"

// 0x10037090 (ListBank::FindVariable). Finds the declared variable `name` from the current nesting level outwards
// and stores its level (`group`), its slot (`index`) and the element offset (`element`). For an array reference
// such as "name[i][j]" every index expression is evaluated; their count must match the variable's dimensions and
// each must lie inside its dimension. The element offset is the last index times the product of the dimensions
// after it (only the last one is kept). A plain name must not be an array. Returns 1 when found, 0 otherwise or
// after an error.
int ListBank::FindVariable(const char *name, int *index, int *group, int *element)
{
    char base[0x100];
    char part[0x100];
    int indices[8];
    int bracket = 0;
    int i;
    for (i = (int)strlen(name) - 1; i > 0; i--)
    {
        if (name[i] == '[')
            bracket = 1;
    }
    int g;
    int k;
    if (bracket != 0)
    {
        strcpy(base, name);
        int count = 0;
        int found = GetIndexPart(base, part, 0);
        while (found != 0)
        {
            Evaluate(part, 0);
            indices[count] = *result;
            count++;
            found = GetIndexPart(base, part, count);
        }
        for (i = 0; base[i] != '['; i++)
            ;
        base[i] = 0;
        for (g = depth; g >= 0; g--)
        {
            for (k = 0; k < variableCounts[g]; k++)
            {
                if (failed != 0)
                    return 0;
                if (strcmp(base, variableNames[g][k]) == 0)
                {
                    *index = k;
                    *group = g;
                    *element = 0;
                    if (dimensions[g][k] != count)
                        return ReportError(INDEX_DIMENSION_TEXT);
                    for (int d = 0; d < dimensions[g][k]; d++)
                    {
                        if (indices[d] >= sizes[g][k][d])
                            return ReportError(INDEX_RANGE_TEXT);
                        int stride = 1;
                        for (int e = dimensions[g][k] - 1; e > d; e--)
                            stride *= sizes[g][k][e];
                        *element = indices[d] * stride;
                    }
                    return 1;
                }
            }
        }
        return 0;
    }
    for (g = depth; g >= 0; g--)
    {
        for (k = 0; k < variableCounts[g]; k++)
        {
            if (failed != 0)
                return 0;
            if (strcmp(name, variableNames[g][k]) == 0)
            {
                if (dimensions[g][k] == 0)
                {
                    *element = 0;
                    *index = k;
                    *group = g;
                    return 1;
                }
                return ReportError(INDEX_DIMENSION_TEXT);
            }
        }
    }
    return 0;
}

// "型が異常です。" ("The type is wrong."), Shift-JIS.
#define BAD_TYPE_TEXT "^ªÙíÅ·B"
// "関数名が重複してます。" ("The function name is a duplicate."), Shift-JIS.
#define DUPLICATE_NAME_TEXT "Ö¼ªd¡µÄÜ·B"
// "対応する括弧がありません。" ("There is no matching bracket."), Shift-JIS.
#define NO_BRACKET_TEXT "Î·éÊª èÜ¹ñB"

// 0x10036ca0 (ListBank::RegisterBuiltin, Script_RegisterBuiltin). Registers a builtin script function from its
// C-like signature, such as "int name(int, &float, string)": the return type, the name and up to 16 parameter types
// ("&" before a type adds 10: passed by reference) with the handler and its id. A name already registered with the
// same parameter types, an unknown type or missing brackets is reported (the latter also clears the last result).
// Returns 1 when registered, 0 otherwise.
int ListBank::RegisterBuiltin(const char *signature, short id, BuiltinHandler handler)
{
    char params[0x100];
    char word[0x100];
    builtinHandlers[builtinCount].handler = handler;
    char *cursor = (char *)signature;
    builtinIds[builtinCount] = id;
    builtinReturnTypes[builtinCount] = ReadType(&cursor);
    ReadWord(&cursor, builtinNames[builtinCount]);
    for (int j = 0; j < 0x10; j++)
        builtinParamTypes[builtinCount][j] = (char)0xff;
    strcpy(params, cursor);
    cursor = params;
    if (params[0] != '(')
        ReportError(NO_BRACKET_TEXT);
    else
    {
        params[0] = ' ';
        if (params[strlen(params) - 1] == ')')
        {
            params[strlen(params) - 1] = ' ';
            int i = 0;
            builtinParamTypes[builtinCount][0] = (char)0xff;
            do
            {
                if (failed != 0)
                    return 0;
                ReadTerm(&cursor, word);
                builtinParamTypes[builtinCount][i] = 0;
                if (word[0] == '&')
                {
                    builtinParamTypes[builtinCount][i - 1] += 10;
                    ReadTerm(&cursor, word);
                }
                builtinParamCounts[builtinCount] = (char)i;
                if (word[0] == 0)
                    break;
                if (strcmp(word, "int") != 0)
                {
                    if (strcmp(word, "float") == 0)
                        builtinParamTypes[builtinCount][i] += 1;
                    else if (strcmp(word, "string") == 0)
                        builtinParamTypes[builtinCount][i] += 2;
                    else
                    {
                        builtinParamTypes[builtinCount][i] = 0;
                        ReportError(BAD_TYPE_TEXT);
                    }
                }
                i++;
                builtinParamCounts[builtinCount] = (char)i;
                builtinParamTypes[builtinCount][i] = (char)0xff;
            } while (i < 0x10);
            for (int k = 0; k < builtinCount; k++)
            {
                if (strcmp(builtinNames[k], builtinNames[builtinCount]) == 0 &&
                    memcmp(builtinParamTypes[builtinCount], builtinParamTypes[k], 0x10) == 0)
                    return ReportError(DUPLICATE_NAME_TEXT);
            }
            builtinCount++;
            return 1;
        }
        ReportError(NO_BRACKET_TEXT);
        self->resultType = 0;
        *self->result = 0;
        return 0;
    }
    self->resultType = 0;
    *self->result = 0;
    return 0;
}
