// FontRenderer of the EN build: text drawn through bitmap fonts or the Windows font and cached in text textures.
// Declared once here (JP: decomp/include/font_renderer.h).
#ifndef GURUMIN_EN_FONT_RENDERER_H
#define GURUMIN_EN_FONT_RENDERER_H

#include <windows.h>

const int FONT_TEXT_SLOTS = 0x20;     // text textures cached by the renderer
const int FONT_TEXT_SIZE = 0x100;     // bytes of the text kept for each slot

struct FontRenderer
{
    HFONT font;                                      // 0x00: the Windows font
    HBITMAP bitmap;                                  // 0x04: bitmap selected into `dc`
    HDC dc;                                          // 0x08: memory DC with the Windows font (used when no bitmap font is loaded)
    unsigned short *buffers[FONT_TEXT_SLOTS];        // 0x0c: ARGB4444 pixel buffer of each text texture
    char texts[FONT_TEXT_SLOTS][FONT_TEXT_SIZE];     // 0x8c: the text each texture holds
    int textures[FONT_TEXT_SLOTS];                   // 0x208c: texture slots of the text textures
    int recency[FONT_TEXT_SLOTS];                    // 0x210c: age counter of each texture
    int colours[FONT_TEXT_SLOTS];                    // 0x218c: colour each texture was rendered in
    float alphas[FONT_TEXT_SLOTS];                   // 0x220c: alpha each texture was requested with
    int widths[FONT_TEXT_SLOTS];                     // 0x228c: width in pixels of each rendered text
    int heights[FONT_TEXT_SLOTS];                    // 0x230c: height in pixels of each rendered text
    unsigned short *glyphs;                          // 0x238c: normal bitmap font (0 = use the Windows font)
    unsigned short *altGlyphs;                       // 0x2390: alternative (small) bitmap font, may be missing
    int useAlt;                                      // 0x2394: 1 = draw with the alternative font when it is loaded

    int TextWidth(const char *text, float scale);    // EN 0x7a10e0 (JP 0x1014fd30): width of `text` in pixels
    // EN 0x7a2c60 (JP 0x1014f590): lays out `text` and returns the pen position after its last character.
    int DrawString(int color, int x, int y, const char *text, unsigned short size, int fixedAdvance);
};

#endif
