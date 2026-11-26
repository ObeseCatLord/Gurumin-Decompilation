// EN script parser (ListBank): number parsing and a text shift helper. JP twins: ParseInteger and ParseFloat in
// decomp/src/listbank.cpp; the shift helper is EN-only.
#include <stdlib.h>
#include <string.h>
#include "listbank.h"

// Number of 16-bit characters of the buffer ShiftCharactersLeft works on.
#define SHIFT_BUFFER_CHARACTERS 0x80

// Parses `text` as an integer when it only holds digits and signs: stores it in `value` and returns 1, else 0 (also
// 0 once a parse error was reported). Called by the statement compiler.
int ListBank::ParseInteger(const char *text, int *value)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        if (failed != 0)
            return 0;
        if (text[i] != '+' && text[i] != '-' && (text[i] < '0' || text[i] > '9'))
            return 0;
    }
    *value = atol(text);
    return 1;
}

// Parses `text` as a float when it only holds digits, signs and a decimal point: stores it in `value` and returns 1,
// else 0. Called by the statement compiler.
int ListBank::ParseFloat(const char *text, float *value)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        if (failed != 0)
            return 0;
        if (text[i] != '+' && text[i] != '-' && text[i] != '.' && (text[i] < '0' || text[i] > '9'))
            return 0;
    }
    *value = (float)atof(text);
    return 1;
}

// Moves the characters of `buffer` from `start` up to the end of its 0x80 characters `shift` places towards the
// front (the tail is read from `shift` places ahead). Stops early once a parse error was reported. Returns 0. No
// direct caller.
int ListBank::ShiftCharactersLeft(short *buffer, int start, int shift)
{
    if (failed != 0)
        return 0;
    int limit = SHIFT_BUFFER_CHARACTERS - shift;
    for (int i = start; i < limit; i++)
    {
        if (failed != 0)
            return 0;
        buffer[i] = buffer[i + shift];
    }
    return 0;
}
