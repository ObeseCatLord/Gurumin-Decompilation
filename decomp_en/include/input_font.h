// FontSet of the EN build (JP: decomp/include/input_font.h): GDI fonts and glyph bitmaps of the on-screen text.
// Only the members EN code has used so far are named. This is the one declaration of FontSet in decomp_en.
#ifndef GURUMIN_EN_INPUT_FONT_H
#define GURUMIN_EN_INPUT_FONT_H

#include <windows.h>

// Number of GDI fonts and cached glyph bitmaps of a FontSet.
const int FONT_COUNT = 3;
const int GLYPH_CACHE_COUNT = 0x20;

struct FontSet
{
    HFONT fonts[FONT_COUNT];            // 0x000: the GDI fonts
    void *glyphs[GLYPH_CACHE_COUNT];    // 0x00c: cached glyph bitmaps
    char m_008c[0x238c - 0x8c];         // 0x08c: not used by EN code yet
    void *buffer;                       // 0x238c: font buffer
    void *buffer2;                      // 0x2390: second font buffer

    int Release();                      // EN 0x7a3be0 (JP 0x1015a0c0)
};

#endif
