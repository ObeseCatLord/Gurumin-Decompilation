// Looking up keys and fields of the character's message lines (EN build, EN-only message editor: no JP twin found).
#include <stdlib.h>
#include <string.h>
#include "message_fields.h"
#include "world_renderer.h"

// Result of the finders and scanners when nothing qualifies.
const int LOOKUP_NONE = -1;
// First field of a message line: the command tag, or the separator that starts a key.
const int LINE_TAG_FIELD = 0;
// Text of the first field that starts a new key.
const char KEY_SEPARATOR[] = "---";
// Fields FindLineField looks at (fields 1 to 6 of the line).
const int LINE_FIELD_SCAN_COUNT = 6;
// Characters of a command tag.
const int LINE_TAG_LENGTH = 3;
// Largest field text the key lookups read, and the scanners and FindLineField.
const int LOOKUP_FIELD_SIZE = 0x100;
const int SCAN_FIELD_SIZE = 0x80;

// WorldRenderer::FindMessageLine: the line number at which key `key` starts, that is the line of the `key`-th line whose
// first field is "---" (counting from 0); -1 when there are fewer keys. Called by every message editor function.
int WorldRenderer::FindMessageLine(int key)
{
    char field[LOOKUP_FIELD_SIZE];
    int separators = 0;
    for (int line = 0; line < messageCount; line++)
    {
        MessageLine_CopyField(messages[line], LINE_TAG_FIELD, field);
        if (strcmp(field, KEY_SEPARATOR) == 0)
        {
            if (key == separators)
                return line;
            separators++;
        }
    }
    return LOOKUP_NONE;
}

// WorldRenderer::CountMessageKeys: the number of lines whose first field is "---", counted once and then cached in
// m_03860 (WORLD_ID_EMPTY = not counted yet). Called by the message editor.
int WorldRenderer::CountMessageKeys()
{
    if (m_03860 != MESSAGE_ID_CACHE_EMPTY)
        return m_03860;
    char field[LOOKUP_FIELD_SIZE];
    int keyCount = 0;
    for (int line = 0; line < messageCount; line++)
    {
        MessageLine_CopyField(messages[line], LINE_TAG_FIELD, field);
        if (strcmp(field, KEY_SEPARATOR) == 0)
            keyCount++;
    }
    m_03860 = keyCount;
    return keyCount;
}

// WorldRenderer::FindLineField: looks through fields 1 to 6 of line `line` for one equal to `text`. Returns 1 and the
// number of that field in *fieldOut (when given), or 0 when none is equal. Called by the message editor.
int WorldRenderer::FindLineField(int line, const char *text, int *fieldOut)
{
    char field[SCAN_FIELD_SIZE];
    int number = 0;
    for (int index = 0; index < LINE_FIELD_SCAN_COUNT; index++)
    {
        MessageLine_CopyField(messages[line], index + 1, field);
        if (strcmp(field, text) == 0)
        {
            number = index + 1;
            break;
        }
    }
    if (number > 0)
    {
        if (fieldOut != 0)
            *fieldOut = number;
        return 1;
    }
    return 0;
}

// Field of a message line that follows the command tag, and the second, third and fourth numbers after it.
const int LINE_FIELD_SECOND = 1;
const int LINE_FIELD_THIRD = 2;
const int LINE_FIELD_FOURTH = 3;
const int LINE_FIELD_FIFTH = 4;
const int LINE_FIELD_SIXTH = 5;

// WorldRenderer::GetKeyZimSp2Pair: looks through the lines of key `key` for the first one that starts with "ZIM" or
// "SP2". A "ZIM" line gives the numbers in its fields 3 and 4, an "SP2" line the numbers in its fields 2 and 3; they
// are stored in out[0] and out[1]. Returns 1 when such a line exists, else 0. Called by the message editor.
int WorldRenderer::GetKeyZimSp2Pair(int key, float *out)
{
    char field[SCAN_FIELD_SIZE];
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == LOOKUP_NONE)
        end = messageCount;
    for (int line = start; line < end; line++)
    {
        if (memcmp(messages[line], "ZIM", LINE_TAG_LENGTH) == 0)
        {
            GetMessageField(line, LINE_FIELD_FOURTH, field);
            out[0] = (float)atof(field);
            GetMessageField(line, LINE_FIELD_FIFTH, field);
            out[1] = (float)atof(field);
            return 1;
        }
        if (memcmp(messages[line], "SP2", LINE_TAG_LENGTH) == 0)
        {
            GetMessageField(line, LINE_FIELD_THIRD, field);
            out[0] = (float)atof(field);
            GetMessageField(line, LINE_FIELD_FOURTH, field);
            out[1] = (float)atof(field);
            return 1;
        }
    }
    return 0;
}

// WorldRenderer::GetKeyRefTriple: looks through the lines of key `key` for the first one that starts with "REF", "RE2",
// "SEF", "SP3" or "CS4" and stores the numbers in its fields 3, 4 and 5 in out[0], out[1] and out[2]. Returns 1 when
// such a line exists, else 0. Called by the message editor.
int WorldRenderer::GetKeyRefTriple(int key, float *out)
{
    char field[SCAN_FIELD_SIZE];
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == LOOKUP_NONE)
        end = messageCount;
    for (int line = start; line < end; line++)
    {
        if (memcmp(messages[line], "REF", LINE_TAG_LENGTH) == 0 || memcmp(messages[line], "RE2", LINE_TAG_LENGTH) == 0 ||
            memcmp(messages[line], "SEF", LINE_TAG_LENGTH) == 0 || memcmp(messages[line], "SP3", LINE_TAG_LENGTH) == 0 ||
            memcmp(messages[line], "CS4", LINE_TAG_LENGTH) == 0)
        {
            GetMessageField(line, LINE_FIELD_THIRD, field);
            out[0] = (float)atof(field);
            GetMessageField(line, LINE_FIELD_FOURTH, field);
            out[1] = (float)atof(field);
            GetMessageField(line, LINE_FIELD_FIFTH, field);
            out[2] = (float)atof(field);
            return 1;
        }
    }
    return 0;
}

// WorldRenderer::ScanLineCommandD: when line `line` starts with "RLK", "ALK" or "RLS", returns the number in its second
// field (and stores that field's number in *position when given); -1 for any other line. Called by FindKeyCommandD.
int WorldRenderer::ScanLineCommandD(int line, int *position)
{
    char field[SCAN_FIELD_SIZE];
    int valueField = 0;
    GetMessageField(line, LINE_TAG_FIELD, field);
    if (strcmp(field, "RLK") == 0)
        valueField = LINE_FIELD_SECOND;
    if (strcmp(field, "ALK") == 0)
        valueField = LINE_FIELD_SECOND;
    if (strcmp(field, "RLS") == 0)
        valueField = LINE_FIELD_SECOND;
    if (valueField > 0)
    {
        GetMessageField(line, valueField, field);
        if (position != 0)
            *position = valueField;
        return atoi(field);
    }
    return LOOKUP_NONE;
}

// WorldRenderer::ScanLineCommandH: as ScanLineCommandD for the tags "CFD", "BLS" and "BLK". Called by FindKeyCommandH.
int WorldRenderer::ScanLineCommandH(int line, int *position)
{
    char field[SCAN_FIELD_SIZE];
    int valueField = 0;
    GetMessageField(line, LINE_TAG_FIELD, field);
    if (strcmp(field, "CFD") == 0)
        valueField = LINE_FIELD_SECOND;
    if (strcmp(field, "BLS") == 0)
        valueField = LINE_FIELD_SECOND;
    if (strcmp(field, "BLK") == 0)
        valueField = LINE_FIELD_SECOND;
    if (valueField > 0)
    {
        GetMessageField(line, valueField, field);
        if (position != 0)
            *position = valueField;
        return atoi(field);
    }
    return LOOKUP_NONE;
}
