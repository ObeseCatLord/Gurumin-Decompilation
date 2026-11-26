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

void patch_code(void *address, const void *bytes, size_t size)
{
    DWORD old;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old);
    memcpy(address, bytes, size);
    VirtualProtect(address, size, old, &old);
    FlushInstructionCache(GetCurrentProcess(), address, size);
}

// Finds the `call rel32` instructions (E8) in the function at `function` (`size` bytes) whose target is `callee`;
// stores up to `max` of their addresses (as APP.dll addresses) in `sites` and returns how many there are. An E8
// counts only when its relative target lands exactly on `callee`, so a stray E8 byte inside another instruction is
// not taken for a call unless it also happens to point there.
int find_calls(DWORD function, DWORD size, DWORD callee, DWORD *sites, int max)
{
    BYTE *start = app_address(function);
    BYTE *target = app_address(callee);
    int found = 0;
    for (BYTE *p = start; p + 5 <= start + size; p++)
    {
        if (*p != 0xe8 || p + 5 + *(INT32 *)(p + 1) != target)
            continue;
        if (found < max)
            sites[found] = function + (DWORD)(p - start);
        found++;
        p += 4;
    }
    return found;
}

// Points the calls at `sites` (from find_calls) at `hook`. The original target stays reachable at its own address.
void redirect_calls(const DWORD *sites, int count, void *hook)
{
    for (int i = 0; i < count; i++)
    {
        BYTE *call = app_address(sites[i]);
        INT32 relative = (INT32)((BYTE *)hook - (call + 5));
        patch_code(call + 1, &relative, 4);
    }
}

// Replaces entry `index` of a COM vtable with `hook`; returns the original, or NULL when it is already hooked.
void *vtable_hook(void **vtable, int index, void *hook)
{
    void *original = vtable[index];
    if (original == hook)
        return NULL;
    DWORD old;
    VirtualProtect(&vtable[index], sizeof(void *), PAGE_READWRITE, &old);
    vtable[index] = hook;
    VirtualProtect(&vtable[index], sizeof(void *), old, &old);
    return original;
}
