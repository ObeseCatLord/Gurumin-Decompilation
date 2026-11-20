// The frame pacer (0x1014ade0, called once per drawn frame by the game loop, the loader and the screenshot code):
// waits for the 60 Hz deadline and runs the fixed-rate engine ticks that fell due.

#include <windows.h>
#include <mmsystem.h>

extern "C" int DAT_10f654f0;      // 1 = wait with Sleep and run one step per frame (smooth mode)
extern "C" int DAT_10202b08;      // debug keys enabled
extern "C" int DAT_10f654d8;      // next deadline (timeGetTime milliseconds)
extern "C" unsigned int DAT_10f65508;  // 60 Hz steps so far; every second one is an engine tick
extern "C" int DAT_10f65504;      // engine ticks so far
extern "C" int DAT_10f654c0;      // counts down by one per tick
// Frame blend (after-image) strength (engine_1013e.cpp).
extern "C" float DAT_10f6549c;    // current strength
extern "C" float DAT_10f1fcf8;    // step per tick
extern "C" float DAT_10f628d0;    // limit
extern "C" int DAT_10f65498;      // 1 while the frame blend is active
extern "C" int DAT_10f654dc;      // frames since the rate was last measured
extern "C" DWORD DAT_10f654e0;    // time of the last measurement
extern "C" float DAT_10f654e8;    // measured frames per second
extern "C" void *DAT_10f6550c;    // texture bound to stage 0 (texture_bind.cpp)
extern "C" void FUN_1013f9a0(void);   // ColourFades_Step
extern "C" void FUN_10147a00(void);   // TextureSlots_Age
extern "C" void FUN_101483f0(void);   // TexCache_Age

// One engine tick: steps the colour fades, counts the tick, counts down DAT_10f654c0 and moves the frame blend
// strength by its step, capped at the limit; at 0 or below the blend ends.
static __forceinline void Engine_Tick()
{
    FUN_1013f9a0();
    DAT_10f65504++;
    if (DAT_10f654c0 != 0)
        DAT_10f654c0--;
    if (DAT_10f65498 != 0)
    {
        DAT_10f6549c += DAT_10f1fcf8;
        if (DAT_10f628d0 < DAT_10f6549c)
            DAT_10f6549c = DAT_10f628d0;
        if (DAT_10f6549c <= 0.0f)
        {
            DAT_10f6549c = 0;
            DAT_10f65498 = 0;
        }
    }
}

// 0x1014ade0 (Frame_Pace). Steps are 16 or 17 ms (17 on two of every three, about 60 per second) and every second
// step runs an engine tick. In smooth mode it sleeps until the deadline and takes one step from now. Otherwise it
// takes steps until the deadline is ahead of the clock (catching up after slow frames); with the debug keys, right
// Shift slows this to a step a second from now and right Ctrl runs one tick per frame without waiting. Every 30
// frames the frame rate is measured into DAT_10f654e8. Then the stage 0 binding is forgotten and the texture caches
// age. Returns 1.
extern "C" int FUN_1014ade0(void)
{
    if (DAT_10f654f0 != 0)
    {
        while (DAT_10f654d8 > (int)timeGetTime())
            Sleep(1);
        int now = timeGetTime();
        DAT_10f654d8 = now + 16;
        if (DAT_10f65508 % 3 != 0)
            DAT_10f654d8++;
        DAT_10f65508++;
        if ((DAT_10f65508 & 1) == 0)
            Engine_Tick();
    }
    else if (DAT_10202b08 != 0 && (GetKeyState(VK_RSHIFT) & 0xf000) != 0)
    {
        while (DAT_10f654d8 <= (int)timeGetTime())
        {
            int now = timeGetTime();
            DAT_10f654d8 = now + 1000;
            if (DAT_10f65508 % 3 != 0)
                DAT_10f654d8++;
            DAT_10f65508++;
            if ((DAT_10f65508 & 1) == 0)
                Engine_Tick();
        }
    }
    else if (DAT_10202b08 != 0 && (GetKeyState(VK_RCONTROL) & 0xf000) != 0)
    {
        Engine_Tick();
        DAT_10f654d8 = timeGetTime() + 16;
    }
    else
    {
        while (DAT_10f654d8 <= (int)timeGetTime())
        {
            DAT_10f654d8 += 16;
            if (DAT_10f65508 % 3 != 0)
                DAT_10f654d8++;
            DAT_10f65508++;
            if ((DAT_10f65508 & 1) == 0)
                Engine_Tick();
        }
    }
    if (++DAT_10f654dc > 29)
    {
        DAT_10f654dc = 0;
        DAT_10f654e8 = 1000.0f / ((float)(timeGetTime() - DAT_10f654e0) / 30.0f);
        DAT_10f654e0 = timeGetTime();
    }
    DAT_10f6550c = 0;
    FUN_10147a00();
    FUN_101483f0();
    return 1;
}
