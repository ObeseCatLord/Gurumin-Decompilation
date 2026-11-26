// The game window procedure (EN build). Ported from the JP GameWindow::WindowProc (APP.dll 0x1013e4f0, unmatched).
#include <windows.h>
#include <imm.h>

#include "ime.h"
#include "steam.h"
#include "window.h"

// Handles the messages the game cares about: quit, paint, focus changes (active), key presses (forwarded to the name entry
// callback), IME composition (forwarded to the callbacks set by the text input code), middle-button capture and the mouse
// wheel; it closes the IME's own window (WM_IME_SETCONTEXT) because the game draws the composition itself. Everything else
// goes to DefWindowProc. Pumps the Steam callbacks first. Called through the window class procedure (Window_StaticProc).
LRESULT __stdcall GameWindow::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    SteamAPI_RunCallbacks();
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        ValidateRect(window, 0);
        return 0;
    case WM_ACTIVATEAPP:
        active = wParam;
        break;
    case WM_KEYDOWN:
        if (g_textInputFlag != 0)
        {
            g_textInputFlag(wParam, lParam);
            return DefWindowProcA(window, message, wParam, lParam);
        }
        break;
    case WM_IME_ENDCOMPOSITION:
        onCompositionString();
        compositionEnded = 1;
        return 0;
    case WM_IME_COMPOSITION:
        if (lParam & GCS_COMPSTR)
        {
            onCompositionString();
            compositionEnded = 1;
        }
        if (lParam & GCS_RESULTSTR)
        {
            onCompositionResult();
            compositionResult = 1;
        }
        if (lParam & GCS_CURSORPOS)
            onCursorPos();
        if (lParam & GCS_COMPCLAUSE)
            onAttributes();
        return 0;
    case WM_MBUTTONDOWN:
        SetCapture(window);
        middleCapture = 1;
        break;
    case WM_MBUTTONUP:
        ReleaseCapture();
        middleCapture = 0;
        break;
    case WM_MOUSEWHEEL:
        wheelDelta += (short)HIWORD(wParam);
        break;
    case WM_IME_SETCONTEXT:
        SendMessageA(ImmGetDefaultIMEWnd(window), WM_CLOSE, 0, 0);
        return 0;
    }
    return DefWindowProcA(window, message, wParam, lParam);
}
