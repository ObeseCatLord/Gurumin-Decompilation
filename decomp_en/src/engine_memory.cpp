// The engine's memory functions of the EN build (EN-only, linked from the sound code: no JP twin found).
#include <stdlib.h>
#include "engine_memory.h"

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
