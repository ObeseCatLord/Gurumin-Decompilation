// EN window creation helpers (JP twin: window_create.cpp).

#include "window.h"

// The Windows procedure registered for the class: stores the RenderWindow pointer in the window's extra data on
// the first message and forwards every message to its (virtual) WindowProc.
extern "C" LRESULT CALLBACK Window_StaticProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (GetWindowLongA(window, WINDOW_OBJECT_SLOT) == 0)
        SetWindowLongA(window, WINDOW_OBJECT_SLOT, (LONG)g_renderWindow);
    return g_renderWindow->WindowProc(window, message, wParam, lParam);
}
