// HUD numbers drawn from the digit strip of texWaku01: each digit is a 16x16 cell, digits 0..9 side by side, in
// three rows (one per number style). Coordinates are 640x480 screen pixels. All three functions are called by
// FUN_1008c680 (the HUD); Number_DrawRightAligned also by FUN_10021e70.

#include <stdio.h>
#include <string.h>
#include "game.h"
#include "texture_object.h"
#include "hud_digits.h"

// Size of the digit sheet in texels and of one digit cell in pixels.
#define DIGIT_SHEET_SIZE 256.0f
#define DIGIT_CELL 16

// Top and bottom texel rows of each digit style, inset by half a texel so the cell does not bleed into its
// neighbours.
#define DIGIT_ROW_V0(top) (((top) + 0.5f) / DIGIT_SHEET_SIZE)
#define DIGIT_ROW_V1(top) (((top) + DIGIT_CELL - 0.5f) / DIGIT_SHEET_SIZE)
#define DIGIT_ROW_0 0.0f
#define DIGIT_ROW_16 16.0f
#define DIGIT_ROW_32 32.0f
#define DIGIT_ROW_48 48.0f
#define DIGIT_ROW_64 64.0f
#define DIGIT_ROW_80 80.0f

// Depth of the digits: FUN_10012990 draws at the front (0), the other two at 0.7.
#define DIGIT_Z_FRONT 0.0f
#define DIGIT_Z_BACK 0.7f

// `grow` is in eighths of a pixel: the cell is enlarged by grow * 8 pixels on every side (a pulse effect).
#define DIGIT_GROW_PIXELS 8.0f

// Draws the digit `digit` from the sheet row starting at texel `rowTop` with its top-left corner at (left, top) in
// screen pixels, `grow` pixels larger on every side, at depth `z`.
#define DRAW_DIGIT(left, top, digit, rowTop, z)                                                                      \
    DAT_118d0ba8[DAT_104b7200.texWaku01].DrawTinted(                                                                \
        (int)((float)(left) - grow), (int)((float)(top) - grow), (int)((float)((left) + DIGIT_CELL) + grow),       \
        (int)((float)((top) + DIGIT_CELL) + grow), (float)((digit) * DIGIT_CELL) / DIGIT_SHEET_SIZE,               \
        DIGIT_ROW_V0(rowTop), (float)((digit) * DIGIT_CELL + DIGIT_CELL) / DIGIT_SHEET_SIZE, DIGIT_ROW_V1(rowTop), \
        z, alpha, 0.0f, grey)

// Number_Draw: draws `value` in decimal starting at (x, y) in digit style 1, 2 or 3 (sheet rows 0, 64 and 80; any
// other style draws nothing but still advances) at the front depth, each digit `grow` eighths of a pixel larger on
// every side. Returns the width of the number in pixels (the sum of the digits' advance widths).
extern "C" int FUN_10012990(int x, int y, int value, float alpha, int style, float grow, float grey)
{
    int widths[10];
    char text[256];
    int i;
    y--;
    int advance = 0;
    // Advance width of each digit in pixels: "1" is narrow, 5/6/7/9 one pixel narrower than the rest.
    widths[0] = 12;
    widths[1] = 9;
    widths[2] = 12;
    widths[3] = 12;
    widths[4] = 12;
    widths[5] = 11;
    widths[6] = 11;
    widths[7] = 11;
    widths[8] = 12;
    widths[9] = 11;
    sprintf(text, "%d", value);
    // Empty measuring pass (kept by the compiler because strlen is re-evaluated every iteration).
    for (i = 0; i < (int)strlen(text); i++)
    {
    }

    grow *= DIGIT_GROW_PIXELS;
    for (i = 0; i < (int)strlen(text); i++)
    {
        int digit = text[i] - '0';
        if (style == 1)
            DRAW_DIGIT(x + advance, y, digit, DIGIT_ROW_0, DIGIT_Z_FRONT);
        else if (style == 2)
            DRAW_DIGIT(x + advance, y, digit, DIGIT_ROW_64, DIGIT_Z_FRONT);
        else if (style == 3)
            DRAW_DIGIT(x + advance, y, digit, DIGIT_ROW_80, DIGIT_Z_FRONT);
        advance += widths[digit];
    }
    return advance;
}

// Number_DrawRightAligned: like Number_Draw but the number ends at x, in style 0, 1 or 2 (sheet rows 16, 32 and 48)
// at depth 0.7. Returns the pen position after the last digit relative to x (0 for a well-formed number).
extern "C" int FUN_10012d90(int x, int y, int value, float alpha, int style, float grow, float grey)
{
    int widths[10];
    char text[256];
    int i;
    y--;
    int advance = 0;
    // Advance width of each digit in pixels: "1" is narrow, 5/6/7/9 one pixel narrower than the rest.
    widths[0] = 12;
    widths[1] = 9;
    widths[2] = 12;
    widths[3] = 12;
    widths[4] = 12;
    widths[5] = 11;
    widths[6] = 11;
    widths[7] = 11;
    widths[8] = 12;
    widths[9] = 11;
    sprintf(text, "%d", value);
    // Start one number-width to the left so the number ends at x.
    for (i = 0; i < (int)strlen(text); i++)
        advance -= widths[text[i] - '0'];

    grow *= DIGIT_GROW_PIXELS;
    for (i = 0; i < (int)strlen(text); i++)
    {
        int digit = text[i] - '0';
        if (style == 0)
            DRAW_DIGIT(x + advance, y, digit, DIGIT_ROW_16, DIGIT_Z_BACK);
        else if (style == 1)
            DRAW_DIGIT(x + advance, y, digit, DIGIT_ROW_32, DIGIT_Z_BACK);
        else if (style == 2)
            DRAW_DIGIT(x + advance, y, digit, DIGIT_ROW_48, DIGIT_Z_BACK);
        advance += widths[digit];
    }
    return advance;
}

// Number_DrawBack: Number_Draw with styles 0, 1 or 2 (sheet rows 16, 32 and 48) at depth 0.7. Returns the width of
// the number in pixels.
extern "C" int FUN_100131b0(int x, int y, int value, float alpha, int style, float grow, float grey)
{
    int widths[10];
    char text[256];
    int i;
    y--;
    int advance = 0;
    // Advance width of each digit in pixels: "1" is narrow, 5/6/7/9 one pixel narrower than the rest.
    widths[0] = 12;
    widths[1] = 9;
    widths[2] = 12;
    widths[3] = 12;
    widths[4] = 12;
    widths[5] = 11;
    widths[6] = 11;
    widths[7] = 11;
    widths[8] = 12;
    widths[9] = 11;
    sprintf(text, "%d", value);
    // Empty measuring pass (kept by the compiler because strlen is re-evaluated every iteration).
    for (i = 0; i < (int)strlen(text); i++)
    {
    }

    grow *= DIGIT_GROW_PIXELS;
    for (i = 0; i < (int)strlen(text); i++)
    {
        int digit = text[i] - '0';
        if (style == 0)
            DRAW_DIGIT(x + advance, y, digit, DIGIT_ROW_16, DIGIT_Z_BACK);
        else if (style == 1)
            DRAW_DIGIT(x + advance, y, digit, DIGIT_ROW_32, DIGIT_Z_BACK);
        else if (style == 2)
            DRAW_DIGIT(x + advance, y, digit, DIGIT_ROW_48, DIGIT_Z_BACK);
        advance += widths[digit];
    }
    return advance;
}
