// Message window animation, per-character key timers, attached meshes and the debugger check.

#include <windows.h>
#include <stdlib.h>

#include "app_0008.h"
#include "engine_memory.h"
#include "game_vars.h"

#include "char_tables.h"

extern "C" int DAT_1020247c[8];

// Releases and frees the owned mesh node and clears the two counters. Returns 1.
int MeshOwner::M_1000be10()
{
    m_0030 = 0;
    m_0034 = 0;
    if (mesh != 0)
    {
        mesh->M_1019e9a0();
        DAT_11b1ba40(mesh);
    }
    mesh = 0;
    return 1;
}

// Animates the message window: opening (state 1) counts 0 -> 15, closing (state 2) counts back to 0, after
// which the window is hidden (state 0).
extern "C" int FUN_1000cbe0(void)
{
    if (DAT_104bf8d4 == 1)
    {
        if (DAT_104bf8d8 < 15)
            DAT_104bf8d8++;
    }
    else if (DAT_104bf8d4 == 2)
    {
        if (DAT_104bf8d8 > 0)
            DAT_104bf8d8--;
    }
    if (DAT_104bf8d8 == 0)
        DAT_104bf8d4 = 0;
    return 0;
}

// Shows key prompt `key` over character `index` for `frames` frames (only refreshes the timer when the same
// prompt is already shown).
extern "C" int FUN_1000d620(int index, int key, int frames)
{
    if (DAT_104bf8f8[index] != 0 && DAT_1021d138[index] == key)
    {
        DAT_1021f740[index] = frames;
        return 0;
    }
    DAT_1021f740[index] = frames;
    DAT_104bf8f8[index] = 1;
    DAT_1021d138[index] = key;
    return 0;
}

// Counts down the key prompt timers of all characters and hides expired prompts.
extern "C" int FUN_1000d670(void)
{
    for (int i = 0; i < 0x7c; i++)
    {
        if (DAT_104bf8f8[i] != 0)
        {
            if (--DAT_1021f740[i] < 1)
                DAT_104bf8f8[i] = 0;
        }
    }
    return 0;
}

// Releases and frees the meshes of all 64 attached mesh slots. Returns 1.
extern "C" int FUN_1000d9d0(void)
{
    for (int i = 0; i < 0x40; i++)
    {
        DAT_10edbe0c[i].m_002c = 0;
        DAT_10edbe0c[i].m_0030 = 0;
        if (DAT_10edbe0c[i].mesh != 0)
        {
            DAT_10edbe0c[i].mesh->M_1019e9a0();
            DAT_11b1ba40(DAT_10edbe0c[i].mesh);
        }
        DAT_10edbe0c[i].mesh = 0;
    }
    return 1;
}

// Pushes `value` into the 8-entry history ring DAT_1020247c.
extern "C" int FUN_1000da20(int value)
{
    DAT_1020247c[DAT_104bfe88] = value;
    DAT_104bfe88 = (DAT_104bfe88 + 1) & 7;
    return 0;
}

typedef BOOL (WINAPI *IsDebuggerPresentProc)(void);

// Anti-debugger check: when IsDebuggerPresent reports a debugger, drops random noise into the anti-tamper
// table (see anti_tamper.cpp).
extern "C" int FUN_1000eb90(void)
{
    HMODULE kernel = LoadLibraryA("KERNEL32.DLL");
    if (kernel != 0)
    {
        IsDebuggerPresentProc isDebuggerPresent = (IsDebuggerPresentProc)GetProcAddress(kernel, "IsDebuggerPresent");
        if (isDebuggerPresent != 0 && isDebuggerPresent())
        {
            int value = rand();
            DAT_104c0270[rand() % 16] = value;
        }
        FreeLibrary(kernel);
    }
    return 0;
}
