// The engine's window object of the EN build (creates the Win32 window and forwards its messages). Declared once here.
#ifndef GURUMIN_EN_WINDOW_H
#define GURUMIN_EN_WINDOW_H

#include <windows.h>

// Offset in the window's extra data where the object pointer is kept.
const int WINDOW_OBJECT_SLOT = 8;

struct RenderWindow
{
    virtual LRESULT __stdcall WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);  // first vtable slot
};

extern "C" RenderWindow *g_renderWindow;   // EN 0x021a3498 (JP DAT_10f1a860): the window whose WindowProc receives the messages

// The Windows procedure registered for the window class. EN 0x7a34b0 (JP 0x1013e410).
extern "C" LRESULT CALLBACK Window_StaticProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

#endif
