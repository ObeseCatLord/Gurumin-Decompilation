// HUD numbers drawn from the digit strips of the HUD frame sheets: each digit is a 16x16 cell, digits 0..9 side by side,
// in rows (one per number style). Coordinates are 640x480 screen pixels. Ported from the matched JP source
// decomp/src/hud_digits.cpp; the EN build also places the digits for wide screens (aspect modes, advances divided by
// the screen width divisor).
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "hud_digits.h"
#include "render.h"
#include "texture_mgr.h"

// Size of the digit sheet in texels and of one digit cell in pixels.
const float DIGIT_SHEET_SIZE = 256.0f;
const int DIGIT_CELL = 16;
const int DIGIT_COUNT = 10;
const int TEXT_BUFFER_SIZE = 256;

// Top and bottom texel rows of each digit style, inset by half a texel so the cell does not bleed into its neighbours.
#define DIGIT_ROW_V0(top) (((top) + 0.5f) / DIGIT_SHEET_SIZE)
#define DIGIT_ROW_V1(top) (((top) + DIGIT_CELL - 0.5f) / DIGIT_SHEET_SIZE)
const float DIGIT_ROW_0 = 0.0f;
const float DIGIT_ROW_16 = 16.0f;
const float DIGIT_ROW_32 = 32.0f;
const float DIGIT_ROW_48 = 48.0f;
const float DIGIT_ROW_64 = 64.0f;
const float DIGIT_ROW_80 = 80.0f;

// Depth of the digits: Number_Draw draws at the front, the other two at 0.7.
const float DIGIT_Z_FRONT = 0.0f;
const float DIGIT_Z_BACK = 0.7f;

// `grow` is in eighths of a pixel: the cell is enlarged by grow * 8 pixels on every side (a pulse effect).
const float DIGIT_GROW_PIXELS = 8.0f;

// Advance width of each digit in pixels: "1" is narrow, 5/6/7/9 one pixel narrower than the rest.
#define DIGIT_ADVANCES {12, 9, 12, 12, 12, 11, 11, 11, 12, 11}

// Draws the digit `digit` from the sheet row starting at texel `rowTop` of `sheet` with its top-left corner at
// (left, top) in screen pixels, `grow` pixels larger on every side, at depth `z`.
#define DRAW_DIGIT(sheet, left, top, digit, rowTop, z, aspect)                                                       \
    g_textures[sheet].DrawTinted(                                                                                    \
        (int)((float)(left) - grow), (int)((float)(top) - grow), (int)((float)((left) + DIGIT_CELL) + grow),         \
        (int)((float)((top) + DIGIT_CELL) + grow), (float)((digit) * DIGIT_CELL) / DIGIT_SHEET_SIZE,                 \
        DIGIT_ROW_V0(rowTop), (float)((digit) * DIGIT_CELL + DIGIT_CELL) / DIGIT_SHEET_SIZE, DIGIT_ROW_V1(rowTop),   \
        z, alpha, 0.0f, grey, aspect)

// Draws `value` in decimal starting at (x, y) in digit style 1, 2 or 3 (sheet rows 0, 64 and 80 of the first HUD sheet)
// at the front depth, each digit `grow` eighths of a pixel larger on every side. Returns the width of the number in
// pixels. Called by the HUD.
extern "C" int Number_Draw(int x, int y, int value, float alpha, int style, float grow, float grey)
{
    int widths[DIGIT_COUNT] = DIGIT_ADVANCES;
    char text[TEXT_BUFFER_SIZE];
    y--;
    int advance = 0;
    sprintf(text, "%d", value);
    // The original measures the text first with an empty loop (the compiler keeps only its strlen).
    for (int i = 0; i < (int)strlen(text); i++)
    {
    }
    grow *= DIGIT_GROW_PIXELS;
    for (int i = 0; i < (int)strlen(text); i++)
    {
        int digit = text[i] - '0';
        if (style == 1)
            DRAW_DIGIT(g_game.texWaku01, x + advance, y, digit, DIGIT_ROW_0, DIGIT_Z_FRONT, ASPECT_CENTRE);
        else if (style == 2)
            DRAW_DIGIT(g_game.texWaku01, x + advance, y, digit, DIGIT_ROW_64, DIGIT_Z_FRONT, ASPECT_CENTRE);
        else if (style == 3)
            DRAW_DIGIT(g_game.texWaku01, x + advance, y, digit, DIGIT_ROW_80, DIGIT_Z_FRONT, ASPECT_LEFT);
        advance = (int)((float)widths[digit] / g_screenWidthDivisor + (float)advance);
    }
    return advance;
}

// Like Number_Draw but the number ends at x, in style 0, 1 or 2 (sheet rows 16, 32 and 48 of the second HUD sheet) at
// the back depth. Returns the pen position after the last digit relative to x (0 for a well-formed number).
extern "C" int Number_DrawRightAligned(int x, int y, int value, float alpha, int style, float grow, float grey)
{
    int widths[DIGIT_COUNT] = DIGIT_ADVANCES;
    char text[TEXT_BUFFER_SIZE];
    y--;
    int advance = 0;
    sprintf(text, "%d", value);
    // Start one number-width to the left so the number ends at x.
    for (int i = 0; i < (int)strlen(text); i++)
        advance -= widths[text[i] - '0'];
    grow *= DIGIT_GROW_PIXELS;
    for (int i = 0; i < (int)strlen(text); i++)
    {
        int digit = text[i] - '0';
        if (style == 0)
            DRAW_DIGIT(g_game.texWaku02, x + advance, y, digit, DIGIT_ROW_16, DIGIT_Z_BACK, ASPECT_LEFT);
        else if (style == 1)
            DRAW_DIGIT(g_game.texWaku02, x + advance, y, digit, DIGIT_ROW_32, DIGIT_Z_BACK, ASPECT_CENTRE);
        else if (style == 2)
            DRAW_DIGIT(g_game.texWaku02, x + advance, y, digit, DIGIT_ROW_48, DIGIT_Z_BACK, ASPECT_CENTRE);
        advance = (int)((float)widths[digit] / g_screenWidthDivisor + (float)advance);
    }
    return advance;
}

// Number_Draw with styles 0, 1 or 2 (sheet rows 16, 32 and 48 of the second HUD sheet) at the back depth. Returns the
// width of the number in pixels. Called by the HUD.
extern "C" int Number_DrawBack(int x, int y, int value, float alpha, int style, float grow, float grey)
{
    int widths[DIGIT_COUNT] = DIGIT_ADVANCES;
    char text[TEXT_BUFFER_SIZE];
    y--;
    int advance = 0;
    sprintf(text, "%d", value);
    // The original measures the text first with an empty loop (the compiler keeps only its strlen).
    for (int i = 0; i < (int)strlen(text); i++)
    {
    }
    grow *= DIGIT_GROW_PIXELS;
    for (int i = 0; i < (int)strlen(text); i++)
    {
        int digit = text[i] - '0';
        if (style == 0)
            DRAW_DIGIT(g_game.texWaku02, x + advance, y, digit, DIGIT_ROW_16, DIGIT_Z_BACK, ASPECT_CENTRE);
        else if (style == 1)
            DRAW_DIGIT(g_game.texWaku02, x + advance, y, digit, DIGIT_ROW_32, DIGIT_Z_BACK, ASPECT_CENTRE);
        else if (style == 2)
            DRAW_DIGIT(g_game.texWaku02, x + advance, y, digit, DIGIT_ROW_48, DIGIT_Z_BACK, ASPECT_CENTRE);
        advance = (int)((float)widths[digit] / g_screenWidthDivisor + (float)advance);
    }
    return advance;
}
