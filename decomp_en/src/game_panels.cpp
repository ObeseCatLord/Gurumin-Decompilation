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

// Cursor distances above which the cursor moves at the next faster speed, and the speeds (pixels a frame).
const float CURSOR_FAR = 256.0f;
const float CURSOR_MIDDLE = 128.0f;
const float CURSOR_NEAR = 20.0f;
const float CURSOR_SPEED_FAR = 160.0f;
const float CURSOR_SPEED_MIDDLE = 80.0f;
const float CURSOR_SPEED_NEAR = 20.0f;

// Game::MoveCursor: moves the menu cursor towards its target: 160 pixels a frame when more than 256 away, 80 when more
// than 128, 20 when more than 20, and onto the target otherwise. Returns 1. Called by the menu and name-entry frames.
int Game::MoveCursor()
{
    Vec4 step;
    step.x = (float)(cursorTargetX - cursorX);
    step.y = (float)(cursorTargetY - cursorY);
    step.z = 0;
    float length = step.Length();   // the distance to the target, then reused as the speed
    if (length > CURSOR_FAR)
    {
        step.Copy(Vec3_Normalized(&step));
        length = CURSOR_SPEED_FAR;
        step.Copy(step * length);
        cursorX += (int)step.x;
        cursorY += (int)step.y;
        return 1;
    }
    if (length > CURSOR_MIDDLE)
    {
        step.Copy(Vec3_Normalized(&step));
        length = CURSOR_SPEED_MIDDLE;
        step.Copy(step * length);
        cursorX += (int)step.x;
        cursorY += (int)step.y;
        return 1;
    }
    if (length > CURSOR_NEAR)
    {
        step.Copy(Vec3_Normalized(&step));
        length = CURSOR_SPEED_NEAR;
        step.Copy(step * length);
        cursorX += (int)step.x;
        cursorY += (int)step.y;
        return 1;
    }
    cursorX = cursorTargetX;
    cursorY = cursorTargetY;
    return 1;
}

// Speech balloon cells on texWaku01: width, the height of a cap or middle row, the extra bottom cap height (a pixel
// short of two rows), and the texture rectangles of the lit top cap, the unlit top cap, a middle row and the bottom cap.
const int BALLOON_WIDTH = 0xa0;
const int BALLOON_CELL_HEIGHT = 0x20;
const int BALLOON_BOTTOM_HEIGHT = 0x3f;
const float BALLOON_U1 = 0.625f;
const float BALLOON_LIT_V0 = 0.875f;
const float BALLOON_UNLIT_V0 = 0.5f;
const float BALLOON_UNLIT_V1 = 0.625f;
const float BALLOON_MIDDLE_V0 = 0.625f;
const float BALLOON_MIDDLE_V1 = 0.75f;
const float BALLOON_BOTTOM_V0 = 0.75390625f;
const float BALLOON_BOTTOM_V1 = 0.875f;

// Game::DrawBalloon: draws a speech balloon from texWaku01 at (x, y): a top cap (lit or unlit), `rows` middle rows of 32
// pixels and a bottom cap, all 160 pixels wide, faded by `fade`. The parameters `a` and `b` are not used. Returns 0.
// Called by the item menu and the message windows.
int Game::DrawBalloon(int x, int y, int lit, int rows, int a, int b, float fade)
{
    Render_Command(RENDER_TEXTURE_ADDRESS, 0, 0);
    int right = x + BALLOON_WIDTH;
    if (lit != 0)
        g_textures[texWaku01].DrawTinted(x, y, right, y + BALLOON_CELL_HEIGHT, 0.0f, BALLOON_LIT_V0, BALLOON_U1, 1.0f, 0, fade, 0, 0,
                                         ASPECT_STRETCH);
    else
        g_textures[texWaku01].DrawTinted(x, y, right, y + BALLOON_CELL_HEIGHT, 0.0f, BALLOON_UNLIT_V0, BALLOON_U1, BALLOON_UNLIT_V1, 0,
                                         fade, 0, 0, ASPECT_STRETCH);
    for (int i = 0; i < rows; i++)
        g_textures[texWaku01].DrawTinted(x, y + BALLOON_CELL_HEIGHT + i * BALLOON_CELL_HEIGHT, right,
                                         y + 2 * BALLOON_CELL_HEIGHT + i * BALLOON_CELL_HEIGHT, 0.0f, BALLOON_MIDDLE_V0, BALLOON_U1,
                                         BALLOON_MIDDLE_V1, 0, fade, 0, 0, ASPECT_STRETCH);
    y += rows * BALLOON_CELL_HEIGHT;
    g_textures[texWaku01].DrawTinted(x, y + BALLOON_CELL_HEIGHT, right, y + BALLOON_BOTTOM_HEIGHT, 0.0f, BALLOON_BOTTOM_V0, BALLOON_U1,
                                     BALLOON_BOTTOM_V1, 0, fade, 0, 0, ASPECT_STRETCH);
    return 0;
}
