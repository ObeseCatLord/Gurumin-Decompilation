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

