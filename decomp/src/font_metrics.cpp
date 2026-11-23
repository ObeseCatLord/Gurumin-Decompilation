// Text renderer: glyph metrics.
//
// The game draws all text from a pre-rendered Shift-JIS bitmap font (3ddata\ddfont8.dat, loaded by 0x101455d0).
// Each glyph is a 144-byte record = 72 little-endian 16-bit words: 17 rows of 4 words, each word holding five pixels of
// 3-bit coverage. Bits 9..13 of word 3 store the glyph's advance width (see tools/ddfont_tool.py and docs/MODDING_MAP.md).
// Replacing this font with a TrueType/Unicode one is a roadmap goal (docs/ROADMAP.md); these functions are the hooks.

#include <windows.h>
#include <string.h>

#include "font_renderer.h"

// Returns the advance width in pixels of one Shift-JIS character (width bits + 1).
// The arguments mirror the character-draw function so callers can pass the same list; only `code` is used.
// `code` arrives with its two bytes in memory order (lead byte first) and is swapped into a numeric SJIS code here.
// Codes outside the font fall back to record 4 (index 0x1c0 = 4 * 72 words, a full-width space in the table).
int FontRenderer::GlyphWidth(int x, int y, int color, unsigned short code, int a, int b)
{
    unsigned short *font = glyphs;
    if (useAlt == 1 && altGlyphs != 0)
        font = altGlyphs;

    // Swap the two bytes of the character in place: lead byte becomes the high byte.
    unsigned short raw = code;
    unsigned char high = (unsigned char)(raw >> 8);
    unsigned char low = (unsigned char)(raw & 0xff);
    ((unsigned char *)&code)[0] = high;
    ((unsigned char *)&code)[1] = low;

    // 72 words per glyph; the first code in the font is 0x8141 (the ideographic comma).
    int index = (code - 0x8141) * 72;
    if (index < 0 || index > 0x108840)  // 0x108840 = 15,048 glyphs * 72 words (end of the table)
        index = 0x1c0;
    return ((font[index + 3] >> 9) & 0x1f) + 1;
}

// 0x101453e0 (Font_DrawGlyph). Draws the Shift-JIS character `code` (bytes in memory order, as in GlyphWidth) at
// (x, y) of the 512-pixel-wide ARGB4444 surface `pixels`: each of the 17 rows has `width` pixels whose 3-bit
// coverage becomes the alpha nibble's top bits over `color` (coverage 0 leaves just `color`), and one column right
// of the glyph plus one row below it are filled with `color`. Returns the advance width (glyph width + 1).
int FontRenderer::DrawGlyph(unsigned short *pixels, int x, int y, unsigned short code, unsigned short color, int unused)
{
    unsigned short *font = glyphs;
    if (useAlt == 1 && altGlyphs != 0)
        font = altGlyphs;
    unsigned short raw = code;
    unsigned char high = (unsigned char)(raw >> 8);
    unsigned char low = (unsigned char)(raw & 0xff);
    ((unsigned char *)&code)[0] = high;
    ((unsigned char *)&code)[1] = low;
    // the font skips the unused codes between 0x9ffc and 0xe040
    int index;
    if (code >= 0xe040)
        index = (code - 0x4043) * 72;
    else
        index = (code - 0x8141) * 72;
    if (index < 0 || index > 0x108840)
        index = 0x1c0;
    int width = (font[index + 3] >> 9) & 0x1f;
    int row = 0;
    unsigned short *line = &pixels[y * 0x200 + x];
    do
    {
        unsigned short *out = line;
        for (int column = 0; column < width; column++)
        {
            unsigned short bits = font[column / 5 + row + index];
            if (((bits >> (column % 5 * 3)) & 7) == 0)
                *out = color;
            else
                *out = (((bits >> (column % 5 * 3)) & 7) << 13) + 0x1000 | color;
            out++;
        }
        row += 4;
        line += 0x200;
    } while (row < 0x44);
    int advance = width + 1;
    unsigned short *edge = &pixels[y * 0x200 + width + x];
    for (int i = 0x11; i != 0; i--)
    {
        for (int k = width; k < advance; k++)
            edge[k - width] = color;
        edge += 0x200;
    }
    unsigned short *bottom = &pixels[(y + 0x11) * 0x200 + width + x];
    for (int k = width; k < advance; k++)
        bottom[k - width] = color;
    return advance;
}

// Width in pixels of `text` drawn at `scale`: measured with the bitmap font when one is loaded (the text-draw
// routine also returns the width), otherwise with the Windows font through GetTextExtentPoint32A.
int FontRenderer::TextWidth(const char *text, float scale)
{
    unsigned short *font = glyphs;
    if (useAlt == 1 && altGlyphs != 0)
        font = altGlyphs;
    if (font == 0)
    {
        SIZE size;
        GetTextExtentPoint32A(dc, text, strlen(text), &size);
        return (int)((float)size.cx * scale);
    }
    return (int)((float)DrawString(0, 0, 0, text, 0, 0) * scale);
}

#include <stdio.h>

// The engine's file object (save_slot.cpp GameFile).
struct GameFile
{
    FILE *handle;
    char name[0x24];
    GameFile();                                      // 0x101ad9e0
    ~GameFile();                                     // 0x101b2570
    FILE *Open(const char *path, const char *mode);  // 0x101b2310: the opened file, 0 on failure
    void Close();                                    // 0x101b22f0
    int Read(void *buffer, int size, int count);     // 0x101b23a0
};

extern "C" const char *FUN_1013f8a0(const char *name, int folder);   // builds the data path of `name`
#include "engine_memory.h"
extern "C" HWND DAT_10f6569c;                        // the game window
#include "render_vars.h"
#include "ui_vars.h"

extern "C" int FUN_10175c90(const char *name, int width, int height, const unsigned char *pixels, int alpha,
                            int flags);              // Texture_CreateFromPixels

// "ＭＳ Ｐゴシック" (MS P Gothic), Shift-JIS.
#define FONT_FACE "\x82\x6c\x82\x72\x20\x82\x6f\x83\x53\x83\x56\x83\x62\x83\x4e"

// 0x101455d0 (Font_Init). Loads the bitmap fonts: `path` (as given or in the data folder) or ddfont8s.dat as the
// main font with ddfont8.dat as the alternative, or only ddfont8.dat. Then creates the Windows font with its DC and
// bitmap, and the 32 text textures "tex00".."tex31" with their pixel buffers. Returns 1.
int FontRenderer::Init(HWND window, const char *path)
{
    useAlt = 0;
    GameFile file;
    FILE *opened;
    if (path != 0)
    {
        opened = file.Open(path, "rb");
        if (opened != 0)
            goto loadFont;
        opened = file.Open(FUN_1013f8a0(path, 0), "rb");
        if (opened != 0)
            goto loadFont;
    }
    opened = file.Open(FUN_1013f8a0("ddfont8s.dat", 0), "rb");
    if (opened != 0)
    {
    loadFont:
        glyphs = (unsigned short *)DAT_11b0b938(0x211080, "m_lpFont");
        file.Read(glyphs, 1, 0x211080);
        file.Close();
        altGlyphs = 0;
        bitmap = 0;
        dc = 0;
        font = 0;
        opened = file.Open(FUN_1013f8a0("ddfont8.dat", 0), "rb");
        if (opened != 0)
        {
            altGlyphs = (unsigned short *)DAT_11b0b938(0x211080, "m_lpFont2");
            file.Read(altGlyphs, 1, 0x211080);
            file.Close();
        }
    }
    else
    {
        opened = file.Open(FUN_1013f8a0("ddfont8.dat", 0), "rb");
        if (opened != 0)
        {
            glyphs = (unsigned short *)DAT_11b0b938(0x211080, "m_lpFont");
            file.Read(glyphs, 1, 0x211080);
            file.Close();
            altGlyphs = 0;
            bitmap = 0;
            dc = 0;
            font = 0;
        }
    }
    HWND gameWindow = DAT_10f6569c;
    HDC windowDc = GetDC(gameWindow);
    bitmap = CreateCompatibleBitmap(windowDc, 0x200, 0x40);
    dc = CreateCompatibleDC(windowDc);
    SelectObject(dc, bitmap);
    ReleaseDC(gameWindow, windowDc);
    LOGFONTA logFont;
    memset(&logFont, 0, sizeof(logFont));
    strcpy(logFont.lfFaceName, FONT_FACE);
    logFont.lfPitchAndFamily = 0;
    logFont.lfCharSet = SHIFTJIS_CHARSET;
    logFont.lfHeight = 0x11;
    logFont.lfWeight = 0xdac;
    logFont.lfItalic = 0;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfOutPrecision = 0;
    logFont.lfClipPrecision = 0;
    logFont.lfQuality = 0;
    font = CreateFontIndirectA(&logFont);
    SelectObject(dc, font);
    for (int i = 0; i < 0x20; i++)
    {
        m_220c[i] = 0;
        m_210c[i] = 0;
        texts[i][0] = 0;
        unsigned int *buffer;
        int words;
        if (DAT_10f654a4 != 0)
        {
            buffer = (unsigned int *)DAT_11b0b938(0x14000, "m_fontbuff");
            words = 0x5000;
        }
        else
        {
            buffer = (unsigned int *)DAT_11b0b938(0xc000, "m_fontbuff");
            words = 0x3000;
        }
        buffers[i] = buffer;
        for (; words != 0; words--)
            *buffer++ = 0;
        char name[0x80];
        sprintf(name, "tex%02d", i);
        int creating = DAT_1101d878;
        int deep = DAT_10f654a4;
        DAT_1101d878 = 1;
        DAT_10f654a4 = 0;
        textures[i] = FUN_10175c90(name, 0x200, 0x20, (unsigned char *)buffers, (int)buffers, 0);
        DAT_10f654a4 = deep;
        DAT_1101d878 = creating;
    }
    return 1;
}
