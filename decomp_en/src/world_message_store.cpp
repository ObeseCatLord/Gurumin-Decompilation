// Appending and saving the message lines of the world (EN build, EN-only message editor: no JP twin found).
#include <stdio.h>
#include <string.h>
#include "game_file.h"
#include "world_renderer.h"

// Result of FindMessageLine when the key has no lines.
const int STORE_LINE_NOT_FOUND = -1;
// Size of the buffer one saved line is formatted into.
const int SAVED_LINE_SIZE = 0x200;

// WorldRenderer::AppendKeyLinesFrom: adds the lines of key `key` of `source` (ids included) to the end of this table,
// then forgets the cached lowest, tagged and highest ids of every key and the key count. The key count is worked out
// (CountMessageKeys) before the lines are added. Returns 0. Called by the message editor.
int WorldRenderer::AppendKeyLinesFrom(WorldRenderer *source, int key)
{
    int first = source->FindMessageLine(key);
    int end = source->FindMessageLine(key + 1);
    if (end == STORE_LINE_NOT_FOUND)
        end = source->messageCount;
    CountMessageKeys();
    for (int line = first; line < end; line++)
    {
        strcpy(messages[messageCount], source->messages[line]);
        messageIds[messageCount] = source->messageIds[line];
        messageCount++;
    }
    for (int cached = 0; cached < MESSAGE_ID_CACHE_SIZE; cached++)
    {
        lowestIdCache[cached] = MESSAGE_ID_CACHE_EMPTY;
        lineTagIdCache[cached] = MESSAGE_ID_CACHE_EMPTY;
        highestIdCache[cached] = MESSAGE_ID_CACHE_EMPTY;
    }
    m_03860 = MESSAGE_ID_CACHE_EMPTY;
    return 0;
}

// WorldRenderer::SaveMessages: writes the table to the file `path` as one "##nnnn,text" line per entry (the id with four
// digits, then the text, then CR LF). Does nothing when the file cannot be created. Returns 0. Called by the message
// editor.
int WorldRenderer::SaveMessages(const char *path)
{
    GameFile file;
    if (file.Open(path, "wb"))
    {
        for (int line = 0; line < messageCount; line++)
        {
            char text[SAVED_LINE_SIZE];
            sprintf(text, "##%04d,%s\r\n", messageIds[line], messages[line]);
            file.Write(text, 1, strlen(text));
        }
        file.Close();
    }
    return 0;
}
