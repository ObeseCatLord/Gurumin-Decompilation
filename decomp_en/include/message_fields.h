// Inline copy of the message line field reader. The original compiler built WorldRenderer::GetMessageField (EN 0x733400)
// into some of its callers (FindMessageLine, CountMessageKeys, FindLineField) and called it from others; the inlining
// callers include this header instead of calling the out-of-line method. Declared once here.
#ifndef GURUMIN_EN_MESSAGE_FIELDS_H
#define GURUMIN_EN_MESSAGE_FIELDS_H

#include <string.h>

// Characters of the buffer a field is collected in, including the terminator.
const int MESSAGE_LINE_FIELD_BUFFER_SIZE = 128;

// Copies field number `field` (0-based; fields end at a comma, a line end or the end of the line) of the message line
// `line` into `out`. Returns 1, or -1 when the line has fewer fields. Same code as WorldRenderer::GetMessageField.
inline int MessageLine_CopyField(const char *line, int field, char *out)
{
    char buffer[MESSAGE_LINE_FIELD_BUFFER_SIZE];
    int length = 0;
    int count = 0;
    int size = strlen(line);
    for (int i = 0; i < size + 2; i++)
    {
        char c = line[i];
        if (c == 0 || c == '\n' || c == '\r' || c == ',')
        {
            if (count == field)
            {
                strcpy(out, buffer);
                return 1;
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
    return -1;
}

#endif
