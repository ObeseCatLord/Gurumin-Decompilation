// Report of the blocks the engine allocator still holds (EN build, EN-only: no JP twin found).
#include <stdio.h>
#include "engine_memory.h"

// Writes the file `path` with one line per block slot in use (slot number, tag and address); when no block is in use
// the file is not made. No direct caller.
extern "C" void Engine_WriteBlockReport(const char *path)
{
    for (int first = 0; first < ENGINE_BLOCK_SLOTS; first++)
    {
        if (g_engineBlocks[first] != 0)
        {
            FILE *file = fopen(path, "wt");
            if (file != 0)
            {
                for (int slot = 0; slot < ENGINE_BLOCK_SLOTS; slot++)
                {
                    if (g_engineBlocks[slot] != 0)
                    {
                        fprintf(file, "%05d %s %x", slot, g_engineBlockTags[slot], (unsigned int)g_engineBlocks[slot]);
                        fputs("\n", file);
                    }
                }
                fclose(file);
            }
            return;
        }
    }
}
