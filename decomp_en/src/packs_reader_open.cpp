// Opening the Packs image reader on a fresh scratch block (EN build, EN-only: no JP twin found).
#include <stdio.h>
#include <stdlib.h>

#include "small_b11.h"
#include "packs_table.h"

// Forgets the reported image size, replaces the scratch block with a new one, closes the image file of the previous
// preview and runs the image reader with `command` on the new block. Returns 1 when the reader succeeded, else 0.
// Called by the screens that preview a Packs image (see Packs_RunImageReader, Packs_CloseImageReader).
extern "C" int Packs_OpenImageReader(int command)
{
    g_packsImageWidth = 0;
    g_packsImageHeight = 0;
    if (g_scratchBlock != 0)
        free(g_scratchBlock);
    g_scratchBlock = 0;
    g_scratchBlock = operator new(PACKS_SCRATCH_BLOCK_BYTES);
    if (g_packsImageFile != 0)
        fclose(g_packsImageFile);
    g_packsImageFile = 0;
    g_packsImageState = 0;
    if (g_scratchBlock != 0)
    {
        int width = g_packsImageWidth;
        int height = g_packsImageHeight;
        if (g_packsReader(command, g_scratchBlock, PACKS_READER_ROW_BYTES, &width, &height, 1) != 0)
        {
            g_packsImageWidth = width;
            g_packsImageHeight = height;
            g_packsImageState = 0;
            return 1;
        }
    }
    return 0;
}
