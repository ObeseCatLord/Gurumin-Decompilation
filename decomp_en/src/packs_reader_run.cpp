// Running the Packs image reader (EN build, EN-only: no JP twin found).
#include "small_b11.h"
#include "packs_table.h"

// Calls the image reader of the Packs function table with `command` on the scratch block and the image size, and keeps
// the size it returns. Returns 0 when there is no scratch block or the reader fails, else 1. Called by the screens that
// preview a Packs image (EN 0x5c8130, 0x60cb10).
extern "C" int Packs_RunImageReader(int command)
{
    if (g_scratchBlock == 0)
        return 0;
    int width = g_packsImageWidth;
    int height = g_packsImageHeight;
    if (g_packsReader(command, g_scratchBlock, PACKS_READER_ROW_BYTES, &width, &height, 1) == 0)
        return 0;
    g_packsImageWidth = width;
    g_packsImageHeight = height;
    return 1;
}
