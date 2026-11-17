#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <imm.h>

typedef void (*WindowCallback)();

struct RenderWindow
{
    virtual LRESULT __stdcall WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    HWND window;
    int active;
    HINSTANCE instance;
    int compositionResult;
    int compositionEnded;
    WindowCallback onCompositionString;
    WindowCallback onCompositionResult;
    WindowCallback onCursorPos;
    WindowCallback onAttributes;
    char pad_0028[0x30];
    char title[0x64];
    char className[0x64];
    int wheelDelta;
    int middleCapture;
    HICON icon;
    void RegisterWindowClass(HINSTANCE instance);
    void Create(HINSTANCE instance);
};

extern "C" RenderWindow *DAT_10f1a860;
extern "C" int DAT_10f65514;
extern "C" int DAT_10202b24;
extern "C" int DAT_10202b28;

void RenderWindow::Create(HINSTANCE instance)
{
    middleCapture = 0;
    wheelDelta = 0;
    active = 0;
    RegisterWindowClass(instance);
    this->instance = instance;
    DAT_10f1a860 = this;
    if (DAT_10f65514 != 0)
        window = CreateWindowExA(0, className, title, WS_POPUP, CW_USEDEFAULT, 0, DAT_10202b24, DAT_10202b28, 0, 0, this->instance, 0);
    else
        window = CreateWindowExA(0, className, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, DAT_10202b24, DAT_10202b28, 0, 0, this->instance, 0);
}

extern "C" LRESULT CALLBACK FUN_1013e410(HWND, UINT, WPARAM, LPARAM);

void RenderWindow::RegisterWindowClass(HINSTANCE instance)
{
    WNDCLASSEXA wc;
    wc.hIcon = icon;
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = FUN_1013e410;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorA(0, IDC_ARROW);
    wc.lpszClassName = className;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW - 0);
    wc.lpszMenuName = 0;
    wc.hIconSm = 0;
    RegisterClassExA(&wc);
}
LRESULT __stdcall RenderWindow::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
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