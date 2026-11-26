// Line and field access by message key (EN build, EN-only: no JP twin found).
#include "world_renderer.h"

// Field of a message line these functions read and write (0-based: the third one).
const int KEY_THIRD_FIELD = 2;

// WorldRenderer::RemoveKeyedMessage: removes line `offset` (counted from the first line of key `key`) from the table.
// Returns 0. No direct caller.
int WorldRenderer::RemoveKeyedMessage(int key, int offset)
{
    RemoveMessageLine(FindMessageLine(key) + offset);
    return 0;
}

// WorldRenderer::GetKeyThirdField: copies the third field of the first line of key `key` to `out`. Returns 0. No direct
// caller.
int WorldRenderer::GetKeyThirdField(int key, char *out)
{
    int line = FindMessageLine(key);
    GetMessageField(line, KEY_THIRD_FIELD, out);
    return 0;
}

// WorldRenderer::SetKeyThirdField: replaces the third field of the first line of key `key` with `text`. Returns 0. No
// direct caller.
int WorldRenderer::SetKeyThirdField(int key, const char *text)
{
    int line = FindMessageLine(key);
    SetMessageField(line, KEY_THIRD_FIELD, text);
    return 0;
}
