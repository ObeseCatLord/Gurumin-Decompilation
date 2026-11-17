// Engine frame helpers: Direct3D scene brackets, the 60 fps frame budget, data folder names.

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>
#include "d3d9_min.h"

extern "C" IDirect3DDevice9 *DAT_10f1fcf0;
extern "C" int DAT_10f6547c;
extern "C" DWORD DAT_10f654d8;
extern "C" int DAT_10f65508;
extern "C" int DAT_10f65504;
// Frame blend (after-image) strength: rises by the step every other frame up to the limit (see FUN_1013fb10).
extern "C" float DAT_10f6549c;  // current strength
extern "C" float DAT_10f1fcf8;  // step per update
extern "C" float DAT_10f628d0;  // limit
extern "C" int DAT_10f65498;    // 1 while the frame blend is active
extern "C" int DAT_10f654f4;    // frame blending allowed by the settings
extern "C" int DAT_10202b0c;    // number of frame buffers
extern "C" int DAT_10f6553c;
extern "C" char DAT_10202b40[][0x100];
extern "C" int DAT_10f0edf8;
extern "C" char DAT_10f64368[4][0x100];
extern "C" char DAT_10f627d0[];

// Starts a new Direct3D scene (ending the open one first).
extern "C" int FUN_1013ed20(void)
{
    if (DAT_10f6547c != 0)
        DAT_10f1fcf0->EndScene();
    DAT_10f1fcf0->BeginScene();
    DAT_10f6547c = 1;
    return 0;
}

// Ends the open Direct3D scene.
extern "C" int FUN_1013ed60(void)
{
    if (DAT_10f6547c != 0)
        DAT_10f1fcf0->EndScene();
    DAT_10f6547c = 0;
    return 0;
}

// Sets the next-frame deadline DAT_10f654d8 to now (no wait).
extern "C" int FUN_1013eeb0(void)
{
    DAT_10f654d8 = timeGetTime();
    return 1;
}

// FrameTimer_ResetNext: next-frame deadline = now + 16 ms (one 60 Hz frame).
extern "C" int FUN_1013eee0(void)
{
    DAT_10f654d8 = timeGetTime() + 16;
    return 1;
}

// Clears six per-frame counters.
extern "C" int FUN_1013ef00(void)
{
    DAT_10f65508 = 0;
    DAT_10f65504 = 0;
    DAT_10f6549c = 0;
    DAT_10f1fcf8 = 0;
    DAT_10f628d0 = 0;
    DAT_10f6553c = 0;
    return 1;
}

// Sets the name of data folder `index` (used by FUN_1013f8a0 to build paths).
extern "C" void FUN_1013f8f0(const char *text, int index)
{
    strcpy(DAT_10202b40[index], text);
}

// Returns "<music folder>\\<name>" in one of four rotating buffers.
extern "C" char *FUN_1013f910(const char *name)
{
    DAT_10f0edf8++;
    sprintf(DAT_10f64368[DAT_10f0edf8 & 3], "%s\\%s", DAT_10f627d0, name);
    return DAT_10f64368[DAT_10f0edf8 & 3];
}

// Sets the music folder (a trailing backslash is removed).
extern "C" void FUN_1013f950(const char *dir)
{
    strcpy(DAT_10f627d0, dir);
    if (DAT_10f627d0[strlen(DAT_10f627d0) - 1] == '\\')
        DAT_10f627d0[strlen(DAT_10f627d0) - 1] = 0;
}

// Same as FrameTimer_ResetNext but returns 0.
extern "C" int FUN_1013faf0(void)
{
    DAT_10f654d8 = timeGetTime() + 16;
    return 0;
}

extern "C" int DAT_10f6552c;  // set when Windows asked the game to quit (WM_QUIT)

// Pumps the Windows message queue: handles at most 30 pending messages per call so a flood of input cannot stall
// the frame. Returns 0 (and sets the quit flag) when WM_QUIT arrives, else 1.
extern "C" int FUN_1013ec90(void)
{
    MSG msg;
    int count = 0;
    while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (count >= 30)
            break;
        count++;
        if (msg.message == WM_QUIT)
        {
            DAT_10f6552c = 1;
            return 0;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 1;
}

// Starts the frame blend (after-image effect): `active` turns it on, and the strength moves towards `target`
// over `frames` updates (at least 1). The limit is the higher of the current strength and the target. Without
// blending support (setting off or a single frame buffer) everything stays at 0. Also restarts the frame timer.
// Returns 0.
extern "C" int FUN_1013fb10(int active, int target, int frames)
{
    DAT_10f65498 = active;
    float goal = (float)target;
    DAT_10f628d0 = DAT_10f6549c < goal ? goal : DAT_10f6549c;
    if (frames < 1)
        frames = 1;
    DAT_10f1fcf8 = (goal - DAT_10f6549c) / (float)frames;
    DAT_10f654d8 = timeGetTime() + 16;
    if (DAT_10f654f4 == 0 || DAT_10202b0c == 1)
    {
        DAT_10f6549c = 0;
        DAT_10f628d0 = 0;
        DAT_10f1fcf8 = 0;
    }
    return 0;
}
