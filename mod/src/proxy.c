// d3d9.dll proxy: forwards every export to the system's d3d9.dll and lets display.c adjust the game
// when it creates Direct3D (Render_CreateDevice 0x10148b00 calls Direct3DCreate9 before it sizes the
// window and fills the present parameters).
#include <windows.h>
#include "display.h"

static HMODULE system_d3d9;

static FARPROC system_export(const char *name)
{
    if (!system_d3d9)
    {
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);   // SysWOW64 for this 32-bit process
        lstrcatA(path, "\\d3d9.dll");
        system_d3d9 = LoadLibraryA(path);
    }
    return system_d3d9 ? GetProcAddress(system_d3d9, name) : NULL;
}

void *WINAPI proxy_Direct3DCreate9(UINT sdk_version)
{
    typedef void *(WINAPI *create_fn)(UINT);
    create_fn create = (create_fn)system_export("Direct3DCreate9");
    if (!create)
        return NULL;
    display_before_direct3d();
    void *d3d = create(sdk_version);
    if (d3d)
        display_hook_direct3d(d3d);
    return d3d;
}

// The other exports pass straight through (other programs or tools may load this d3d9.dll too).
HRESULT WINAPI proxy_Direct3DCreate9Ex(UINT sdk_version, void **out)
{
    typedef HRESULT(WINAPI *fn)(UINT, void **);
    fn f = (fn)system_export("Direct3DCreate9Ex");
    return f ? f(sdk_version, out) : E_NOTIMPL;
}

int WINAPI proxy_D3DPERF_BeginEvent(DWORD colour, const wchar_t *name)
{
    typedef int(WINAPI *fn)(DWORD, const wchar_t *);
    fn f = (fn)system_export("D3DPERF_BeginEvent");
    return f ? f(colour, name) : 0;
}

int WINAPI proxy_D3DPERF_EndEvent(void)
{
    typedef int(WINAPI *fn)(void);
    fn f = (fn)system_export("D3DPERF_EndEvent");
    return f ? f() : 0;
}

void WINAPI proxy_D3DPERF_SetMarker(DWORD colour, const wchar_t *name)
{
    typedef void(WINAPI *fn)(DWORD, const wchar_t *);
    fn f = (fn)system_export("D3DPERF_SetMarker");
    if (f)
        f(colour, name);
}

void WINAPI proxy_D3DPERF_SetRegion(DWORD colour, const wchar_t *name)
{
    typedef void(WINAPI *fn)(DWORD, const wchar_t *);
    fn f = (fn)system_export("D3DPERF_SetRegion");
    if (f)
        f(colour, name);
}

BOOL WINAPI proxy_D3DPERF_QueryRepeatFrame(void)
{
    typedef BOOL(WINAPI *fn)(void);
    fn f = (fn)system_export("D3DPERF_QueryRepeatFrame");
    return f ? f() : FALSE;
}

void WINAPI proxy_D3DPERF_SetOptions(DWORD options)
{
    typedef void(WINAPI *fn)(DWORD);
    fn f = (fn)system_export("D3DPERF_SetOptions");
    if (f)
        f(options);
}

DWORD WINAPI proxy_D3DPERF_GetStatus(void)
{
    typedef DWORD(WINAPI *fn)(void);
    fn f = (fn)system_export("D3DPERF_GetStatus");
    return f ? f() : 0;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(instance);
        display_set_module(instance);
    }
    else if (reason == DLL_PROCESS_DETACH)
        display_shutdown();
    return TRUE;
}
