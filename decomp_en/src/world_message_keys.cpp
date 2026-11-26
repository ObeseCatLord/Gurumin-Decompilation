// Message-line keys of the world: tag tests, per-key loops and line insertion (EN build, EN-only: no JP twin found).
#include <stdio.h>
#include <string.h>
#include "world_renderer.h"

// Number of line slots of the message table (WorldRenderer::messages).
const int MESSAGE_LINE_COUNT = 2400;
// Characters of a command tag that starts a message line.
const int MESSAGE_TAG_LENGTH = 3;
// Id of an empty line.
const int MESSAGE_EMPTY_ID = -1;
// Returned by FindMessageLine and the scanners when there is no line.
const int KEY_NOT_FOUND = -1;

// WorldRenderer::KeyHasRpkLine: 1 when any line of key `key` (up to where key + 1 starts, or the end of the table)
// begins with "RPK" or "RP2". No direct caller.
int WorldRenderer::KeyHasRpkLine(int key)
{
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == KEY_NOT_FOUND)
        end = messageCount;
    for (int line = start; line < end; line++)
    {
        if (memcmp(messages[line], "RPK", MESSAGE_TAG_LENGTH) == 0 || memcmp(messages[line], "RP2", MESSAGE_TAG_LENGTH) == 0)
            return 1;
    }
    return 0;
}

// WorldRenderer::OffsetKeysFromA: runs ApplyKeyOffsetsA with the three floats on every key whose highest line id is at
// least `firstId`. Returns 0. No direct caller.
int WorldRenderer::OffsetKeysFromA(int firstId, float x, float y, float z)
{
    int keyCount = CountMessageKeys();
    for (int key = 0; key < keyCount; key++)
    {
        LowestMessageId(key);
        if (HighestMessageId(key) >= firstId)
            ApplyKeyOffsetsA(key, firstId, x, y, z);
    }
    return 0;
}

// WorldRenderer::OffsetKeysFromB: the same with ApplyKeyOffsetsB and four floats. Returns 0. No direct caller.
int WorldRenderer::OffsetKeysFromB(int firstId, float a, float b, float c, float d)
{
    int keyCount = CountMessageKeys();
    for (int key = 0; key < keyCount; key++)
    {
        LowestMessageId(key);
        if (HighestMessageId(key) >= firstId)
            ApplyKeyOffsetsB(key, firstId, a, b, c, d);
    }
    return 0;
}

// WorldRenderer::SetKeyCommandBField: formats `value` as text and stores it with SetMessageField, in the field that
// ScanLineCommandB reports, on every line of key `key` where the scanner finds one. Returns -1. No direct caller.
int WorldRenderer::SetKeyCommandBField(int key, int value)
{
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == KEY_NOT_FOUND)
        end = messageCount;
    int field;
    char text[0x28];
    sprintf(text, "%d", value);
    for (int line = start; line < end; line++)
    {
        if (ScanLineCommandB(line, &field) != KEY_NOT_FOUND)
            SetMessageField(line, field, text);
    }
    return -1;
}

// WorldRenderer::SetKeyCommandAField: as SetKeyCommandBField with ScanLineCommandA, and only when the field it
// reports is valid. Returns -1. No direct caller.
int WorldRenderer::SetKeyCommandAField(int key, int value)
{
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == KEY_NOT_FOUND)
        end = messageCount;
    int field;
    char text[0x28];
    sprintf(text, "%d", value);
    for (int line = start; line < end; line++)
    {
        if (ScanLineCommandA(line, &field) != KEY_NOT_FOUND && field != KEY_NOT_FOUND)
            SetMessageField(line, field, text);
    }
    return -1;
}

// WorldRenderer::InsertMessageLine: opens an empty line at `position`: moves the ids and texts of the lines from
// `position` on one slot up (the last slot is dropped), gives the new line the id MESSAGE_EMPTY_ID and no text, and
// counts it. Returns 1. Called by the message editor.
int WorldRenderer::InsertMessageLine(int position)
{
    for (int line = MESSAGE_LINE_COUNT - 2; line >= position; line--)
    {
        messageIds[line + 1] = messageIds[line];
        strcpy(messages[line + 1], messages[line]);
    }
    messageIds[position] = MESSAGE_EMPTY_ID;
    messages[position][0] = '\0';
    messageCount++;
    return 1;
}

// WorldRenderer::EffectLevelKeyKind: looks through the lines of key `key` (as KeyHasRpkLine does) and returns
// EFFECT_LEVEL_KEY_K when a line begins with "RFK" or "AFK", EFFECT_LEVEL_KEY_Q for "RFQ" or "AFQ" (the first line
// that is either decides), and 0 when there is none. No direct caller.
int WorldRenderer::EffectLevelKeyKind(int key)
{
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == KEY_NOT_FOUND)
        end = messageCount;
    for (int line = start; line < end; line++)
    {
        if (memcmp(messages[line], "RFK", MESSAGE_TAG_LENGTH) == 0 || memcmp(messages[line], "AFK", MESSAGE_TAG_LENGTH) == 0)
            return EFFECT_LEVEL_KEY_K;
        if (memcmp(messages[line], "RFQ", MESSAGE_TAG_LENGTH) == 0 || memcmp(messages[line], "AFQ", MESSAGE_TAG_LENGTH) == 0)
            return EFFECT_LEVEL_KEY_Q;
    }
    return 0;
}
