// Scroll marks and cursor of the menu windows (EN build). Ported from the JP source decomp/src/game_panels.cpp (APP.dll
// 0x100092c0, unmatched) and the EN bytes.
#include "font_renderer.h"
#include "game.h"
#include "texture_mgr.h"

// Scroll marks of the window lists in Shift-JIS: filled while the blink frame is at most MARK_BLINK_FRAME, outlined after
// that.
const char MARK_UP_FILLED[] = "\x81" "\xa3";       // black up triangle
const char MARK_UP_OUTLINE[] = "\x81" "\xa2";      // white up triangle
const char MARK_DOWN_FILLED[] = "\x81" "\xa5";     // black down triangle
const char MARK_DOWN_OUTLINE[] = "\x81" "\xa4";    // white down triangle
// Blink frame above which the outlined marks are drawn.
const int MARK_BLINK_FRAME = 5;
// Text colour (RGB) of the marks, text depth and opacity, and scale.
const int MARK_COLOUR = 0xffefcf;
const float MARK_DEPTH = 0.0f;
const float MARK_SCALE = 1.0f;
const float MARK_ALPHA = 0.9f;
// Pixels from the window's left edge to the marks, and from its top to the down mark's row.
const int MARK_X_OFFSET = 0x20;
const int MARK_DOWN_Y_OFFSET = 0x2c;
// Pixels from the window's top to the cursor row of a list window, and the cursor's left shift in a grid window.
const int LIST_CURSOR_Y_OFFSET = 0xc;
const int GRID_CURSOR_X_OFFSET = -0x10;
// Fade counter value of a fully faded in window.
const float WINDOW_FADE_FULL = 32.0f;

// Draws the marks of the three menu windows, last window first: for list windows an up mark when the list is scrolled, a
// down mark when an item follows the visible ones, and the cursor on the chosen item of the active window; for grid
// windows (m_0158 set) only the cursor at its column and row. `frame` blinks the marks and turns the cursor. Returns 0.
// Called by the menu drawing (0x5ccac0, 0x5cf280, 0x5e29a0).
int Game::DrawWindowMarks(int frame)
{
    for (int w = MENU_SLOT_COUNT - 1; w >= 0; w--)
    {
        if (windowRefresh[w] == 0)
            continue;
        if (m_0158[w] != 0)
        {
            if (windowVisible[w] != 0 && activeWindow == w && windowOpen[w] != 0)
            {
                Render_Command(RENDER_BLEND_MODE, BLEND_ADD, 0);
                DrawCursor(m_0170[w] * m_0164[w] + GRID_CURSOR_X_OFFSET + windowX[w], windowFlags[w] * m_017c[w] + windowY[w],
                           (float)windowFade[w] / WINDOW_FADE_FULL, frame);
                Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            }
        }
        else if (windowVisible[w] != 0)
        {
            if (m_0134[w] > 0)
            {
                if (frame > MARK_BLINK_FRAME)
                    textRenderer->DrawText(windowX[w] + MARK_X_OFFSET, windowY[w], MARK_UP_OUTLINE, MARK_COLOUR, MARK_DEPTH,
                                           MARK_SCALE, MARK_ALPHA);
                else
                    textRenderer->DrawText(windowX[w] + MARK_X_OFFSET, windowY[w], MARK_UP_FILLED, MARK_COLOUR, MARK_DEPTH,
                                           MARK_SCALE, MARK_ALPHA);
            }
            // Index of the last visible row, the list entry after it decides whether the down mark shows.
            int lastRow = windowLastRow[w];
            if (lastRow < MENU_ENTRY_SLOTS && labels[w][lastRow + m_0134[w] + 1][0] != 0)
            {
                if (frame > MARK_BLINK_FRAME)
                    textRenderer->DrawText(windowX[w] + MARK_X_OFFSET,
                                           windowFlags[w] * lastRow + MARK_DOWN_Y_OFFSET + windowY[w],
                                           MARK_DOWN_OUTLINE, MARK_COLOUR, MARK_DEPTH, MARK_SCALE, MARK_ALPHA);
                else
                    textRenderer->DrawText(windowX[w] + MARK_X_OFFSET,
                                           windowFlags[w] * lastRow + MARK_DOWN_Y_OFFSET + windowY[w],
                                           MARK_DOWN_FILLED, MARK_COLOUR, MARK_DEPTH, MARK_SCALE, MARK_ALPHA);
            }
            if (activeWindow == w && windowOpen[w] != 0)
            {
                Render_Command(RENDER_BLEND_MODE, BLEND_ADD, 0);
                DrawCursor(windowX[w], (windowCursor[w] - m_0134[w]) * windowFlags[w] + LIST_CURSOR_Y_OFFSET + windowY[w],
                           (float)windowFade[w] / WINDOW_FADE_FULL, frame);
                Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            }
        }
    }
    return 0;
}
