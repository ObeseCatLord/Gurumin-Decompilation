// FontRenderer: declared here once (first used by src/font_metrics.cpp).
#ifndef GURUMIN_FONT_RENDERER_H
#define GURUMIN_FONT_RENDERER_H

#include <windows.h>

struct FontRenderer
{
    HFONT font;                 // 0x00: the Windows font (MS P Gothic, 17 pixels)
    HBITMAP bitmap;             // 0x04: 512x64 bitmap selected into `dc`
    HDC dc;                     // 0x08: memory DC with the Windows font (used when no bitmap font is loaded)
    unsigned short *buffers[0x20];   // 0x0c: ARGB4444 pixel buffer of each text texture (512 x 32)
    char texts[0x20][0x100];    // 0x8c: the text each texture holds
    int textures[0x20];         // 0x208c: texture slots "tex00".."tex31"
    int recency[0x20];          // 0x210c: 0x20 right after the texture is drawn, counted down by every other draw
    int colours[0x20];          // 0x218c: colour each texture was rendered in
    float alphas[0x20];         // 0x220c: alpha each texture was requested with (part of the cache key)
    int widths[0x20];           // 0x228c: width in pixels of each rendered text
    int heights[0x20];          // 0x230c: height in pixels (always 0x11)
    unsigned short *glyphs;     // 0x238c: normal font (ddfont8.dat), 72 words per glyph
    unsigned short *altGlyphs;  // 0x2390: alternative font (ddfont8s.dat, the small variant), may be missing
    int useAlt;                 // 0x2394: 1 = draw with the alternative font when it is loaded

    int Init(HWND window, const char *path);
    int GlyphWidth(int color, int x, int y, unsigned short code, unsigned short size, unsigned short sizeAgain);
    int DrawGlyph(unsigned short *pixels, int x, int y, unsigned short code, unsigned short color, unsigned short unused);
    int DrawString(int color, int x, int y, const char *text, unsigned short size, int fixedAdvance);
    // 0x1014f900: draws `text` into the 512-pixel-wide ARGB4444 buffer `pixels` and returns the pen position.
    int DrawStringToPixels(unsigned short *pixels, int x, int y, const char *text, unsigned short color, int fixedAdvance);
    // 0x1014fdd0: draws `text` through the texture cache (renders it into a text texture when no slot has the same text,
    // colour and alpha) at (x, y) scaled by `scale`, clipped to the rectangle (clipLeft, clipTop)-(clipRight, clipBottom).
    // Returns the width of the text in pixels at that scale.
    int DrawTextClipped(int x, int y, const char *text, int clipLeft, int clipTop, int clipRight, int clipBottom, int colour,
                        float z, float alpha, float scale);
    int TextWidth(const char *text, float scale);   // 0x1014fd30: width of `text` in pixels
    // 0x101501f0: draws `text` at (x, y) in `colour` with opacity `alpha`, scaled by `scale`.
    int DrawText(int x, int y, const char *text, int colour, float depth, float alpha, float scale);
    // 0x101508d0 (text_outline.cpp): DrawText with a brown outline when outlines are on.
    int DrawOutlined(int x, int y, const char *text, int colour, float depth, float alpha, float scale);
    // 0x101509b0 (text_outline.cpp): DrawOutlined with the outline in `outlineColour`.
    int DrawOutlinedIn(int x, int y, const char *text, int colour, float depth, float alpha, float scale,
                       int outlineColour);
};

extern "C" FontRenderer DAT_1020e538;   // the game's text renderer

#endif
