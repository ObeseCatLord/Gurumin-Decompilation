// Editing the character's message lines (EN build, EN-only: no JP twin found).
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "world_renderer.h"

// Largest field text GetMessageNumber reads.
const int MESSAGE_FIELD_SIZE = 0x100;
// Field of a message line GetMessageNumber converts.
const int MESSAGE_NUMBER_FIELD = 1;

// WorldRenderer::GetMessageNumber: reads field MESSAGE_NUMBER_FIELD of the line that key `key` starts at and returns it
// as a number. No direct caller.
int WorldRenderer::GetMessageNumber(int key)
{
    char text[MESSAGE_FIELD_SIZE];
    int line = FindMessageLine(key);
    GetMessageField(line, MESSAGE_NUMBER_FIELD, text);
    return atol(text);
}

// WorldRenderer::AddMessageLine: inserts an empty line after the last line of key `key` (at the table end when it is the
// last key) and returns how many lines the key had before. No direct caller.
int WorldRenderer::AddMessageLine(int key)
{
    int first = FindMessageLine(key);
    int next = FindMessageLine(key + 1);
    if (next == -1)
        next = messageCount;
    InsertMessageLine(next);
    return next - first;
}

// WorldRenderer::SetKeyedMessage: stores `id` and `text` as line `offset` of key `key` (the line index is where the key
// starts plus `offset`). Returns 0. No direct caller.
int WorldRenderer::SetKeyedMessage(int key, int offset, int id, const char *text)
{
    int line = FindMessageLine(key) + offset;
    messageIds[line] = (short)id;
    strcpy(messages[line], text);
    return 0;
}

// WorldRenderer::DeleteMessageKeysWithIds: repeatedly looks for a key whose lowest id lies between `lowest` and
// `highest` (borders included) and deletes it, until none is left. Returns 0. No direct caller.
int WorldRenderer::DeleteMessageKeysWithIds(int lowest, int highest)
{
    int keyCount = CountMessageKeys();
    for (;;)
    {
        int key;
        for (key = 0; key < keyCount; key++)
        {
            int id = LowestMessageId(key);
            if (id >= lowest && id <= highest)
                break;
        }
        if (key >= keyCount)
            return 0;
        DeleteMessageKey(key);
        keyCount--;
    }
}

// Field of a message line that holds a key's number (the same one GetMessageNumber reads).
const int MESSAGE_NUMBER_FIELD_SET = 1;
// Length of the text a number is formatted into.
const int MESSAGE_NUMBER_TEXT_SIZE = 0x100;

// WorldRenderer::SetMessageNumber: writes `number` as text into the number field of the first line of key `key`.
// Returns `number`. No direct caller.
int WorldRenderer::SetMessageNumber(int key, int number)
{
    int line = FindMessageLine(key);
    char text[MESSAGE_NUMBER_TEXT_SIZE];
    sprintf(text, "%d", number);
    SetMessageField(line, MESSAGE_NUMBER_FIELD_SET, text);
    return number;
}

// WorldRenderer::ShiftIdsFrom: for every key whose highest id is at least `firstId`, shifts that key's ids by `shift`
// (ShiftKeyIds). Returns 0. No direct caller.
int WorldRenderer::ShiftIdsFrom(int firstId, int shift)
{
    int keyCount = CountMessageKeys();
    for (int key = 0; key < keyCount; key++)
    {
        int lowest = LowestMessageId(key);
        if (HighestMessageId(key) >= firstId)
            ShiftKeyIds(key, firstId, shift);
    }
    return 0;
}

// Field of a message line that holds its tag, and the tag GetKeyZimNumber looks for.
const int MESSAGE_TAG_FIELD = 0;
const char MESSAGE_ZIM_TAG[] = "ZIM";
// Characters of the field text GetKeyZimNumber reads.
const int ZIM_FIELD_SIZE = 0x80;
// Result of GetKeyZimNumber for a key without a usable ZIM line.
const int ZIM_NUMBER_NONE = -1;

// WorldRenderer::GetKeyZimNumber: goes through the lines of key `key` and returns the number in the second field of
// the first line whose first field is "ZIM" and whose number is not -1; -1 when there is none. No direct caller.
long WorldRenderer::GetKeyZimNumber(int key)
{
    char field[ZIM_FIELD_SIZE];
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == MESSAGE_LINE_NOT_FOUND)
        end = messageCount;
    for (int line = start; line < end; line++)
    {
        GetMessageField(line, MESSAGE_TAG_FIELD, field);
        if (strcmp(field, MESSAGE_ZIM_TAG) == 0)
        {
            GetMessageField(line, MESSAGE_NUMBER_FIELD, field);
            long number = atol(field);
            if (number != ZIM_NUMBER_NONE)
                return number;
        }
    }
    return ZIM_NUMBER_NONE;
}

// WorldRenderer::ShiftKeyIdsClamped: forgets the cached ids of key `key`, then adds `shift` to the id of every line of
// the key. A negative shift is shortened first so that no id goes below 0. Returns 0. No direct caller.
int WorldRenderer::ShiftKeyIdsClamped(int key, int shift)
{
    if (key < MESSAGE_ID_CACHE_SIZE)
    {
        lowestIdCache[key] = MESSAGE_ID_CACHE_EMPTY;
        lineTagIdCache[key] = MESSAGE_ID_CACHE_EMPTY;
        highestIdCache[key] = MESSAGE_ID_CACHE_EMPTY;
    }
    int first = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == MESSAGE_LINE_NOT_FOUND)
        end = messageCount;
    for (int line = first; line < end; line++)
    {
        if (messageIds[line] + shift < 0)
            shift = -messageIds[line];
    }
    for (int line = first; line < end; line++)
        messageIds[line] += (short)shift;
    return 0;
}
