// The engine's window object of the EN build (creates the Win32 window and forwards its messages). Declared once here.
#ifndef GURUMIN_EN_WINDOW_H
#define GURUMIN_EN_WINDOW_H

#include <windows.h>

// Offset in the window's extra data where the object pointer is kept.
const int WINDOW_OBJECT_SLOT = 8;

typedef void (*WindowCallback)();   // an IME event callback, called by the window procedure

// The engine's window; only the members EN code has used (the rest of the object is not declared).
struct RenderWindow
{
    virtual LRESULT __stdcall WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);  // first vtable slot
    HWND window;                         // 0x04: the Win32 window
    int active;                          // 0x08: not yet named
    HINSTANCE instance;                  // 0x0c: module instance
    int compositionResult;               // 0x10: an IME result string arrived
    int compositionEnded;                // 0x14: the IME composition string changed
    WindowCallback onCompositionString;  // 0x18: IME callbacks installed by TextInput_Begin
    WindowCallback onCompositionResult;  // 0x1c
    WindowCallback onCursorPos;          // 0x20
    WindowCallback onAttributes;         // 0x24
    char m_0028[0x30];                   // 0x28: not yet named
    char title[0x64];                    // 0x58: window title
    char className[0x64];                // 0xbc: window class name
    int wheelDelta;                      // 0x120: mouse wheel movement collected by the window procedure
    int middleCapture;                   // 0x124: the middle button holds the mouse capture

    void RegisterWindowClass(HINSTANCE instance);   // EN 0x7a3a50 (JP 0x101489c0): registers the window class
    void Create(HINSTANCE instance);                // EN 0x77d420 (JP 0x10148a40): registers the class and creates the window
};

extern "C" int g_fullScreen;   // EN 0x021e4c40 (JP DAT_10f65514): non-zero = full screen (borderless popup window)
extern "C" RenderWindow g_gameWindow;   // EN 0x00909348 (JP DAT_10f65698): the game window

extern "C" RenderWindow *g_renderWindow;   // EN 0x021a3498 (JP DAT_10f1a860): the window whose WindowProc receives the messages

// The Windows procedure registered for the window class. EN 0x7a34b0 (JP 0x1013e410).
extern "C" LRESULT CALLBACK Window_StaticProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

#endif
