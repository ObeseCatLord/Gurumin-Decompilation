// Blended glyph drawing of the font renderer (EN build). JP twin: decomp/src/font_metrics.cpp (FontRenderer::DrawGlyph,
// the 0x101453e0 variant that writes plain alpha); this EN function blends the coverage with the colour instead.
#include "font_renderer.h"

// Glyph records are 72 words; the font starts at 0x8141 (0xe040 and up skip the unused codes up to 0x4043 further on).
const int GLYPH_WORDS = 72;
const int GLYPH_FIRST_CODE = 0x8141;
const int GLYPH_SECOND_BLOCK_CODE = 0xe040;
const int GLYPH_SECOND_BLOCK_FIRST = 0x4043;
const int GLYPH_LAST_INDEX = 0x108840;
const int GLYPH_FALLBACK_INDEX = 0x1c0;
const int GLYPH_WIDTH_WORD = 3;
const int GLYPH_WIDTH_SHIFT = 9;
const int GLYPH_WIDTH_MASK = 0x1f;
// Glyph rows: a glyph is 17 rows of PIXELS_PER_WORD pixels per 3-bit coverage cell, stored four words apart.
const int GLYPH_ROWS = 17;
const int GLYPH_ROW_WORD_STEP = 4;
const int GLYPH_WORD_END = 0x44;
const int GLYPH_COVERAGE_PER_WORD = 5;
const int GLYPH_COVERAGE_BITS = 3;
const int GLYPH_COVERAGE_MASK = 7;
// Width in pixels of the text buffer.
const int GLYPH_BUFFER_WIDTH = 0x280;
// Coverage scaling (coverage * 0x2000 + 0x1000) and the full channel value.
const int GLYPH_COVERAGE_SCALE = 0x2000;
const int GLYPH_COVERAGE_BIAS = 0x1000;
const int GLYPH_CHANNEL_MAX = 0xf;
const int GLYPH_OPAQUE_ALPHA = 0xfff0;

// Draws one Shift-JIS glyph (bytes in memory order) into the 640-pixel wide ARGB4444 `buffer` at (x, y): every pixel
// blends the three colour nibbles with the glyph coverage, and one column right of the glyph and one row below it
// are filled with `colour`. Returns the advance (glyph width + 1). Called while a text texture is drawn.
int FontRenderer::DrawGlyphBlended(unsigned short *buffer, int x, int y, unsigned short code, unsigned short colour, int unused)
{
    unsigned short *font = glyphs;
    unsigned char high = (unsigned char)(code >> 8);
    unsigned char low = (unsigned char)code;
    ((unsigned char *)&code)[0] = high;
    ((unsigned char *)&code)[1] = low;
    int index;
    if (code >= GLYPH_SECOND_BLOCK_CODE)
        index = (code - GLYPH_SECOND_BLOCK_FIRST) * GLYPH_WORDS;
    else
        index = (code - GLYPH_FIRST_CODE) * GLYPH_WORDS;
    if (index < 0 || index > GLYPH_LAST_INDEX)
        index = GLYPH_FALLBACK_INDEX;
    int width = (font[index + GLYPH_WIDTH_WORD] >> GLYPH_WIDTH_SHIFT) & GLYPH_WIDTH_MASK;
    int row = 0;
    unsigned short *line = &buffer[y * GLYPH_BUFFER_WIDTH + x];
    do
    {
        unsigned short *out = line;
        for (int column = 0; column < width; column++)
        {
            int coverage = ((font[column / GLYPH_COVERAGE_PER_WORD + row + index] >>
                             (column % GLYPH_COVERAGE_PER_WORD * GLYPH_COVERAGE_BITS)) & GLYPH_COVERAGE_MASK) *
                               GLYPH_COVERAGE_SCALE + GLYPH_COVERAGE_BIAS;
            int rest = GLYPH_CHANNEL_MAX - coverage;
            *out = (unsigned short)(((((colour >> 8 & GLYPH_CHANNEL_MAX) * rest / GLYPH_CHANNEL_MAX | GLYPH_OPAQUE_ALPHA) << 4 |
                                       (colour >> 4 & GLYPH_CHANNEL_MAX) * rest / GLYPH_CHANNEL_MAX) << 4)) |
                   (unsigned short)(((colour & GLYPH_CHANNEL_MAX) * rest + coverage * GLYPH_CHANNEL_MAX) / GLYPH_CHANNEL_MAX);
            out++;
        }
        row += GLYPH_ROW_WORD_STEP;
        line += GLYPH_BUFFER_WIDTH;
    } while (row < GLYPH_WORD_END);
    int advance = width + 1;
    unsigned short *edge = &buffer[y * GLYPH_BUFFER_WIDTH + width + x];
    for (int i = GLYPH_ROWS; i != 0; i--)
    {
        for (int k = width; k < advance; k++)
            edge[k - width] = (unsigned short)colour;
        edge += GLYPH_BUFFER_WIDTH;
    }
    unsigned short *bottom = &buffer[(y + GLYPH_ROWS) * GLYPH_BUFFER_WIDTH + width + x];
    for (int k = width; k < advance; k++)
        bottom[k - width] = (unsigned short)colour;
    return advance;
}
