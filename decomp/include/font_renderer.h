// FontRenderer: declared here once (first used by src/font_metrics.cpp).
#ifndef GURUMIN_FONT_RENDERER_H
#define GURUMIN_FONT_RENDERER_H

#include <windows.h>

struct FontRenderer
{
    HFONT font;                 // 0x00: the Windows font (MS P Gothic, 17 pixels)
    HBITMAP bitmap;             // 0x04: 512x64 bitmap selected into `dc`
    HDC dc;                     // 0x08: memory DC with the Windows font (used when no bitmap font is loaded)
    unsigned int *buffers[0x20];    // 0x0c: pixel buffer of each text texture
    char texts[0x20][0x100];    // 0x8c: the text each texture holds
    int textures[0x20];         // 0x208c: texture slots "tex00".."tex31"
    int m_210c[0x20];
    char pad_218c[0x80];
    int m_220c[0x20];
    char pad_228c[0x100];
    unsigned short *glyphs;     // 0x238c: normal font (ddfont8.dat), 72 words per glyph
    unsigned short *altGlyphs;  // 0x2390: alternative font (ddfont8s.dat, the small variant), may be missing
    int useAlt;                 // 0x2394: 1 = draw with the alternative font when it is loaded

    int Init(HWND window, const char *path);
    int GlyphWidth(int color, int x, int y, unsigned short code, unsigned short size, unsigned short sizeAgain);
    int DrawGlyph(unsigned short *pixels, int x, int y, unsigned short code, unsigned short color, int unused);
    int DrawString(int color, int x, int y, const char *text, unsigned short size, int fixedAdvance);
    int TextWidth(const char *text, float scale);   // 0x1014fd30: width of `text` in pixels
    // 0x101501f0: draws `text` at (x, y) in `colour` with opacity `alpha`, scaled by `scale`.
    int DrawText(int x, int y, const char *text, int colour, int a, float alpha, float scale);
    // 0x101508d0 (text_outline.cpp): DrawText with a brown outline when outlines are on.
    int DrawOutlined(int x, int y, const char *text, int colour, int a, float alpha, float scale);
    // 0x101509b0 (text_outline.cpp): DrawOutlined with the outline in `outlineColour`.
    int DrawOutlinedIn(int x, int y, const char *text, int colour, int a, float alpha, float scale,
                       int outlineColour);
};

extern "C" FontRenderer DAT_1020e538;   // the game's text renderer

#endif
