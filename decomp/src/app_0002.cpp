// More soundarc.dll commands (see soundarc.cpp). Arguments go in with codes 0..9, then the command code;
// strings travel with WM_COPYDATA.

#include <windows.h>

#include "sound_vars.h"

extern "C" void __stdcall FUN_10001170(int, int, int, int, int, int);
extern "C" HWND FUN_10001570(void);

// Stops all sound (FUN_10001170 with no arguments). Returns 1.
extern "C" int FUN_10001550(void)
{
    FUN_10001170(0, 0, 0, 0, 0, 0);
    return 1;
}

// Command 0x221 (no arguments).
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

// Command 0x227: sets the master music attenuation (in 1/100 dB, larger = quieter).
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


// Plays file `path` on music stream `stream` with four playback parameters (command 0x207).
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
// Command 0x201: stops music stream `stream`.
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

// Command 0x212: fades music stream `stream` out over `fadeTime` ms.
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
// Command 0x235: plays 3D sound effect `id` with nine parameters (position, volume, pitch, ...).
extern "C" LRESULT FUN_100017c0(int id, int a, int b, int c, int d, int e, int f, int g, int h, int i)
{
    if (DAT_1020aee4 == 0)
        return 0;
    HWND wnd = FUN_10001570();
    if (wnd != 0)
    {
        SendMessageA(wnd, 0x400, 0, a);
        SendMessageA(wnd, 0x400, 1, b);
        SendMessageA(wnd, 0x400, 2, c);
        SendMessageA(wnd, 0x400, 3, d);
        SendMessageA(wnd, 0x400, 4, e);
        SendMessageA(wnd, 0x400, 5, f);
        SendMessageA(wnd, 0x400, 6, g);
        SendMessageA(wnd, 0x400, 7, h);
        SendMessageA(wnd, 0x400, 8, i);
        SendMessageA(wnd, 0x400, 9, id);
        return SendMessageA(wnd, 0x400, 0x235, 0);
    }
    return 0;
}

// Command 0x267: moves 3D sound `stream` to position (x, y, z) with velocity (dx, dy, dz); null pointers
// send zeros.
extern "C" LRESULT FUN_10001890(int stream, const int *x, const int *y, const int *z, const int *dx, const int *dy, const int *dz)
{
    if (DAT_1020aee4 == 0)
        return 0;
    HWND wnd = FUN_10001570();
    if (wnd != 0)
    {
        SendMessageA(wnd, 0x400, 0, stream);
        if (x != 0)
        {
            SendMessageA(wnd, 0x400, 1, *x);
            SendMessageA(wnd, 0x400, 2, *y);
            SendMessageA(wnd, 0x400, 3, *z);
        }
        else
        {
            SendMessageA(wnd, 0x400, 1, 0);
            SendMessageA(wnd, 0x400, 2, 0);
            SendMessageA(wnd, 0x400, 3, 0);
        }
        if (dx != 0)
        {
            SendMessageA(wnd, 0x400, 4, *dx);
            SendMessageA(wnd, 0x400, 5, *dy);
            SendMessageA(wnd, 0x400, 6, *dz);
        }
        else
        {
            SendMessageA(wnd, 0x400, 4, 0);
            SendMessageA(wnd, 0x400, 5, 0);
            SendMessageA(wnd, 0x400, 6, 0);
        }
        return SendMessageA(wnd, 0x400, 0x267, 0);
    }
    return 0;
}
