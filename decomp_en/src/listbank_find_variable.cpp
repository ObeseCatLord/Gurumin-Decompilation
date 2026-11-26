// Looking up declared script variables, array elements included (EN build). Ported from the JP source
// decomp/src/listbank_vars.cpp (APP.dll 0x10037090, unmatched there) and the EN bytes. This unit sees
// ListBank::ReportError as the inline member it was in the original.
#include <stdio.h>
#include <string.h>

#include "listbank.h"

// Error: "the index is out of range" (Shift-JIS).
#define INDEX_RANGE_TEXT "\x93\x59\x82\xa6\x8e\x9a\x82\xcc\x94\xcd\x88\xcd\x82\xf0\x92\xb4\x82\xa6\x82\xc4\x82\xa2\x82\xdc\x82\xb7\x81\x42"
// Error: "the number of indexes is wrong" (Shift-JIS).
#define INDEX_DIMENSION_TEXT "\x93\x59\x82\xa6\x8e\x9a\x82\xcc\x8e\x9f\x8c\xb3\x82\xaa\x88\xe1\x82\xa2\x82\xdc\x82\xb7\x81\x42"
// Most index expressions a variable reference can have.
#define MAX_INDEX_COUNT 8

// Finds the declared variable `name` from the current nesting level outwards and stores its level (`group`), its
// slot (`index`) and the element offset (`element`). For an array reference such as "name[i][j]" every index
// expression is evaluated; their count must match the variable's dimensions and each must lie inside its dimension.
// The element offset is the last index times the product of the dimensions after it (only the last one is kept).
// A plain name must not be an array. Returns 1 when found, 0 otherwise or after an error. Called by the expression
// evaluator and the assignment code of the script parser.
int ListBank::FindVariable(const char *name, int *index, int *group, int *element)
{
    char base[READ_BUFFER_LIMIT];
    char part[READ_BUFFER_LIMIT];
    int indices[MAX_INDEX_COUNT];
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
        int found = GetIndexPart(base, part, 0);
        int count = 0;
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
