// Screen transitions (fades between scenes) of the EN build. Ported from the matched JP source
// decomp/src/screen_transition.cpp.
#include <windows.h>
#include <mmsystem.h>

#include "engine.h"
#include "render.h"
#include "screen_transition.h"

// Milliseconds to the next frame deadline after a transition started or ended.
const int TRANSITION_FRAME_MS = 16;

// Starts (mode != 0) or ends (mode 0) a transition: swaps the double-buffered capture target, records the mode, resets
// the frame timer to now + 16 ms and redraws the frame once. Ignored while disabled or busy. Called when a scene starts
// and ends.
extern "C" int Screen_StartTransition(int mode)
{
    if (g_transitionsEnabled == 0)
        return 0;
    if (g_frameBlendActive == 0)
    {
        DWORD now;
        if (mode != 0)
        {
            if (g_transitionMode != 0)
                return 0;
            g_swapTargetSaved = g_swapTargetInUse;
            g_swapTargetInUse = (g_swapTargetInUse - 1) & 1;
            now = timeGetTime();
            g_transitionMode = mode;
            g_transitionModeShown = mode;
            g_transitionCapturing = 1;
        }
        else
        {
            g_swapTargetInUse = g_swapTargetSaved;
            now = timeGetTime();
            g_transitionMode = 0;
            g_transitionModeShown = 0;
            g_transitionCapturing = 0;
        }
        g_nextFrameDeadline = now + TRANSITION_FRAME_MS;
        Render_CloseScene();
        if (g_renderToBackBuffer != 0)
        {
            Render_ResetTarget();
            Render_BeginScene();
            return 0;
        }
        Render_ResetSwapTarget();
        Render_BeginScene();
    }
    return 0;
}
