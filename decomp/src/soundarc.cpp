// Messages to soundarc.dll, the separate sound process (docs/MODDING_MAP.md "Sound architecture").
// Every command is SendMessage(soundarc window, WM_USER, code, value); arguments are sent first with codes 0 and 1.
// DAT_1020aee4 = sound enabled.

#include <windows.h>
#include <stdio.h>

extern "C" int DAT_1020aee4;
extern "C" char DAT_1020ade0[];
extern "C" HWND FUN_10001570(void);

// Command 0x230 with two arguments.
extern "C" LRESULT FUN_10001660(int first, int second)
{
    if (DAT_1020aee4 == 0)
        return 0;
    HWND wnd = FUN_10001570();
    if (wnd != 0)
    {
        SendMessageA(wnd, 0x400, 0, first);
        SendMessageA(wnd, 0x400, 1, second);
        return SendMessageA(wnd, 0x400, 0x230, 0);
    }
    return 0;
}

// Command 0x201: stop stream `value`.
extern "C" LRESULT FUN_10001ba0(int value)
{
    if (DAT_1020aee4 == 0)
        return 0;
    HWND wnd = FUN_10001570();
    if (wnd != 0)
    {
        SendMessageA(wnd, 0x400, 0, value);
        return SendMessageA(wnd, 0x400, 0x201, 0);
    }
    return 0;
}

// Command 0x212: fade out stream `first` over `second` ms.
extern "C" LRESULT FUN_10001bf0(int first, int second)
{
    if (DAT_1020aee4 == 0)
        return 0;
    HWND wnd = FUN_10001570();
    if (wnd != 0)
    {
        SendMessageA(wnd, 0x400, 0, first);
        SendMessageA(wnd, 0x400, 1, second);
        return SendMessageA(wnd, 0x400, 0x212, 0);
    }
    return 0;
}

// Command 0x224 with one value.
extern "C" LRESULT FUN_10001c50(LPARAM param)
{
    if (DAT_1020aee4 != 0)
    {
        HWND wnd = FUN_10001570();
        if (wnd != 0)
            return SendMessageA(wnd, 0x400, 0x224, param);
    }
    return 0;
}

// Command 0x225 with one value.
extern "C" LRESULT FUN_10001c80(LPARAM param)
{
    if (DAT_1020aee4 != 0)
    {
        HWND wnd = FUN_10001570();
        if (wnd != 0)
            return SendMessageA(wnd, 0x400, 0x225, param);
    }
    return 0;
}

// SoundArc_Init: remembers where soundarc.dll is (`dir`, or the current folder) and starts it when `enable`.
// Returns 1 when its window was found or started.
extern "C" int FUN_10001ee0(int enable, const char *dir)
{
    if (dir != 0)
        sprintf(DAT_1020ade0, "%s\\soundarc.dll", dir);
    else
        sprintf(DAT_1020ade0, ".\\soundarc.dll");
    DAT_1020aee4 = enable;
    return FUN_10001570() != 0;
}

// Shuts the sound process down (WM_CLOSE to its window).
extern "C" int FUN_10001f30(void)
{
    if (DAT_1020aee4 != 0)
    {
        HWND wnd = FUN_10001570();
        SendMessageA(wnd, WM_CLOSE, 0, 0);
        DAT_1020aee4 = 0;
    }
    return 1;
}

