// The engine's memory functions of the EN build (EN-only, linked from the sound code: no JP twin found).
#include <stdlib.h>
#include <string.h>
#include "engine_memory.h"
#include "sound_engine.h"

// Forgets `block` in the engine's block table when it is listed there, then frees it with the C runtime. Installed as
// g_engineFree at start-up (EN 0x77bb90), so every engine free ends here.
extern "C" void Engine_FreeBlock(void *block)
{
    int slot;
    for (slot = 0; slot < ENGINE_BLOCK_SLOTS; slot++)
    {
        if (g_engineBlocks[slot] == block)
            break;
    }
    if (slot != ENGINE_BLOCK_SLOTS)
        g_engineBlocks[slot] = 0;
    free(block);
}

// Takes the first free slot of the block table (the last one when it is full), remembers the first characters of the
// caller's tag there, allocates the block and records it. Installed as g_engineAlloc at start-up (EN 0x77bb90).
extern "C" void *Engine_AllocBlock(unsigned int size, const char *tag)
{
    if (size > ENGINE_ALLOC_MAX_SIZE)
        return 0;
    if (size == 0)
        return 0;
    int slot;
    for (slot = 0; slot < ENGINE_BLOCK_SLOTS; slot++)
    {
        if (g_engineBlocks[slot] == 0)
            break;
    }
    if (slot == ENGINE_BLOCK_SLOTS)
        slot = ENGINE_BLOCK_SLOTS - 1;
    strncpy(g_engineBlockTags[slot], tag, ENGINE_BLOCK_TAG_SIZE - 1);
    g_engineBlocks[slot] = malloc(size);
    return g_engineBlocks[slot];
}

// Empties the block table and tags, sets the default tag to "Unknown", installs the engine's copy, allocate and free
// functions and clears the spare words. Returns 1. Called by the sound engine start (EN 0x7c5610) and EN 0x77cc40.
extern "C" int MemoryTracker_Reset(void)
{
    int slot;
    for (slot = 0; slot < ENGINE_BLOCK_SLOTS; slot++)
        g_engineBlocks[slot] = 0;
    for (slot = 0; slot < ENGINE_BLOCK_SLOTS; slot++)
        g_engineBlockTags[slot][0] = 0;
    g_engineSpareCount = 0;
    strcpy(g_engineDefaultTag, "Unknown");
    g_engineCopy = (void *(*)(void *, const void *, int))memcpy;
    g_engineAlloc = Engine_AllocBlock;
    g_engineFree = Engine_FreeBlock;
    memset(g_engineSpareWords, 0, sizeof(g_engineSpareWords));
    g_engineSpareText[0] = 0;
    return 1;
}
