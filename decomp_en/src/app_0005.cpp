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
