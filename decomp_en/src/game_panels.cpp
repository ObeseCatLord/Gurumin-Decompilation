// Game drawing helpers for menus and the HUD (EN build). Coordinates are 640x480 screen pixels; UV coordinates index
// the menu texture sheets. Ported from the matched JP source decomp/src/game_panels.cpp.
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "texture_mgr.h"

// Fill alpha of a panel relative to its frame alpha.
const float PANEL_FILL_ALPHA = 0.5f;
// Cursor cell on texMenu1 (a 32x32 cell, in 256-pixel sheet units) and its offset from the cursor position.
const float CURSOR_U0 = 1.0f / 256;
const float CURSOR_V0 = 97.0f / 256;
const float CURSOR_U1 = 31.0f / 256;
const float CURSOR_V1 = 127.0f / 256;
// Frames the cursor pulse takes to turn once; the cursor starts turned by -CURSOR_PULSE_BASE frames.
const float CURSOR_TURN_FRAMES = 20.0f;
const int CURSOR_PULSE_BASE = 5;
// Digit cell geometry on texMenu1: 16-pixel cells in a 256-pixel sheet, digit 0 at cell 3; characters are 8 pixels
// wide before scaling.
const int DIGIT_CELL_PIXELS = 16;
const int DIGIT_FIRST_CELL = 3;
const float SHEET_SIZE = 256.0f;
const float DIGIT_LABEL_V0 = 0.625f;
const float DIGIT_LABEL_V1 = 0.6875f;
const float DIGIT_LABEL_U0 = 0.1875f;
const float DIGIT_LABEL_U1 = 0.4453125f;
const float DIGIT_ROW_V0 = 0.6875f;
const float DIGIT_ROW_V1 = 0.75f;
const float DIGIT_CELL_WIDTH = 16.0f;       // width of a digit before the 0.7 narrowing
const float DIGIT_NARROWING = 0.7f;         // digits are drawn 70 % as wide as their text cell
const float HALF = 0.5f;
const float DIGIT_ASPECT = 1.5f;            // digit height is 1.5 times half its width
const int DIGIT_TEXT_PIXELS = 8;
// The label of a labelled number is this many digit cells wide.
const int LABEL_CELLS = 3;
const int TEXT_BUFFER_SIZE = 256;

// Draws a framed panel at (x, y, w, h): the frame from texMenu14 and the half-transparent fill from texMenu15.
// Called by the menu and message window drawing.
int Game::DrawPanel(int x, int y, int w, int h, float z, float alpha)
{
    Render_Command(RENDER_TEXTURE_ADDRESS, 0, 0);
    g_textures[texMenu14].DrawFrame(x, y, x + w, y + h, z, alpha, 1, 0);
    g_textures[texMenu15].DrawFill(x, y, x + w, y + h, z, alpha * PANEL_FILL_ALPHA, 1, 0);
    return 0;
}

// Same as DrawPanel with the frame from texMenu4 (the alternative panel style).
int Game::DrawPanelAlt(int x, int y, int w, int h, float z, float alpha)
{
    Render_Command(RENDER_TEXTURE_ADDRESS, 0, 0);
    g_textures[texMenu4].DrawFrameAlt(x, y, x + w, y + h, z, alpha, 1, 0);
    g_textures[texMenu15].DrawFillAlt(x, y, x + w, y + h, z, alpha * PANEL_FILL_ALPHA, 1, 0);
    return 0;
}

// Stores the cursor target position and draws the menu cursor (a 32x32 cell of texMenu1) at the current position,
// turning with `frame`. Called by the window mark drawing.
int Game::DrawCursor(int x, int y, float fade, int frame)
{
    cursorTargetX = x;
    cursorTargetY = y;
    Render_Command(RENDER_BLEND_MODE, BLEND_ADD, 0);
    g_textures[texMenu1].DrawTinted(cursorX - 4, cursorY + 2, cursorX + 0x1c, cursorY + 0x22, CURSOR_U0, CURSOR_V0,
                                    CURSOR_U1, CURSOR_V1, 0, fade, (frame - CURSOR_PULSE_BASE) / CURSOR_TURN_FRAMES, 0, ASPECT_CENTRE);
    Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    return 1;
}

// Draws `value` in decimal centred on (x, y) with a label cell block in front, using the digit cells of texMenu1
// (the label fills the first three cells, the digits follow). Called by the HUD counters.
int Game::DrawLabelledNumber(int value, int x, int y, float z, float fade, float scale, float alpha)
{
    char text[TEXT_BUFFER_SIZE];
    sprintf(text, "%d", value);
    int length = strlen(text) + 2;
    int halfWidth = (int)((float)(length * DIGIT_TEXT_PIXELS) * scale * DIGIT_NARROWING);
    int digitWidth = (int)(scale * DIGIT_CELL_WIDTH * DIGIT_NARROWING);
    int halfHeight = (int)((float)digitWidth * HALF * DIGIT_ASPECT);
    g_textures[texMenu1].DrawTinted(x - halfWidth, y - halfHeight, x - halfWidth + digitWidth * LABEL_CELLS,
                                    y + halfHeight, DIGIT_LABEL_U0, DIGIT_LABEL_V0, DIGIT_LABEL_U1, DIGIT_LABEL_V1, z,
                                    fade, alpha, 0, ASPECT_STRETCH);
    for (int i = 2; i < length; i++)
    {
        int digit = text[i - 2] - '0';
        if (digit < 0)
            digit = 0;
        else if (digit > 9)
            digit = 9;
        int left = x - halfWidth + i * digitWidth;
        g_textures[texMenu1].DrawTinted(left, y - halfHeight, left + digitWidth, y + halfHeight,
                                        (float)((digit + DIGIT_FIRST_CELL) * DIGIT_CELL_PIXELS) / SHEET_SIZE,
                                        DIGIT_ROW_V0,
                                        (float)((digit + DIGIT_FIRST_CELL + 1) * DIGIT_CELL_PIXELS) / SHEET_SIZE,
                                        DIGIT_ROW_V1, z, fade, alpha, 0, ASPECT_STRETCH);
    }
    return 0;
}
