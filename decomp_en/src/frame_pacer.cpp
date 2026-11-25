// The frame pacer (EN build): called once per drawn frame by the game loop, the loader and the screenshot code; waits for
// the 60 Hz deadline and runs the fixed-rate engine ticks that fell due. Ported from the matched JP source
// decomp/src/frame_pacer.cpp.
#include <windows.h>
#include <mmsystem.h>
#include "colour_fades.h"
#include "engine.h"
#include "frame_pacer.h"
#include "render.h"

// Milliseconds a step lasts (a third of the steps last one more), and a step's length under the slow-motion key.
const int FRAME_STEP_MS = 16;
const int SLOW_STEP_MS = 1000;
// Every this many steps the step is one millisecond longer except on the first (about 60 steps per second).
const int LONG_STEP_PERIOD = 3;
// Frames between two measurements of the frame rate, and the milliseconds per second.
const int FRAME_RATE_WINDOW = 30;
const float MS_PER_SECOND = 1000.0f;
// Virtual keys of the debug pacing: right Shift slows to a step a second, right Ctrl runs one tick per frame.
const int DEBUG_SLOW_KEY = VK_RSHIFT;
const int DEBUG_STEP_KEY = VK_RCONTROL;
// Bits of GetKeyState that say the key is down.
const int KEY_DOWN_BITS = 0xf000;

// One engine tick: steps the colour fades, counts the tick, counts down the transition and moves the frame blend strength
// by its step, capped at the limit; at 0 or below the blend ends.
static __forceinline void Engine_Tick()
{
    ColourBlends_Update();
    g_engineTickCount++;
    if (g_transitionMode != 0)
        g_transitionMode--;
    if (g_frameBlendActive != 0)
    {
        g_frameBlendStrength += g_frameBlendStep;
        if (g_frameBlendLimit < g_frameBlendStrength)
            g_frameBlendStrength = g_frameBlendLimit;
        if (g_frameBlendStrength <= 0.0f)
        {
            g_frameBlendStrength = 0;
            g_frameBlendActive = 0;
        }
    }
}

// Frame_Pace: steps are 16 or 17 ms (17 on two of every three, about 60 per second) and every second step runs an engine
// tick. In smooth mode it sleeps until the deadline and takes one step from now. Otherwise it takes steps until the
// deadline is ahead of the clock (catching up after slow frames); with the debug keys, right Shift slows this to a step a
// second from now and right Ctrl runs one tick per frame without waiting. Every 30 frames the frame rate is measured.
// Then the stage 0 binding is forgotten and the texture caches age. Returns 1.
extern "C" int Frame_Pace(void)
{
    if (g_smoothMode != 0)
    {
        while ((int)g_nextFrameDeadline > (int)timeGetTime())
            Sleep(1);
        int now = timeGetTime();
        g_nextFrameDeadline = now + FRAME_STEP_MS;
        if (g_frameStepCount % LONG_STEP_PERIOD != 0)
            g_nextFrameDeadline++;
        g_frameStepCount++;
        if ((g_frameStepCount & 1) == 0)
            Engine_Tick();
    }
    else if (g_debugKeysEnabled != 0 && (GetKeyState(DEBUG_SLOW_KEY) & KEY_DOWN_BITS) != 0)
    {
        while ((int)g_nextFrameDeadline <= (int)timeGetTime())
        {
            int now = timeGetTime();
            g_nextFrameDeadline = now + SLOW_STEP_MS;
            if (g_frameStepCount % LONG_STEP_PERIOD != 0)
                g_nextFrameDeadline++;
            g_frameStepCount++;
            if ((g_frameStepCount & 1) == 0)
                Engine_Tick();
        }
    }
    else if (g_debugKeysEnabled != 0 && (GetKeyState(DEBUG_STEP_KEY) & KEY_DOWN_BITS) != 0)
    {
        Engine_Tick();
        g_nextFrameDeadline = timeGetTime() + FRAME_STEP_MS;
    }
    else
    {
        while ((int)g_nextFrameDeadline <= (int)timeGetTime())
        {
            g_nextFrameDeadline += FRAME_STEP_MS;
            if (g_frameStepCount % LONG_STEP_PERIOD != 0)
                g_nextFrameDeadline++;
            g_frameStepCount++;
            if ((g_frameStepCount & 1) == 0)
                Engine_Tick();
        }
    }
    if (++g_framesSinceMeasure > FRAME_RATE_WINDOW - 1)
    {
        g_framesSinceMeasure = 0;
        g_measuredFrameRate = MS_PER_SECOND / ((float)(timeGetTime() - g_measureStartTime) / (float)FRAME_RATE_WINDOW);
        g_measureStartTime = timeGetTime();
    }
    g_stage0Texture = 0;
    TextureSlots_Age();
    TextureCache_Age();
    return 1;
}
