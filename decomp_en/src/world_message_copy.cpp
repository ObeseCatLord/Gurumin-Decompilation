// Copying the message lines of one key into another world renderer (EN build, EN-only: no JP twin found).
#include <string.h>
#include "world_renderer.h"

// WorldRenderer::CopyKeyLinesTo: the lines from the first line of `key` up to the first line of the next key (or the end
// of the table) become the whole message table of `target`, ids included. The target's old lines are dropped.
int WorldRenderer::CopyKeyLinesTo(WorldRenderer *target, int key)
{
    int first = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == MESSAGE_LINE_NOT_FOUND)
        end = messageCount;
    target->messageCount = 0;
    for (int line = first; line < end; line++)
    {
        strcpy(target->messages[target->messageCount], messages[line]);
        target->messageIds[target->messageCount] = messageIds[line];
        target->messageCount++;
    }
    return 0;
}
