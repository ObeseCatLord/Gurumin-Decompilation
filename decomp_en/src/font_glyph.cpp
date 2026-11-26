// Glyph metrics of the font renderer (EN build). JP twin: decomp/src/font_metrics.cpp (FontRenderer::GlyphWidth).
#include "font_renderer.h"

// Shift-JIS characters whose width is fixed or taken from the alternative font, as the little-endian 16-bit values read
// from the text (lead byte in the low half).
const unsigned short GLYPH_YEN = 0x8f81;           // 0x818f: always measured with the alternative font
const unsigned short GLYPH_SPACE = 0x4081;         // 0x8140: ideographic space
const unsigned short GLYPH_APOSTROPHE = 0x6681;    // 0x8166
const unsigned short GLYPH_PERIOD = 0x4481;        // 0x8144
const unsigned short GLYPH_CARET = 0x4381;         // 0x8143
const unsigned short GLYPH_EXCLAMATION = 0x4981;   // 0x8149
// Advance (before the +1 every glyph gets) of the characters above.
const int GLYPH_SPACE_WIDTH = 5;
const int GLYPH_NARROW_WIDTH = 4;
const int GLYPH_EXCLAMATION_WIDTH = 10;
// Glyph records are 72 words; the font starts at 0x8141; records past 0x108840 words, or before the start, use record 4
// (index 0x1c0, a full-width space). The advance sits in bits 9..13 of word 3 of a record.
const int GLYPH_WORDS = 72;
const int GLYPH_FIRST_CODE = 0x8141;
const int GLYPH_LAST_INDEX = 0x108840;
const int GLYPH_FALLBACK_INDEX = 0x1c0;
const int GLYPH_WIDTH_WORD = 3;
const int GLYPH_WIDTH_SHIFT = 9;
const int GLYPH_WIDTH_MASK = 0x1f;

// Advance in pixels of one Shift-JIS character (the font's width field + 1, with fixed widths for a few punctuation
// marks). `code` arrives with its bytes in memory order and is swapped in place into a numeric code for the table lookup;
// the yen sign is measured with the alternative font even when that is not selected. Called by the string layout
// (DrawString) and the text measuring code.
int FontRenderer::GlyphWidth(int colour, int x, int y, unsigned short code, int unused, int unusedAgain)
{
    unsigned short *font = glyphs;
    unsigned short raw = code;
    if (raw == GLYPH_YEN)
        font = altGlyphs;
    unsigned char high = (unsigned char)(raw >> 8);
    unsigned char low = (unsigned char)raw;
    ((unsigned char *)&code)[1] = low;
    ((unsigned char *)&code)[0] = high;
    int index = (code - GLYPH_FIRST_CODE) * GLYPH_WORDS;
    if (index < 0 || index > GLYPH_LAST_INDEX)
        index = GLYPH_FALLBACK_INDEX;
    int width = (font[index + GLYPH_WIDTH_WORD] >> GLYPH_WIDTH_SHIFT) & GLYPH_WIDTH_MASK;
    if (raw == GLYPH_SPACE)
        width = GLYPH_SPACE_WIDTH;
    else if (raw == GLYPH_APOSTROPHE || raw == GLYPH_PERIOD || raw == GLYPH_CARET)
        width = GLYPH_NARROW_WIDTH;
    else if (raw == GLYPH_EXCLAMATION)
        width = GLYPH_EXCLAMATION_WIDTH;
    return width + 1;
}
