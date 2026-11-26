// Editing the character's message lines (EN build, EN-only: no JP twin found).
#include <stdlib.h>
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
