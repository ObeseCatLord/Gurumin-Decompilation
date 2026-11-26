// Message line table of the world: lowest id of a key and removing a line (EN build, EN-only: no JP twin found).
#include <string.h>
#include "world_renderer.h"

// Result of LowestMessageId for a key without lines.
const int LOWEST_ID_NONE = 10000;
// Value of an id cache entry that has not been worked out yet.
const int ID_CACHE_EMPTY = -9999;
// Index of the last line of the message table (the table holds 2400 lines).
const int MESSAGE_LAST_LINE = 2399;
// Id of a line that is not in use.
const int MESSAGE_ID_UNUSED = -1;

// WorldRenderer::LowestMessageId: the smallest id among the lines of key `key`, LOWEST_ID_NONE when it has no lines. The
// result is cached for keys below MESSAGE_ID_CACHE_SIZE. Called by the message editor (EN 0x71cc00 and others).
int WorldRenderer::LowestMessageId(int key)
{
    if (key < MESSAGE_ID_CACHE_SIZE && lowestIdCache[key] != ID_CACHE_EMPTY)
        return lowestIdCache[key];
    int first = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == -1)
        end = messageCount;
    int lowest = LOWEST_ID_NONE;
    for (int line = first; line < end; line++)
    {
        if (messageIds[line] < lowest)
            lowest = messageIds[line];
    }
    if (key < MESSAGE_ID_CACHE_SIZE)
        lowestIdCache[key] = lowest;
    return lowest;
}

// WorldRenderer::RemoveMessageLine: takes line `line` out of the table: every later line moves up one place (id and
// text), the last line is emptied and the line count drops by one. Returns 0. Called by the key deletion at EN 0x71ca80
// and the message editor.
int WorldRenderer::RemoveMessageLine(int line)
{
    int last = line;
    for (; last < MESSAGE_LAST_LINE; last++)
    {
        messageIds[last] = messageIds[last + 1];
        strcpy(messages[last], messages[last + 1]);
    }
    messageIds[last] = MESSAGE_ID_UNUSED;
    messages[last][0] = 0;
    messageCount--;
    return 0;
}
