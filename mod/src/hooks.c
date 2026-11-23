// Small helpers shared by the hook modules: APP.dll addresses and code detours.
#include <windows.h>
#include <string.h>
#include "display.h"

static BYTE *hooks_app_base;
static void (*hooks_log)(const char *format, ...);

void hooks_init(BYTE *app_base, void (*log)(const char *format, ...))
{
    hooks_app_base = app_base;
    hooks_log = log;
}

BYTE *app_address(DWORD va)
{
    return hooks_app_base + (va - 0x10000000u);
}

// `expect` lists the original bytes at `va` (0x100 = any byte, e.g. a relocated address).
int code_matches(DWORD va, const short *expect, int length)
{
    const BYTE *target = app_address(va);
    for (int i = 0; i < length; i++)
        if (expect[i] < 0x100 && target[i] != expect[i])
        {
            hooks_log("unexpected code at %08lx: not hooked", va);
            return 0;
        }
    return 1;
}

// Jumps from the first `length` bytes of `va` (whole instructions, none relative) to `hook`; returns a trampoline
// that runs those bytes and continues in the original function.
void *detour(DWORD va, int length, void *hook)
{
    BYTE *target = app_address(va);
    BYTE *trampoline = VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!trampoline)
        return NULL;
    memcpy(trampoline, target, length);
    trampoline[length] = 0xe9;
    *(INT32 *)(trampoline + length + 1) = (INT32)(target + length - (trampoline + length + 5));
    DWORD old;
    VirtualProtect(target, length, PAGE_EXECUTE_READWRITE, &old);
    target[0] = 0xe9;
    *(INT32 *)(target + 1) = (INT32)((BYTE *)hook - (target + 5));
    for (int i = 5; i < length; i++)
        target[i] = 0x90;
    VirtualProtect(target, length, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, length);
    return trampoline;
}
