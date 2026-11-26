// FontRenderer of the EN build: text drawn through bitmap fonts or the Windows font and cached in text textures.
// Declared once here (JP: decomp/include/font_renderer.h).
#ifndef GURUMIN_EN_FONT_RENDERER_H
#define GURUMIN_EN_FONT_RENDERER_H

#include <windows.h>

// <windows.h> turns DrawText into DrawTextA; the renderer's own method keeps its name.
#undef DrawText

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
    int digitAdvance;                                // 0x2398: pen advance in pixels of a digit in proportional text

    int TextWidth(const char *text, float scale);    // EN 0x7a10e0 (JP 0x1014fd30): width of `text` in pixels
    // EN 0x7a3980 (JP 0x10145380): advance in pixels of one Shift-JIS character `code` (bytes in memory order). The
    // other arguments mirror DrawGlyph's list and are not used.
    int GlyphWidth(int colour, int x, int y, unsigned short code, int unused, int unusedAgain);
    // EN 0x7a2790: lays out `text` like DrawString (EN 0x7a2c60) but also draws the glyphs into the 640-pixel wide
    // ARGB4444 text buffer `buffer`; returns the pen position after the last character.
    int DrawStringToBuffer(unsigned short *buffer, int x, int y, const char *text, int colour, int fixedAdvance);
    // EN 0x7a3720: draws one Shift-JIS glyph into `buffer` at (x, y) and returns its advance in pixels.
    int DrawGlyph(unsigned short *buffer, int x, int y, unsigned short code, int colour, int colourAgain);
    // EN 0x7a2c60 (JP 0x1014f590): lays out `text` and returns the pen position after its last character.
    int DrawString(int color, int x, int y, const char *text, unsigned short size, int fixedAdvance);
    // EN 0x78fd50 (JP 0x101501f0): draws `text` at (x, y) in `colour` (0xRRGGBB) at `depth`, faded by `alpha` and scaled.
    int DrawText(int x, int y, const char *text, int colour, float depth, float alpha, float scale);
    // EN 0x790d10 (JP 0x101508d0): DrawText with a dark brown outline when outlines are on.
    int DrawOutlined(int x, int y, const char *text, int colour, float depth, float alpha, float scale);
    // EN 0x790bc0 (JP 0x101509b0): DrawOutlined with the outline in `outlineColour`.
    int DrawOutlinedIn(int x, int y, const char *text, int colour, float depth, float alpha, float scale,
                       int outlineColour);
};

// Non-zero when outlined text draws its outline (EN 0x00908a3c, JP DAT_10202b20).
extern "C" int g_textOutlineEnabled;

// The game's text renderer (EN 0x00924d68, JP DAT_1020e538).
extern "C" FontRenderer g_textRenderer;

#endif
