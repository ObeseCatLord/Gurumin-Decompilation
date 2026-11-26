// Finding message lines by their fields and by their command tag (EN build, EN-only message editor: no JP twin found).
#include <string.h>
#include "world_renderer.h"

// Result of the finders when no line qualifies.
const int FIND_NONE = -1;
// Fields of a message line that the field finders compare.
const int MESSAGE_FIELD_FIRST = 0;
const int MESSAGE_FIELD_SECOND = 1;
const int MESSAGE_FIELD_THIRD = 2;
// Characters of the command tag that starts a message line.
const int MESSAGE_COMMAND_TAG_LENGTH = 3;
// Results of WorldRenderer::KeyMoveCommandKind.
const int MOVE_COMMAND_KIND_CAMERA = 1;
const int MOVE_COMMAND_KIND_GROUND = 2;

// WorldRenderer::FindKeyLineOffset: the position, counted from the first line of key `key`, of the first line of the key
// whose first three comma-separated fields equal `first`, `second` and `third` (ignoring case); a null string matches
// any text. Returns -1 when no line fits. Called by the message editor.
int WorldRenderer::FindKeyLineOffset(int key, const char *first, const char *second, const char *third)
{
    char field[0x100];
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == FIND_NONE)
        end = messageCount;
    int offset = 0;
    for (; line < end; line++)
    {
        GetMessageField(line, MESSAGE_FIELD_FIRST, field);
        if (first == 0 || stricmp(first, field) == 0)
        {
            GetMessageField(line, MESSAGE_FIELD_SECOND, field);
            if (second == 0 || stricmp(second, field) == 0)
            {
                GetMessageField(line, MESSAGE_FIELD_THIRD, field);
                if (third == 0 || stricmp(third, field) == 0)
                    return offset;
            }
        }
        offset++;
    }
    return FIND_NONE;
}

// WorldRenderer::FindKeyLine: as FindKeyLineOffset but returns the line number in the whole table.
int WorldRenderer::FindKeyLine(int key, const char *first, const char *second, const char *third)
{
    char field[0x100];
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == FIND_NONE)
        end = messageCount;
    for (; line < end; line++)
    {
        GetMessageField(line, MESSAGE_FIELD_FIRST, field);
        if (first == 0 || stricmp(first, field) == 0)
        {
            GetMessageField(line, MESSAGE_FIELD_SECOND, field);
            if (second == 0 || stricmp(second, field) == 0)
            {
                GetMessageField(line, MESSAGE_FIELD_THIRD, field);
                if (third == 0 || stricmp(third, field) == 0)
                    return line;
            }
        }
    }
    return FIND_NONE;
}

// WorldRenderer::KeyMoveCommandKind: looks through the lines of key `key` and returns MOVE_COMMAND_KIND_CAMERA when a
// line begins with "RCM", "RCA", "RM2" or "RA2", MOVE_COMMAND_KIND_GROUND for "RGM", "RGA", "RMT" or "RAT" (the first
// line that is any of them decides), and 0 when there is none. No direct caller.
int WorldRenderer::KeyMoveCommandKind(int key)
{
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == FIND_NONE)
        end = messageCount;
    for (int line = start; line < end; line++)
    {
        if (memcmp(messages[line], "RCM", MESSAGE_COMMAND_TAG_LENGTH) == 0 || memcmp(messages[line], "RCA", MESSAGE_COMMAND_TAG_LENGTH) == 0 ||
            memcmp(messages[line], "RM2", MESSAGE_COMMAND_TAG_LENGTH) == 0 || memcmp(messages[line], "RA2", MESSAGE_COMMAND_TAG_LENGTH) == 0)
            return MOVE_COMMAND_KIND_CAMERA;
        if (memcmp(messages[line], "RGM", MESSAGE_COMMAND_TAG_LENGTH) == 0 || memcmp(messages[line], "RGA", MESSAGE_COMMAND_TAG_LENGTH) == 0 ||
            memcmp(messages[line], "RMT", MESSAGE_COMMAND_TAG_LENGTH) == 0 || memcmp(messages[line], "RAT", MESSAGE_COMMAND_TAG_LENGTH) == 0)
            return MOVE_COMMAND_KIND_GROUND;
    }
    return 0;
}
