// Searching the lines of a message key for script commands (EN build, EN-only message editor: no JP twin).
#include <string.h>
#include "world_renderer.h"

// Result of the scanners when a line (or key) has no command of the wanted kind.
const int NO_COMMAND = -1;

// WorldRenderer::FindKeyCommandA: walks the lines of key `key` (from where the key starts up to where key + 1 starts, or
// the end of the table) and returns the first result of ScanLineCommandA that is not -1. Called by the message editor.
int WorldRenderer::FindKeyCommandA(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandA(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandB: as FindKeyCommandA with ScanLineCommandB.
int WorldRenderer::FindKeyCommandB(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandB(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandC: as FindKeyCommandA with ScanLineCommandC.
int WorldRenderer::FindKeyCommandC(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandC(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandD: as FindKeyCommandA with ScanLineCommandD.
int WorldRenderer::FindKeyCommandD(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandD(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandE: as FindKeyCommandA with ScanLineCommandE.
int WorldRenderer::FindKeyCommandE(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandE(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandF: as FindKeyCommandA with ScanLineCommandF.
int WorldRenderer::FindKeyCommandF(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandF(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandG: as FindKeyCommandA with ScanLineCommandG.
int WorldRenderer::FindKeyCommandG(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandG(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandH: as FindKeyCommandA with ScanLineCommandH.
int WorldRenderer::FindKeyCommandH(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandH(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandI: as FindKeyCommandA with ScanLineCommandI.
int WorldRenderer::FindKeyCommandI(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandI(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandJ: as FindKeyCommandA with ScanLineCommandJ.
int WorldRenderer::FindKeyCommandJ(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandJ(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// WorldRenderer::FindKeyCommandK: as FindKeyCommandA with ScanLineCommandK.
int WorldRenderer::FindKeyCommandK(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int found = ScanLineCommandK(line, 0);
        if (found != NO_COMMAND)
            return found;
    }
    return NO_COMMAND;
}

// Position ScanLineCommandA is given to fill; it stays at this value when the line has no position.
const int NO_POSITION = -1;

// WorldRenderer::FindKeyCommandAWithPosition: as FindKeyCommandA, but only a line for which ScanLineCommandA returns
// a result and also fills in a position counts. Called by the message editor.
int WorldRenderer::FindKeyCommandAWithPosition(int key)
{
    int line = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (; line < end; line++)
    {
        int position = NO_POSITION;
        int found = ScanLineCommandA(line, &position);
        if (found != NO_COMMAND && position != NO_POSITION)
            return found;
    }
    return NO_COMMAND;
}

// Characters of the "RLK" and "ROP" command tags that start a message line.
const int MESSAGE_TAG_LENGTH = 3;

// WorldRenderer::KeyHasRlkLine: 1 when any line of key `key` (from where the key starts up to where key + 1 starts, or the
// end of the table) begins with "RLK". Called by the message editor.
int WorldRenderer::KeyHasRlkLine(int key)
{
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (int line = start; line < end; line++)
    {
        if (memcmp(messages[line], "RLK", MESSAGE_TAG_LENGTH) == 0)
            return 1;
    }
    return 0;
}

// WorldRenderer::KeyHasRopLine: as KeyHasRlkLine for lines that begin with "ROP".
int WorldRenderer::KeyHasRopLine(int key)
{
    int start = FindMessageLine(key);
    int end = FindMessageLine(key + 1);
    if (end == NO_COMMAND)
        end = messageCount;
    for (int line = start; line < end; line++)
    {
        if (memcmp(messages[line], "ROP", MESSAGE_TAG_LENGTH) == 0)
            return 1;
    }
    return 0;
}
