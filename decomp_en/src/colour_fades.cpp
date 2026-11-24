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
