// Text renderer: glyph metrics.
//
// The game draws all text from a pre-rendered Shift-JIS bitmap font (3ddata\ddfont8.dat, loaded by 0x101455d0).
// Each glyph is a 144-byte record = 72 little-endian 16-bit words: 17 rows of 4 words, each word holding five pixels of
// 3-bit coverage. Bits 9..13 of word 3 store the glyph's advance width (see tools/ddfont_tool.py and docs/MODDING_MAP.md).
// Replacing this font with a TrueType/Unicode one is a roadmap goal (docs/ROADMAP.md); these functions are the hooks.

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "font_renderer.h"
#include "texture_object.h"

extern "C" int FUN_101616e0(int slot, int left, int top, int right, int bottom, float u0, float v0, float u1, float v1,
                            float z, float alpha, float angle);   // draws a texture slot as a rectangle (item_icon.cpp)

// Returns the advance width in pixels of one Shift-JIS character (width bits + 1).
// The arguments mirror the character-draw function so callers can pass the same list; only `code` is used.
// `code` arrives with its two bytes in memory order (lead byte first) and is swapped into a numeric SJIS code here.
// Codes outside the font fall back to record 4 (index 0x1c0 = 4 * 72 words, a full-width space in the table).
int FontRenderer::GlyphWidth(int color, int x, int y, unsigned short code, unsigned short size, unsigned short sizeAgain)
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
int FontRenderer::DrawGlyph(unsigned short *pixels, int x, int y, unsigned short code, unsigned short color, unsigned short unused)
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

// Full-width Shift-JIS codes (bytes in memory order, lead byte first) that replace the ASCII digits and marks in
// proportional text.
#define SJIS_FULLWIDTH_ZERO     0x4f82  // "０"; "１".."９" follow at 0x5082..0x5882 (lead byte 0x82, trail + 1)
#define SJIS_FULLWIDTH_STEP     0x0100  // next digit: trail byte + 1
#define SJIS_FULLWIDTH_MINUS    0x7c81  // "－"
#define SJIS_FULLWIDTH_PERIOD   0x4481  // "．"
#define SJIS_MIDDLE_DOT         0x4581  // "・", drawn for '_' with a 1-pixel advance

#define FONT_LINE_HEIGHT        17      // rows per glyph = line spacing for '\n'
#define FIXED_MINUS_ADVANCE     9       // '-' in fixed-advance text only moves the pen
#define PAD_COLUMN_SCALE        1.13636f  // "snnn" columns are given in 1/1.13636 pixels (640 vs 563 units)
#define PAD_SEARCH_PASSES       17
#define FIXED_COLUMN_SCALE      1.11111f  // "snnn" columns of fixed-advance text

// 0x1014f590. Measures (and, through GlyphWidth's argument list, lays out) the string `text` starting at (x, y) and
// returns the pen position after the last character. Inline commands in the text:
//   "Lnn"  sets the glyph size to nn (passed on to every following glyph),
//   "snnn" moves the pen to column nnn (scaled by 1.13636 in proportional text),
//   '\n'   returns to the start column and moves down one line.
// With fixedAdvance == 0 the text is proportional and ASCII digits, '-', '.' and '_' are shown as their full-width
// Shift-JIS forms; otherwise every character advances by fixedAdvance pixels and '-' leaves a 9-pixel gap.
// Called by TextWidth (0x1014fd30).
int FontRenderer::DrawString(int color, int x, int y, const char *text, unsigned short size, int fixedAdvance)
{
    int length = strlen(text);
    int pen = 0;
    char number[4];
    int i;

    if (fixedAdvance == 0)
    {
        for (i = 0; i < length; )
        {
            char c = text[i];
            if (c == '0')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO, size, size);
            }
            else if (c == '1')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO + 1 * SJIS_FULLWIDTH_STEP, size, size);
            }
            else if (c == '2')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO + 2 * SJIS_FULLWIDTH_STEP, size, size);
            }
            else if (c == '3')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO + 3 * SJIS_FULLWIDTH_STEP, size, size);
            }
            else if (c == '4')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO + 4 * SJIS_FULLWIDTH_STEP, size, size);
            }
            else if (c == '5')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO + 5 * SJIS_FULLWIDTH_STEP, size, size);
            }
            else if (c == '6')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO + 6 * SJIS_FULLWIDTH_STEP, size, size);
            }
            else if (c == '7')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO + 7 * SJIS_FULLWIDTH_STEP, size, size);
            }
            else if (c == '8')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO + 8 * SJIS_FULLWIDTH_STEP, size, size);
            }
            else if (c == '9')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_ZERO + 9 * SJIS_FULLWIDTH_STEP, size, size);
            }
            else if (c == '-')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_MINUS, size, size);
            }
            else if (c == '.')
            {
                i++;
                pen += GlyphWidth(color, x + pen, y, SJIS_FULLWIDTH_PERIOD, size, size);
            }
            else if (c == '_')
            {
                i++;
                GlyphWidth(color, x + pen, y, SJIS_MIDDLE_DOT, size, size);
                pen += 1;
            }
            else if (c == 'L')
            {
                i++;  // skip the command letter
                number[0] = text[i];
                number[1] = text[i + 1];
                number[2] = 0;
                size = (unsigned char)atol(number);
                i += 2;
            }
            else if (c == 's')
            {
                i++;  // skip the command letter
                number[0] = text[i];
                number[1] = text[i + 1];
                number[2] = text[i + 2];
                number[3] = 0;
                // Searches the first pixel column at or past the target (the result is not used: the pen is set
                // from the target directly below).
                for (int pass = PAD_SEARCH_PASSES; pass != 0; pass--)
                {
                    int column = pen;
                    while ((float)column < (float)atol(number) * PAD_COLUMN_SCALE)
                        column++;
                }
                pen = (int)((float)atol(number) * PAD_COLUMN_SCALE);
                i += 3;
            }
            else if (c == '\n')
            {
                i++;
                pen = 0;
                y += FONT_LINE_HEIGHT;
            }
            else
            {
                // Double-byte Shift-JIS character.
                pen += GlyphWidth(color, x + pen, y, *(const unsigned short *)(text + i), size, size);
                i += 2;
            }
        }
        return pen;
    }

    for (i = 0; i < length; )
    {
        char c = text[i];
        if (c == '-')
        {
            i++;
            pen += FIXED_MINUS_ADVANCE;
        }
        else if (c == 'L')
        {
            i++;  // skip the command letter
            number[0] = text[i];
            number[1] = text[i + 1];
            number[2] = 0;
            size = (unsigned char)atol(number);
            i += 2;
        }
        else if (c == 's')
        {
            i++;  // skip the command letter
            number[0] = text[i];
            number[1] = text[i + 1];
            number[2] = text[i + 2];
            number[3] = 0;
            pen = (unsigned char)atol(number);
            i += 3;
        }
        else if (c == '\n')
        {
            i++;
            pen = 0;
            y += FONT_LINE_HEIGHT;
        }
        else
        {
            GlyphWidth(color, x + pen, y, *(const unsigned short *)(text + i), size, size);
            pen += fixedAdvance;
            i += 2;
        }
    }
    return pen;
}

// 0x1014f900 (Font_DrawStringToPixels). DrawString for a pixel buffer: draws `text` into the 512-pixel-wide ARGB4444
// surface `pixels` starting at (x, y) with DrawGlyph and returns the pen position after the last character.
// Commands as in DrawString, except that "Lnn" sets the colour (nn is its low byte) and "snnn" in proportional text
// also fills the gap up to the new column with the colour. With fixedAdvance != 0 every character advances by
// fixedAdvance, "snnn" columns use a 1/1.11111 scale and '-' leaves a 9-pixel gap. Finally a 4-pixel wide block of the
// colour is written after the text on each of the 17 rows.
int FontRenderer::DrawStringToPixels(unsigned short *pixels, int x, int y, const char *text, unsigned short color, int fixedAdvance)
{
    int length = strlen(text);
    int pen = 0;
    int i = 0;
    unsigned short *row;
    int column;
    char number[4];

    if (fixedAdvance == 0)
    {
        int rowOffset;
        if (length > 0)
        {
        rowOffset = y << 9;
        do
        {
            char c = text[i];
            if (c == '0')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO, color, color);
            }
            else if (c == '1')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO + 1 * SJIS_FULLWIDTH_STEP, color, color);
            }
            else if (c == '2')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO + 2 * SJIS_FULLWIDTH_STEP, color, color);
            }
            else if (c == '3')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO + 3 * SJIS_FULLWIDTH_STEP, color, color);
            }
            else if (c == '4')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO + 4 * SJIS_FULLWIDTH_STEP, color, color);
            }
            else if (c == '5')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO + 5 * SJIS_FULLWIDTH_STEP, color, color);
            }
            else if (c == '6')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO + 6 * SJIS_FULLWIDTH_STEP, color, color);
            }
            else if (c == '7')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO + 7 * SJIS_FULLWIDTH_STEP, color, color);
            }
            else if (c == '8')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO + 8 * SJIS_FULLWIDTH_STEP, color, color);
            }
            else if (c == '9')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_ZERO + 9 * SJIS_FULLWIDTH_STEP, color, color);
            }
            else if (c == '-')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_MINUS, color, color);
            }
            else if (c == '.')
            {
                i++;
                pen += DrawGlyph(pixels, x + pen, y, SJIS_FULLWIDTH_PERIOD, color, color);
            }
            else if (c == '_')
            {
                i++;
                DrawGlyph(pixels, x + pen, y, SJIS_MIDDLE_DOT, color, color);
                pen += 1;
            }
            else if (c == 'L')
            {
                i++;
                number[0] = text[i];
                number[1] = text[i + 1];
                number[2] = 0;
                color = (unsigned char)atol(number);
                i += 2;
            }
            else if (c == 's')
            {
                i++;
                number[0] = text[i];
                number[1] = text[i + 1];
                number[2] = text[i + 2];
                number[3] = 0;
                row = &pixels[rowOffset + x + pen];
                for (int line = FONT_LINE_HEIGHT; line != 0; line--)
                {
                    column = pen;
                    unsigned short *out = row;
                    while ((float)column < (float)atol(number) * PAD_COLUMN_SCALE)
                    {
                        *out++ = color;
                        column++;
                    }
                    row += 0x200;
                }
                pen = (int)((float)atol(number) * PAD_COLUMN_SCALE);
                i += 3;
            }
            else if (c == '\n')
            {
                i++;
                pen = 0;
                y += FONT_LINE_HEIGHT;
                rowOffset += FONT_LINE_HEIGHT << 9;
            }
            else
            {
                unsigned short code = *(const unsigned short *)(text + i);
                pen += DrawGlyph(pixels, x + pen, y, code, color, color);
                i += 2;
            }
        } while (i < length);
        }
    }
    else if (length > 0)
    {
        do
        {
            char c = text[i];
            if (c == '-')
            {
                i++;
                pen += FIXED_MINUS_ADVANCE;
            }
            else if (c == 'L')
            {
                i++;
                number[0] = text[i];
                number[1] = text[i + 1];
                number[2] = 0;
                color = (unsigned char)atol(number);
                i += 2;
            }
            else if (c == 's')
            {
                i++;
                number[0] = text[i];
                number[1] = text[i + 1];
                number[2] = text[i + 2];
                number[3] = 0;
                pen = (unsigned char)(int)((float)atol(number) * FIXED_COLUMN_SCALE);
                i += 3;
            }
            else if (c == '\n')
            {
                i++;
                pen = 0;
                y += FONT_LINE_HEIGHT;
            }
            else
            {
                unsigned short code = *(const unsigned short *)(text + i);
                DrawGlyph(pixels, x + pen, y, code, color, color);
                pen += fixedAdvance;
                i += 2;
            }
        } while (i < length);
    }

    unsigned short *edge = &pixels[y * 0x200 + pen + x];
    for (int row = FONT_LINE_HEIGHT; row != 0; row--)
    {
        for (int k = pen; k < pen + 4; k++)
            edge[k - pen] = color;
        edge += 0x200;
    }
    return pen;
}

#define TEXT_TEXTURE_COUNT      0x20
#define TEXT_TEXTURE_KEY_LENGTH 0x80    // bytes of the text kept for the cache comparison
#define TEXT_SHADOW_SCALE       2.0f    // y offset per unit of scale
#define TEXEL_CENTRE            0.5f    // texture coordinates start half a texel in
#define TEXT_END_PADDING        4       // pixels of colour written after the text
#define TEXT_TEXTURE_PITCH      0x400   // bytes per row of the text textures (512 pixels)

// 0x1014fdd0 (Font_DrawCached). Draws `text` as a texture quad. The 32 text textures form a cache keyed by text,
// colour and alpha; a miss picks a random slot, renders the text into its buffer (colour reduced to 4 bits per
// channel) and uploads it. The quad is placed at (x, y + 2 * scale), spans width * scale by height * scale, and is
// cut to the clip rectangle by moving its texture coordinates with it. Every draw marks its slot as the newest and
// ages the rest. Returns the width of the text in pixels at `scale`.
int FontRenderer::DrawTextClipped(int x, int y, const char *text, int clipLeft, int clipTop, int clipRight, int clipBottom, int colour,
                                  float z, float alpha, float scale)
{
    y += (int)(scale * TEXT_SHADOW_SCALE);
    int slot;
    for (slot = 0; slot < TEXT_TEXTURE_COUNT; slot++)
    {
        if (alphas[slot] == alpha && colours[slot] == colour && strcmp(text, texts[slot]) == 0)
            goto found;
    }
    slot = rand() % TEXT_TEXTURE_COUNT;
    strncpy(texts[slot], text, TEXT_TEXTURE_KEY_LENGTH);
    colours[slot] = colour;
    alphas[slot] = alpha;
    widths[slot] = DrawStringToPixels(buffers[slot], 0, 0, text,
                                      (colour >> 4 & 0xf) | (colour >> 12 & 0xf) << 4 | (colour >> 12 & 0xff00), 0);
    heights[slot] = FONT_LINE_HEIGHT;
    DAT_118d0ba8[textures[slot]].Write16(0, 0, widths[slot] + TEXT_END_PADDING, FONT_LINE_HEIGHT, TEXT_TEXTURE_PITCH, buffers[slot]);
found:
    float textWidth = (float)widths[slot];
    float right = textWidth * scale + (float)x;
    float textHeight = (float)heights[slot];
    float bottom = textHeight * scale + (float)y;
    float u0 = TEXEL_CENTRE;
    float v0 = TEXEL_CENTRE;
    float u1 = textWidth - TEXEL_CENTRE;
    float v1 = textHeight - TEXEL_CENTRE;
    float uPerPixel = (u1 - TEXEL_CENTRE) / (right - (float)x);
    float vPerPixel = (v1 - TEXEL_CENTRE) / (bottom - (float)y);
    if (clipLeft > x)
    {
        u0 = (float)(clipLeft - x) * uPerPixel + TEXEL_CENTRE;
        x = clipLeft;
    }
    if ((float)clipRight < right)
    {
        u1 -= (right - (float)clipRight) * uPerPixel;
        right = (float)clipRight;
    }
    if (clipTop > y)
    {
        v0 = (float)(clipTop - y) * vPerPixel + TEXEL_CENTRE;
        y = clipTop;
    }
    if ((float)clipBottom < bottom)
    {
        v1 -= (bottom - (float)clipBottom) * vPerPixel;
        bottom = (float)clipBottom;
    }
    FUN_101616e0(textures[slot], x, y, (int)right, (int)bottom, u0, v0, u1, v1, z, alpha, 0);
    recency[slot] = TEXT_TEXTURE_COUNT;
    for (int other = 0; other < TEXT_TEXTURE_COUNT; other++)
    {
        if (other != slot && recency[slot] != 0)
            recency[slot]--;
    }
    return (int)((float)widths[slot] * scale);
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

#include "game_file.h"

extern "C" const char *FUN_1013f8a0(const char *name, int folder);   // builds the data path of `name`
#include "engine_memory.h"
#include "win_vars.h"
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
        alphas[i] = 0;
        recency[i] = 0;
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
        buffers[i] = (unsigned short *)buffer;
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
