// EN window creation helpers (JP twin: window_create.cpp).

#include "render.h"
#include "window.h"

// The Windows procedure registered for the class: stores the RenderWindow pointer in the window's extra data on
// the first message and forwards every message to its (virtual) WindowProc.
extern "C" LRESULT CALLBACK Window_StaticProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (GetWindowLongA(window, WINDOW_OBJECT_SLOT) == 0)
        SetWindowLongA(window, WINDOW_OBJECT_SLOT, (LONG)g_renderWindow);
    return g_renderWindow->WindowProc(window, message, wParam, lParam);
}

// Resource id of the game icon.
const int GAME_ICON_RESOURCE = 0x80;

// Registers the window class (redraw on resize, arrow cursor, window-colour background, the game icon). Called by
// Create.
void RenderWindow::RegisterWindowClass(HINSTANCE instance)
{
    WNDCLASSEXA wc;
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Window_StaticProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance;
    wc.hIcon = LoadIconA(instance, MAKEINTRESOURCE(GAME_ICON_RESOURCE));
    wc.hCursor = LoadCursorA(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW - 0);
    wc.lpszMenuName = 0;
    wc.lpszClassName = className;
    wc.hIconSm = 0;
    RegisterClassExA(&wc);
}

// Registers the window class and creates the window at the configured size: a borderless popup in full screen, a
// normal overlapped window otherwise. Called once at start-up.
void RenderWindow::Create(HINSTANCE instance)
{
    middleCapture = 0;
    wheelDelta = 0;
    active = 0;
    RegisterWindowClass(instance);
    this->instance = instance;
    g_renderWindow = this;
    if (g_fullScreen != 0)
        window = CreateWindowExA(0, className, title, WS_POPUP, CW_USEDEFAULT, 0, g_deviceBackBufferWidth, g_deviceBackBufferHeight, 0, 0, this->instance, 0);
    else
        window = CreateWindowExA(0, className, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, g_deviceBackBufferWidth, g_deviceBackBufferHeight, 0, 0, this->instance, 0);
}
