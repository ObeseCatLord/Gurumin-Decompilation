// Highest id of a message key (EN build, EN-only message editor: no JP twin found).
#include "world_renderer.h"

// Result of HighestMessageId for a key without lines and without move commands.
const int HIGHEST_ID_NONE = -10000;
// Kinds of move command a key can hold (KeyCommandCount takes 0 .. MESSAGE_COMMAND_KINDS - 1).
const int MESSAGE_COMMAND_KINDS = 0x10;
// Result of FindMessageLine when the key has no lines.
const int HIGHEST_LINE_NOT_FOUND = -1;
// Variant handed to KeyIdOffset.
const int KEY_ID_OFFSET_VARIANT = 0;

// WorldRenderer::HighestMessageId: the largest id among the lines of key `key` and among the ids its move commands reach
// (each command's id offset added to the key's id offset); HIGHEST_ID_NONE when there are none. The result is cached for
// keys below MESSAGE_ID_CACHE_SIZE. Called by the message editor (EN 0x747cc0, 0x742020 and others).
int WorldRenderer::HighestMessageId(int key)
{
    if (key < MESSAGE_ID_CACHE_SIZE && highestIdCache[key] != MESSAGE_ID_CACHE_EMPTY)
        return highestIdCache[key];
    int first = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == HIGHEST_LINE_NOT_FOUND)
        end = messageCount;
    int highest = HIGHEST_ID_NONE;
    for (int line = first; line < end; line++)
    {
        if (messageIds[line] > highest)
            highest = messageIds[line];
    }
    int keyOffset = KeyIdOffset(key, KEY_ID_OFFSET_VARIANT);
    for (int kind = 0; kind < MESSAGE_COMMAND_KINDS; kind++)
    {
        int commandCount = KeyCommandCount(key, kind);
        for (int index = 0; index < commandCount; index++)
        {
            int reached = KeyCommandId(key, kind, index) + keyOffset;
            if (reached > highest)
                highest = reached;
        }
    }
    if (key < MESSAGE_ID_CACHE_SIZE)
        highestIdCache[key] = highest;
    return highest;
}
