// FontSet (EN build). JP twin: input_font.cpp.
#include "engine_memory.h"
#include "input_font.h"

// Frees the font buffers, deletes the three GDI fonts and frees the 32 cached glyph bitmaps. Returns 1. Called when
// the text system shuts down.
int FontSet::Release()
{
    if (buffer != 0)
        g_engineFree(buffer);
    buffer = 0;
    if (buffer2 != 0)
        g_engineFree(buffer2);
    buffer2 = 0;
    if (fonts[0] != 0)
        DeleteObject(fonts[0]);
    if (fonts[1] != 0)
        DeleteObject(fonts[1]);
    if (fonts[2] != 0)
        DeleteObject(fonts[2]);
    for (int i = 0; i < GLYPH_CACHE_COUNT; i++)
    {
        if (glyphs[i] != 0)
            g_engineFree(glyphs[i]);
        glyphs[i] = 0;
    }
    return 1;
}
