// EN colour blends (JP twin: colour_fades.cpp).

#include "colour_fades.h"

// Advances the two colour blends (light/ambient colours) by one frame each. Called every frame.
extern "C" void ColourBlends_Update(void)
{
    for (int i = 0; i < COLOUR_BLEND_COUNT; i++)
    {
        if (g_colourBlendFrames[i] != 0)
        {
            g_colourBlendFrames[i]--;
            g_colourBlendCurrent[i].Copy(g_colourBlendCurrent[i] + g_colourBlendStep[i]);
            if (g_colourBlendFrames[i] == 0)
                g_colourBlendCurrent[i].Copy(g_colourBlendTarget[i]);
        }
    }
}

// Colour blend argument value from which a start is counted in g_colourBlendBigCount.
const float COLOUR_BLEND_BIG = 530.0f;

// Starts colour blend `index` towards (x, y, z, w) over `frames` frames; with 1 frame or fewer the colour is set at
// once. Callers: the device setup and the scene code.
extern "C" void ColourBlend_Start(int index, int frames, float z, float y, float x, float w)
{
    if (x >= COLOUR_BLEND_BIG)
        g_colourBlendBigCount++;
    if (frames <= 1)
    {
        g_colourBlendCurrent[index].Set(x, y, z, w);
        g_colourBlendFrames[index] = 0;
        return;
    }
    g_colourBlendTarget[index].Set(x, y, z, w);
    float step = 1.0f / (float)frames;
    g_colourBlendStep[index].Copy((g_colourBlendTarget[index] - g_colourBlendCurrent[index]) * step);
    g_colourBlendFrames[index] = frames;
}

// Turns the scene colour effect on with `mode` and colour (r, g, b), or off when mode is negative. Returns 1.
extern "C" int ColourEffect_Set(int mode, int r, int g, int b)
{
    if (mode < 0)
    {
        g_colourEffectActive = 0;
        return 1;
    }
    g_colourEffectActive = 1;
    g_colourEffectPhase = 0;
    g_colourEffectMode = mode;
    g_colourEffectColour.Set((float)r, (float)g, (float)b, 0);
    return 1;
}
