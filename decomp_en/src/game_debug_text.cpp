// Debug text drawn from the bitmap font of menu sheet 1 (EN build; JP twin: Game::DrawDebugText, 0x100091d0).
#include <string.h>
#include "game.h"
#include "texture_mgr.h"

// The font sheet is a grid of glyph cells, DEBUG_FONT_COLUMNS per row, starting at the space character.
const int DEBUG_FONT_FIRST_CHAR = 0x20;
const int DEBUG_FONT_ROW_SHIFT = 5;                  // 32 glyphs per row of the sheet: row = code >> 5
const int DEBUG_FONT_COLUMN_MASK = 0x1f;             // column = code & 31
const int DEBUG_FONT_CELL_WIDTH = 8;                 // glyph cell width in pixels (also the pen advance)
const int DEBUG_FONT_CELL_HEIGHT = 16;               // glyph cell height in pixels
const float DEBUG_FONT_SHEET_SCALE = 1.0f / 256.0f;  // the sheet is 256 x 256 pixels
const float DEBUG_FONT_U_SIZE = 8.0f / 256.0f;       // width of a cell in texture coordinates
const float DEBUG_FONT_V_SIZE = 16.0f / 256.0f;      // height of a cell in texture coordinates

// Draws `text` at (x, y) in the 8x16 debug font, one cell per character; characters up to and including the space
// are skipped (they only move the pen). Called by the debug overlay. Returns 1.
int Game::DrawDebugText(const char *text, int x, int y)
{
    int length = strlen(text);
    for (int i = 0; i < length; i++)
    {
        int code = text[i] - DEBUG_FONT_FIRST_CHAR;
        if (code > 0)
        {
            float v = (float)((code >> DEBUG_FONT_ROW_SHIFT) * DEBUG_FONT_CELL_HEIGHT) * DEBUG_FONT_SHEET_SCALE;
            float u = (float)((code & DEBUG_FONT_COLUMN_MASK) * DEBUG_FONT_CELL_WIDTH) * DEBUG_FONT_SHEET_SCALE;
            g_textures[texMenu1].DrawCell(x, y, x + DEBUG_FONT_CELL_WIDTH, y + DEBUG_FONT_CELL_HEIGHT, u, v,
                                          u + DEBUG_FONT_U_SIZE, v + DEBUG_FONT_V_SIZE, 0, 1.0f, 0);
        }
        x += DEBUG_FONT_CELL_WIDTH;
    }
    return 1;
}
