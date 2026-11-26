// Plain numbers of the menu digit sheet (EN build, EN-only: no JP twin found). Coordinates are 640x480 screen pixels. The
// digits are the 16x16 cells of texMenu1 (digit 0 at cell 3 of a 256-pixel sheet), one row per colour.
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "texture_mgr.h"

// Digit cell geometry on texMenu1.
const int NUMBER_CELL_PIXELS = 16;
const int NUMBER_FIRST_CELL = 3;
const float NUMBER_SHEET_SIZE = 256.0f;
// Width of a digit before the narrowing, and the narrowing; the text cell of a digit is NUMBER_TEXT_PIXELS wide.
const float NUMBER_CELL_WIDTH = 16.0f;
const float NUMBER_NARROWING = 0.7f;
const int NUMBER_TEXT_PIXELS = 8;
// The digit height is NUMBER_ASPECT times half its width.
const float NUMBER_HALF = 0.5f;
const float NUMBER_ASPECT = 1.5f;
const int NUMBER_BUFFER_SIZE = 256;
// Top and bottom of the two digit rows in texture coordinates (rows 13 and 12 of the sheet, in 16-pixel steps).
const float NUMBER_ROW_13_V0 = 0.8125f;
const float NUMBER_ROW_13_V1 = 0.875f;
const float NUMBER_ROW_12_V0 = 0.75f;
const float NUMBER_ROW_12_V1 = 0.8125f;

// Game::DrawNumberRow13: draws `value` in decimal centred on (x, y) with the digits of row 13 of texMenu1, each digit
// `scale` times the size of a plain digit. Called by the HUD counters (EN 0x5cf280).
int Game::DrawNumberRow13(int value, int x, int y, float z, float fade, float scale, float alpha)
{
    char text[NUMBER_BUFFER_SIZE];
    sprintf(text, "%d", value);
    int length = strlen(text);
    int halfWidth = (int)((float)(length * NUMBER_TEXT_PIXELS) * scale * NUMBER_NARROWING);
    int digitWidth = (int)(scale * NUMBER_CELL_WIDTH * NUMBER_NARROWING);
    int halfHeight = (int)((float)digitWidth * NUMBER_HALF * NUMBER_ASPECT);
    for (int i = 0; i < length; i++)
    {
        int digit = text[i] - '0';
        if (digit < 0)
            digit = 0;
        else if (digit > 9)
            digit = 9;
        int left = x - halfWidth + i * digitWidth;
        g_textures[texMenu1].DrawTinted(left, y - halfHeight, left + digitWidth, y + halfHeight,
                                        (float)((digit + NUMBER_FIRST_CELL) * NUMBER_CELL_PIXELS) / NUMBER_SHEET_SIZE,
                                        NUMBER_ROW_13_V0,
                                        (float)((digit + NUMBER_FIRST_CELL + 1) * NUMBER_CELL_PIXELS) / NUMBER_SHEET_SIZE,
                                        NUMBER_ROW_13_V1, z, fade, alpha, 0, ASPECT_STRETCH);
    }
    return 0;
}

// Game::DrawNumberRow12: the same as DrawNumberRow13 with the digits of row 12 of texMenu1. Called by the HUD counters
// (EN 0x5cf280).
int Game::DrawNumberRow12(int value, int x, int y, float z, float fade, float scale, float alpha)
{
    char text[NUMBER_BUFFER_SIZE];
    sprintf(text, "%d", value);
    int length = strlen(text);
    int halfWidth = (int)((float)(length * NUMBER_TEXT_PIXELS) * scale * NUMBER_NARROWING);
    int digitWidth = (int)(scale * NUMBER_CELL_WIDTH * NUMBER_NARROWING);
    int halfHeight = (int)((float)digitWidth * NUMBER_HALF * NUMBER_ASPECT);
    for (int i = 0; i < length; i++)
    {
        int digit = text[i] - '0';
        if (digit < 0)
            digit = 0;
        else if (digit > 9)
            digit = 9;
        int left = x - halfWidth + i * digitWidth;
        g_textures[texMenu1].DrawTinted(left, y - halfHeight, left + digitWidth, y + halfHeight,
                                        (float)((digit + NUMBER_FIRST_CELL) * NUMBER_CELL_PIXELS) / NUMBER_SHEET_SIZE,
                                        NUMBER_ROW_12_V0,
                                        (float)((digit + NUMBER_FIRST_CELL + 1) * NUMBER_CELL_PIXELS) / NUMBER_SHEET_SIZE,
                                        NUMBER_ROW_12_V1, z, fade, alpha, 0, ASPECT_STRETCH);
    }
    return 0;
}
