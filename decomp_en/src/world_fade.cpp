// Screen fade of the scene renderer (EN build).
#include "world_renderer.h"

// WorldRenderer::StartFade: starts a fade in mode `mode` towards `target` percent over `frames` frames; without frames
// the fade jumps to the target. Returns 0. Called by the scene script (fade commands) and when a scene is bound.
int WorldRenderer::StartFade(int mode, int target, int frames)
{
    float value = (float)target;
    fadeMode = mode;
    fadeTarget = value;
    if (frames == 0)
    {
        fadeStep = 0;
        fadeAmount = value;
        fadeTarget = value;
        fadeFrames = 0;
        return 0;
    }
    fadeFrames = frames;
    fadeStep = (value - fadeAmount) / (float)frames;
    return 0;
}
