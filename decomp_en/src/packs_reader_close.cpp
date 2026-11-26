// Closing the Packs image reader (EN build, EN-only: no JP twin found).
#include <stdio.h>
#include <stdlib.h>

#include "small_b11.h"

// Frees the scratch block, tells the image reader of the Packs function table to close its file, and closes the image
// file. Called by the screens that preview a Packs image when they are left (EN 0x5c8130, 0x5e87d0). Returns 0.
extern "C" int Packs_CloseImageReader(void)
{
    if (g_scratchBlock != 0)
        free(g_scratchBlock);
    g_scratchBlock = 0;
    g_packsReader(PACKS_READER_CLOSE, 0, PACKS_READER_ROW_BYTES, 0, 0, 1);
    if (g_packsImageFile != 0)
        fclose(g_packsImageFile);
    g_packsImageFile = 0;
    return 0;
}
