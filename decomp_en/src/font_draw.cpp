// Text drawing of the font renderer into its text buffers (EN build). The EN game lays out and draws bitmap-font
// text in one pass (the JP build only measured here).
#include <stdlib.h>
#include <string.h>

#include "font_renderer.h"

const int TEXT_BUFFER_WIDTH = 640;     // pixels per row of a text buffer
const int TEXT_ROW_HEIGHT = 17;        // pixel rows of one text line (also the glyph height in the buffer)
const int CODE_LENGTH_COLOUR = 2;      // digits of a "[nn" colour command
const int CODE_LENGTH_COLUMN = 3;      // digits of an "snnn" column command
const int FIXED_MINUS_ADVANCE = 9;     // advance of '-' in fixed-advance text
const int END_MARK_WIDTH = 4;          // pixels filled after the text as an end marker
const int UNDERSCORE_ADVANCE = 1; // '_' advances a single pixel

// Shift-JIS full-width forms of ASCII characters, as the little-endian 16-bit values read from the text.
const unsigned short SJIS_ZERO = 0x4f82;
const unsigned short SJIS_DIGIT_STEP = 0x0100;
const unsigned short SJIS_MINUS = 0x5d81;
const unsigned short SJIS_PERIOD = 0x4481;
const unsigned short SJIS_COLON = 0x4681;
const unsigned short SJIS_EXCLAMATION = 0x4981;
const unsigned short SJIS_QUESTION = 0x4881;
const unsigned short SJIS_LESS = 0x8381;
const unsigned short SJIS_GREATER = 0x8481;
const unsigned short SJIS_OPEN_PAREN = 0x6981;
const unsigned short SJIS_CLOSE_PAREN = 0x6a81;
const unsigned short SJIS_BAR = 0x6881;
const unsigned short SJIS_CARET = 0x4381;
const unsigned short SJIS_APOSTROPHE = 0x6681;
const unsigned short SJIS_UNDERSCORE = 0x4581;
const unsigned short SJIS_SPACE = 0x4081;
const unsigned short SJIS_UPPER_A = 0x6082;
const int SJIS_LOWER_A_OFFSET = -0x7e7e;    // 0x8182 as a 16-bit value: full-width 'a'

const float COLUMN_SCALE_PROPORTIONAL = 1.13636f;  // "snnn" columns of proportional text are in 1/1.13636 pixels
const float COLUMN_SCALE_FIXED = 1.11111f;         // the same for fixed-advance text

// FontRenderer::DrawStringToBuffer: draws `text` into the text buffer starting at pixel (x, y) and returns the
// pen position after the last character. Commands: "[nn" selects colour nn, "snnn" moves the pen to column nnn
// (padding with the colour in proportional text), '\n' starts a new line. Proportional text (fixedAdvance == 0)
// shows ASCII letters, digits and marks through their full-width glyphs; otherwise every double-byte character
// advances by fixedAdvance. A 4 pixel wide end marker in the current colour follows the text.
// Called by the text texture renderer (DrawText and the outline drawing).
int FontRenderer::DrawStringToBuffer(unsigned short *buffer, int x, int y, const char *text, int colour,
                                     int fixedAdvance)
{
    int length = strlen(text);
    int pen = 0;
    int i;
    char number[4];

    if (fixedAdvance == 0)
    {
        for (i = 0; i < length; )
        {
            char c = text[i];
            if (c == '0')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '1')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO + 1 * SJIS_DIGIT_STEP, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '2')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO + 2 * SJIS_DIGIT_STEP, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '3')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO + 3 * SJIS_DIGIT_STEP, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '4')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO + 4 * SJIS_DIGIT_STEP, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '5')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO + 5 * SJIS_DIGIT_STEP, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '6')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO + 6 * SJIS_DIGIT_STEP, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '7')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO + 7 * SJIS_DIGIT_STEP, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '8')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO + 8 * SJIS_DIGIT_STEP, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '9')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_ZERO + 9 * SJIS_DIGIT_STEP, colour, colour);
                pen += digitAdvance;
            }
            else if (c == '-')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_MINUS, colour, colour);
            }
            else if (c == '.')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_PERIOD, colour, colour);
            }
            else if (c == ':')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_COLON, colour, colour);
            }
            else if (c == '!')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_EXCLAMATION, colour, colour);
            }
            else if (c == '?')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_QUESTION, colour, colour);
            }
            else if (c == '<')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_LESS, colour, colour);
            }
            else if (c == '>')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_GREATER, colour, colour);
            }
            else if (c == '(')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_OPEN_PAREN, colour, colour);
            }
            else if (c == ')')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_CLOSE_PAREN, colour, colour);
            }
            else if (c == '|')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_BAR, colour, colour);
            }
            else if (c == '^')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_CARET, colour, colour);
            }
            else if (c == '\'')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_APOSTROPHE, colour, colour);
            }
            else if (c == '_')
            {
                i++;
                DrawGlyph(buffer, x + pen, y, SJIS_UNDERSCORE, colour, colour);
                pen += UNDERSCORE_ADVANCE;
            }
            else if (c == '[')
            {
                number[2] = 0;
                number[0] = text[i + 1];
                number[1] = text[i + 2];
                colour = (unsigned char)atol(number);
                i += 1 + CODE_LENGTH_COLOUR;
            }
            else if (c == 's' && text[i + 1] >= '0' && text[i + 1] <= '9')
            {
                i++;
                number[3] = 0;
                number[0] = text[i];
                number[1] = text[i + 1];
                number[2] = text[i + 2];
                unsigned short *rowStart = buffer + y * TEXT_BUFFER_WIDTH + pen + x;
                for (int row = 0; row < TEXT_ROW_HEIGHT; row++)
                {
                    int column = pen;
                    unsigned short *cell = rowStart;
                    while ((float)column < (float)atol(number) * COLUMN_SCALE_PROPORTIONAL)
                    {
                        *cell++ = (unsigned short)colour;
                        column++;
                    }
                    rowStart += TEXT_BUFFER_WIDTH;
                }
                pen = (int)((float)atol(number) * COLUMN_SCALE_PROPORTIONAL);
                i += CODE_LENGTH_COLUMN;
            }
            else if (c == ' ')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_SPACE, colour, colour);
            }
            else if (c >= 'A' && c <= 'Z')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, SJIS_UPPER_A + ((c - 'A') << 8), colour, colour);
            }
            else if (c >= 'a' && c <= 'z')
            {
                i++;
                pen += DrawGlyph(buffer, x + pen, y, ((c - 'a') << 8) + SJIS_LOWER_A_OFFSET, colour, colour);
            }
            else
            {
                pen += DrawGlyph(buffer, x + pen, y, *(const unsigned short *)(text + i), colour, colour);
                i += 2;
            }
        }
    }
    else
    {
        for (i = 0; i < length; )
        {
            char c = text[i];
            if (c == '-')
            {
                i++;
                pen += FIXED_MINUS_ADVANCE;
            }
            else if (c == '[')
            {
                i++;
                number[2] = 0;
                number[0] = text[i];
                number[1] = text[i + 1];
                colour = (unsigned char)atol(number);
                i += CODE_LENGTH_COLOUR;
            }
            else if (c == 's' && text[i + 1] >= '0' && text[i + 1] <= '9')
            {
                i++;
                number[3] = 0;
                number[0] = text[i];
                number[1] = text[i + 1];
                number[2] = text[i + 2];
                pen = (unsigned char)(int)((float)atol(number) * COLUMN_SCALE_FIXED);
                i += CODE_LENGTH_COLUMN;
            }
            else if (c == '\n')
            {
                i++;
                pen = 0;
                y += TEXT_ROW_HEIGHT;
            }
            else
            {
                DrawGlyph(buffer, x + pen, y, *(const unsigned short *)(text + i), colour, colour);
                pen += fixedAdvance;
                i += 2;
            }
        }
    }

    unsigned short *markRow = buffer + y * TEXT_BUFFER_WIDTH + pen + x;
    for (int row = 0; row < TEXT_ROW_HEIGHT; row++)
    {
        for (int column = pen; column < pen + END_MARK_WIDTH; column++)
            markRow[column - pen] = (unsigned short)colour;
        markRow += TEXT_BUFFER_WIDTH;
    }
    return pen;
}
