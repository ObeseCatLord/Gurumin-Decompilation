// Mouse cursor placement (JP twin: app_0005.cpp).

#include <windows.h>
#include "mouse.h"

// Width and height of the game's logical screen the cursor position is given in.
static const int GAME_SCREEN_WIDTH = 640;
static const int GAME_SCREEN_HEIGHT = 480;

// Moves the Windows cursor to game position (x, y) in 640x480 coordinates, scaled to the client area. Called when
// the game warps the cursor (menus that snap it to a choice). Returns 0.
int Mouse::SetCursorGamePos(int x, int y)
{
    POINT origin;
    origin.x = 0;
    origin.y = 0;
    ClientToScreen((HWND)window, &origin);
    RECT client;
    GetClientRect((HWND)window, &client);
    SetCursorPos(client.right * x / GAME_SCREEN_WIDTH + origin.x, client.bottom * y / GAME_SCREEN_HEIGHT + origin.y);
    return 0;
}

// Mouse::ReportCursor: when the mouse is tracked, converts the Windows cursor position to the client area scaled to
// 640x480 and passes it to the cursor callback (if any). Returns 0. Called every frame by the menus.
int Mouse::ReportCursor()
{
    if (moved == 0)
        return 0;
    POINT cursor;
    GetCursorPos(&cursor);
    POINT origin;
    origin.x = 0;
    origin.y = 0;
    ClientToScreen((HWND)window, &origin);
    RECT client;
    GetClientRect((HWND)window, &client);
    if (client.right != 0 && client.bottom != 0)
    {
        cursor.x = (cursor.x - origin.x) * GAME_SCREEN_WIDTH / client.right;
        cursor.y = (cursor.y - origin.y) * GAME_SCREEN_HEIGHT / client.bottom;
        if (cursorCallback != 0)
            cursorCallback(cursor.x, cursor.y);
    }
    return 0;
}
