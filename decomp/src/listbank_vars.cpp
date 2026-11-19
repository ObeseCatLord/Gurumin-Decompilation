// Looking up declared script variables (array elements included). This unit sees ListBank::ReportError as the
// inline member it was in the original: one call site here is expanded, the other calls the out-of-line copy
// (0x100359c0, listbank.cpp).

#include <stdio.h>
#include <string.h>

typedef void (*ParserCallback)(int code, int value);

// The script parser (listbank.cpp ListBank), with the array dimension tables of the declared variables.
struct ListBank
{
    int *result;              // 0x00: where the last evaluated expression left its value
    char resultType;          // 0x04
    char pad_0005[3];
    ParserCallback callback;  // 0x08: message callback (may be 0)
    char names[8][0x80][0x10];// 0x0c: declared variable names per nesting level
    char m_400c[8][0x80];     // 0x400c: variable types (0 = int, 1 = float, 2 = text)
    int m_440c[8][0x100];
    int sizes[8][0x80][8];    // 0x640c: size of each array dimension
    char dimensions[8][0x80]; // 0xe40c: number of array dimensions (0 = not an array)
    char m_e80c[8][0x80];
    char pad_ec0c[0x1b9c0];
    int pad_2a5cc[4];
    int counts[8];            // 0x2a5dc: variables declared per nesting level
    char pad_2a5fc[0x4004];
    char text[0x7d00];        // 0x2e600: script text being parsed
    int textLength;           // 0x36300
    char *cursor[8];          // 0x36304: read position in `text`, one per nesting level
    int depth;                // 0x36324: current nesting level
    int pad_36328;
    int m_3632c;              // 0x3632c: set once an error was reported

    int GetIndexPart(const char *name, char *part, int n);    // 0x10036560
    int Evaluate(const char *expression, char type);         // 0x100caf90
    int FreeAll();                                            // 0x10007d50
    // 0x100359c0 (listbank.cpp): reports a parse error and frees the script buffers. Returns 0.
    int ReportError(const char *message)
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
    int FindVariable(const char *name, int *index, int *group, int *element);
};

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
            for (k = 0; k < counts[g]; k++)
            {
                if (m_3632c != 0)
                    return 0;
                if (strcmp(base, names[g][k]) == 0)
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
        for (k = 0; k < counts[g]; k++)
        {
            if (m_3632c != 0)
                return 0;
            if (strcmp(name, names[g][k]) == 0)
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
