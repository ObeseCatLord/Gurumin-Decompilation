// Comma separated fields of the character's message lines (EN build; JP twin: the field reader of decomp/src/text_fields.cpp).
#include <string.h>
#include "world_renderer.h"

// Characters of the buffer a field is collected in, including the terminator.
const int MESSAGE_FIELD_BUFFER_SIZE = 128;
// Returned by GetMessageField when the line has fewer fields than asked for, and when it found the field.
const int MESSAGE_FIELD_MISSING = -1;
const int MESSAGE_FIELD_FOUND = 1;

// WorldRenderer::GetMessageField: copies field number `field` (0-based; fields end at a comma, a line end or the end
// of the line) of line `line` into `out`. Returns MESSAGE_FIELD_FOUND, or MESSAGE_FIELD_MISSING when the line has fewer
// fields. Called by the message editor and the script commands.
int WorldRenderer::GetMessageField(int line, int field, char *out)
{
    char buffer[MESSAGE_FIELD_BUFFER_SIZE];
    int length = 0;
    int count = 0;
    int size = strlen(messages[line]) + 2;
    for (int i = 0; i < size; i++)
    {
        char c = messages[line][i];
        if (c == 0 || c == '\n' || c == '\r' || c == ',')
        {
            if (count == field)
            {
                strcpy(out, buffer);
                return MESSAGE_FIELD_FOUND;
            }
            count++;
            length = 0;
        }
        else
        {
            buffer[length] = c;
            buffer[length + 1] = 0;
            length++;
        }
    }
    return MESSAGE_FIELD_MISSING;
}
