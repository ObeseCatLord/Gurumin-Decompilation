// Searching the lines of a message key for script commands (EN build, EN-only message editor: no JP twin).
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
