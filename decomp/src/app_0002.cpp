#include <windows.h>

extern "C" int DAT_1020aee4;
extern "C" void __stdcall FUN_10001170(int, int, int, int, int, int);
extern "C" HWND FUN_10001570(void);

extern "C" int FUN_10001550(void)
{
    FUN_10001170(0, 0, 0, 0, 0, 0);
    return 1;
}

extern "C" LRESULT FUN_10001b40(void)
{
    if (DAT_1020aee4 != 0)
    {
        HWND wnd = FUN_10001570();
        if (wnd != 0)
            return SendMessageA(wnd, 0x400, 0x221, 0);
    }
    return 0;
}

extern "C" LRESULT FUN_10001b70(LPARAM param)
{
    if (DAT_1020aee4 != 0)
    {
        HWND wnd = FUN_10001570();
        if (wnd != 0)
            return SendMessageA(wnd, 0x400, 0x227, param);
    }
    return 0;
}


extern "C" int FUN_10001a90(int stream, const char *path, int a, int b, int c, int d)
{
    HWND window;
    if (DAT_1020aee4 != 0 && (window = FUN_10001570()) != 0)
    {
        SendMessageA(window, WM_USER, 0, stream);
        SendMessageA(window, WM_USER, 1, a);
        SendMessageA(window, WM_USER, 2, b);
        SendMessageA(window, WM_USER, 3, c);
        SendMessageA(window, WM_USER, 4, d);
        COPYDATASTRUCT data;
        data.dwData = 0;
        data.lpData = (void *)path;
        data.cbData = 0x100;
        SendMessageA(window, WM_COPYDATA, 0, (LPARAM)&data);
        SendMessageA(window, WM_USER, 0x207, 0);
    }
    return 0;
}
extern "C" LRESULT FUN_10001ba0(int stream)
{
    if (DAT_1020aee4 == 0)
        return 0;
    HWND wnd = FUN_10001570();
    if (wnd != 0)
    {
        SendMessageA(wnd, 0x400, 0, stream);
        return SendMessageA(wnd, 0x400, 0x201, 0);
    }
    return 0;
}

extern "C" LRESULT FUN_10001bf0(int stream, int fadeTime)
{
    if (DAT_1020aee4 == 0)
        return 0;
    HWND wnd = FUN_10001570();
    if (wnd != 0)
    {
        SendMessageA(wnd, 0x400, 0, stream);
        SendMessageA(wnd, 0x400, 1, fadeTime);
        return SendMessageA(wnd, 0x400, 0x212, 0);
    }
    return 0;
}