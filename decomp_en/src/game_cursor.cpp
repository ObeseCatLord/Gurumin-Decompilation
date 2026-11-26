// The game's own mouse cursor (EN build, EN-only: no JP twin found). Coordinates are 640x480 screen pixels; the cursor
// cells are on the menu sheet 1 texture.
#include <math.h>
#include <windows.h>

#include "dialogs.h"
#include "game_cursor.h"
#include "hud_fade.h"
#include "listbank.h"
#include "char_dispatch.h"
#include "mouse.h"
#include "sound_effects.h"
#include "texture_mgr.h"
#include "window.h"

// GameCursor_Draw: when the Windows cursor is inside the client area of the game window it is hidden and, while the game
// draws its own cursor (g_cursorDrawn) and no scene sound mode is running, the cursor image is drawn at (x, y): the plain
// arrow cell when g_cursorPlain is set; in game mode CURSOR_TURN_MODE (with nothing else going on and x + y below
// CURSOR_TURN_LIMIT) a 32x32 cell turned by CURSOR_ANGLE_BASE minus the heading of (g_cursorAimX, -g_cursorAimY); otherwise
// the arrow cell stretched with the screen. Outside the client area the Windows cursor is shown again. Returns 1. No direct
// caller (a hook).
extern "C" int GameCursor_Draw(int x, int y)
{
    MousePoint cursor;   // the same layout as POINT
    GetCursorPos((POINT *)&cursor);
    RECT client;
    GetClientRect(g_gameWindow.window, &client);
    POINT topLeft;
    topLeft.x = client.left;
    topLeft.y = client.top;
    POINT bottomRight;
    bottomRight.x = client.right;
    bottomRight.y = client.bottom;
    ClientToScreen(g_gameWindow.window, &topLeft);
    ClientToScreen(g_gameWindow.window, &bottomRight);
    if (g_mouse.PointInRect(&cursor, topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y) != 0)
    {
        while (ShowCursor(FALSE) >= 0)
            ;
        if (g_cursorDrawn != 0 && g_sceneSoundMode == 0)
        {
            float angle = CURSOR_ANGLE_BASE - (float)atan2(g_cursorAimX, -g_cursorAimY);
            int turning = 0;
            if (g_gameMode == CURSOR_TURN_MODE)
            {
                turning = 1;
                if (x + y >= CURSOR_TURN_LIMIT)
                    turning = 0;
                if (g_overlayOpen != 0)
                    turning = 0;
                if (g_sceneFlag9e33fa != 0)
                    turning = 0;
                if (g_updatesSuspended != 0)
                    turning = 0;
                if (g_gameBuiltinsRegistered == 0)
                    turning = 0;
            }
            if (g_cursorPlain != 0)
                g_textures[g_dialogGame.texMenu1].DrawTinted(x, y, x + CURSOR_PLAIN_SIZE, y + CURSOR_PLAIN_SIZE,
                                                             CURSOR_TURN_U0, CURSOR_ROW_V0, CURSOR_TURN_U1, CURSOR_ROW_V1, 0,
                                                             1.0f, 0, 0, ASPECT_STRETCH);
            else if (turning)
                g_textures[g_dialogGame.texMenu1].DrawTinted(x - CURSOR_TURN_HALF, y - CURSOR_TURN_HALF, x + CURSOR_TURN_HALF,
                                                             y + CURSOR_TURN_HALF, CURSOR_TURN_U0, CURSOR_ROW_V0,
                                                             CURSOR_TURN_U1, CURSOR_ROW_V1, 0, 1.0f, angle, 0, ASPECT_STRETCH);
            else
                g_textures[g_dialogGame.texMenu1].DrawTinted(x - CURSOR_ARROW_LEFT, y, x + CURSOR_PLAIN_SIZE,
                                                             y + CURSOR_ARROW_HEIGHT, CURSOR_ARROW_U0, CURSOR_ARROW_V0,
                                                             CURSOR_ARROW_U1, CURSOR_ARROW_V1, 0, 1.0f, 0, 0, ASPECT_CENTRE);
        }
    }
    else
    {
        while (ShowCursor(TRUE) < 0)
            ;
    }
    return 1;
}

// GameCursor_DrawAtTarget: the cursor of GameCursor_Draw for the mode that turns it towards a point: the angle is
// CURSOR_ANGLE_BASE minus atan2 of the offset from (g_cursorTargetX, g_cursorTargetY), and the turning cell shows in game
// mode CURSOR_TARGET_MODE when nothing else is going on. Returns 1. No direct caller (a hook).
extern "C" int GameCursor_DrawAtTarget(int x, int y)
{
    MousePoint cursor;   // the same layout as POINT
    GetCursorPos((POINT *)&cursor);
    RECT client;
    GetClientRect(g_gameWindow.window, &client);
    POINT topLeft;
    topLeft.x = client.left;
    topLeft.y = client.top;
    POINT bottomRight;
    bottomRight.x = client.right;
    bottomRight.y = client.bottom;
    ClientToScreen(g_gameWindow.window, &topLeft);
    ClientToScreen(g_gameWindow.window, &bottomRight);
    if (g_mouse.PointInRect(&cursor, topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y) != 0)
    {
        while (ShowCursor(FALSE) >= 0)
            ;
        if (g_cursorHidden == 0 && g_cursorDrawn != 0 && g_sceneSoundMode == 0)
        {
            float angle = CURSOR_ANGLE_BASE - (float)atan2((double)(x - g_cursorTargetX), (double)(y - g_cursorTargetY));
            int turning = 0;
            if (g_gameMode == CURSOR_TARGET_MODE)
            {
                turning = 1;
                if (x + y >= CURSOR_TURN_LIMIT)
                    turning = 0;
                if (g_overlayOpen != 0)
                    turning = 0;
                if (g_sceneFlag9e33fa != 0)
                    turning = 0;
                if (g_updatesSuspended != 0)
                    turning = 0;
                if (g_gameBuiltinsRegistered == 0)
                    turning = 0;
                if (!(UI_STATE_TURN_RANGE < (unsigned int)g_uiState90812c - UI_STATE_TURN_FIRST))
                    turning = 0;
                if (g_overlayModelValueD != 0)
                    turning = 0;
            }
            if (g_cursorPlain != 0)
                g_textures[g_dialogGame.texMenu1].DrawTinted(x, y, x + CURSOR_PLAIN_SIZE, y + CURSOR_PLAIN_SIZE,
                                                             CURSOR_TURN_U0, CURSOR_ROW_V0, CURSOR_TURN_U1, CURSOR_ROW_V1, 0,
                                                             1.0f, 0, 0, ASPECT_STRETCH);
            else if (turning)
                g_textures[g_dialogGame.texMenu1].DrawTinted(x - CURSOR_TURN_HALF, y - CURSOR_TURN_HALF, x + CURSOR_TURN_HALF,
                                                             y + CURSOR_TURN_HALF, CURSOR_TURN_U0, CURSOR_ROW_V0,
                                                             CURSOR_TURN_U1, CURSOR_ROW_V1, 0, 1.0f, angle, 0, ASPECT_STRETCH);
            else
                g_textures[g_dialogGame.texMenu1].DrawTinted(x - CURSOR_ARROW_LEFT, y, x + CURSOR_PLAIN_SIZE,
                                                             y + CURSOR_ARROW_HEIGHT, CURSOR_ARROW_U0, CURSOR_ARROW_V0,
                                                             CURSOR_ARROW_U1, CURSOR_ARROW_V1, 0, 1.0f, 0, 0, ASPECT_CENTRE);
        }
    }
    else
    {
        while (ShowCursor(TRUE) < 0)
            ;
    }
    return 1;
}
