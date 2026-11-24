// Shaga pieces (EN build). Ported from the matched JP source decomp/src/shaga_spawn.cpp.
#include "shaga.h"
#include "node.h"
#include "texture_mgr.h"

// Motion name the spawned piece's colour plugs are restarted with.
static const char SHAGA_STOP_MOTION[] = "stop";

// Shaga_Spawn: creates a shaga piece with Shaga_Create, clears the slot's two targets (-1) and its counter, and
// restarts the model's colour plugs with "stop". Returns 0. Called by the equipment-break scripts.
extern "C" int Shaga_Spawn(const Vec4 *position, const char *model, int radius, float d, float e)
{
    int slot = Shaga_Create(position, model, radius, d, e);
    g_textureManager.ResetFrameTimer();
    if (slot > -1)
    {
        g_shagaTargetA[slot] = SHAGA_NO_TARGET;
        g_shagaTargetB[slot] = SHAGA_NO_TARGET;
        g_shagaCounter[slot] = 0;
        g_shagaNode[slot]->RestartColourPlugs(SHAGA_STOP_MOTION, g_colourPlugFrames);
    }
    g_textureManager.ResetFrameTimer();
    return 0;
}
