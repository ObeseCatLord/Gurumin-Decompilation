// Frame pacer reset (EN build, EN-only: no JP twin found).
#include "frame_pacer.h"
#include "engine.h"

// FramePacer_Reset: restarts the frame counting: no 60 Hz steps or engine ticks yet, the frame blend strength, step and
// limit at zero and no scene callback. Returns 1. Called by EN 0x5a2dd0.
extern "C" int FramePacer_Reset(void)
{
    g_frameStepCount = 0;
    g_engineTickCount = 0;
    g_frameBlendStrength = 0;
    g_frameBlendStep = 0;
    g_frameBlendLimit = 0;
    g_frameCallback = 0;
    return 1;
}
