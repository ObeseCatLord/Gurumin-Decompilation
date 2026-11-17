// Text renderer: glyph metrics.
//
// The game draws all text from a pre-rendered Shift-JIS bitmap font (3ddata\ddfont8.dat, loaded by 0x101455d0).
// Each glyph is a 144-byte record = 72 little-endian 16-bit words: 17 rows of 4 words, each word holding five pixels of
// 3-bit coverage. Bits 9..13 of word 3 store the glyph's advance width (see tools/ddfont_tool.py and docs/MODDING_MAP.md).
// Replacing this font with a TrueType/Unicode one is a roadmap goal (docs/ROADMAP.md); these functions are the hooks.

struct FontRenderer
{
    char pad_0000[0x238c];
    unsigned short *glyphs;     // 0x238c: normal font (ddfont8.dat), 72 words per glyph
    unsigned short *altGlyphs;  // 0x2390: alternative font (ddfont8s.dat, the small variant), may be missing
    int useAlt;                 // 0x2394: 1 = draw with the alternative font when it is loaded

    int GlyphWidth(int x, int y, int color, unsigned short code, int a, int b);
};

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
