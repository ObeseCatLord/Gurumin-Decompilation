// The "ROP"/"AOP" command scanner of the message editor (EN build, EN-only: no JP twin found).
#include <stdlib.h>
#include <string.h>
#include "world_renderer.h"

// Result of the scanner for a line that is not an ROP/AOP command.
const int SCAN_NOT_FOUND = -1;
// Size of the buffer a field is read into.
const int SCAN_FIELD_SIZE = 0x80;
// Field index of the number an ROP/AOP command carries.
const int ROP_NUMBER_FIELD = 1;
// Field the command tag is read from.
const int SCAN_TAG_FIELD = 0;

// WorldRenderer::ScanLineCommandI: when line `line` is an "ROP" or "AOP" command, returns the number in its second field
// and stores that field's index in `*position` (when given); returns -1 for any other line. Called by FindKeyCommandI.
int WorldRenderer::ScanLineCommandI(int line, int *position)
{
    char field[SCAN_FIELD_SIZE];
    int fieldIndex = 0;
    GetMessageField(line, SCAN_TAG_FIELD, field);
    if (strcmp(field, "ROP") == 0)
        fieldIndex = ROP_NUMBER_FIELD;
    if (strcmp(field, "AOP") != 0)
    {
        if (fieldIndex <= 0)
            return SCAN_NOT_FOUND;
    }
    else
        fieldIndex = ROP_NUMBER_FIELD;
    GetMessageField(line, fieldIndex, field);
    if (position != 0)
        *position = fieldIndex;
    return atol(field);
}
