// Cached sampler state setting (EN build; JP: Render_SetSamplerState in decomp/src/render_state.cpp).

#include "render.h"

// Sets a sampler state unless the cache already holds that value; the device is only written when it differs.
// Returns 0 when nothing was written, else the result of the device call. The EN code reads the device's current value
// into the first parameter's slot after the cache index is computed. EN 0x7a4ef0 (JP 0x1013f000).
extern "C" int Render_SetSamplerState(unsigned long sampler, unsigned long type, unsigned long value)
{
    unsigned long samplerIndex = sampler;
    unsigned long cacheIndex = sampler * TEXTURE_STAGE_STATE_COUNT + type;
    unsigned long *cached = &g_samplerStateCache[0][cacheIndex];
    if (*cached == value)
        return 0;
    g_d3dDevice->GetSamplerState(samplerIndex, (D3DSAMPLERSTATETYPE)type, &sampler);
    if (sampler != value)
    {
        int result = g_d3dDevice->SetSamplerState(samplerIndex, (D3DSAMPLERSTATETYPE)type, value);
        *cached = value;
        return result;
    }
    *cached = value;
    return 0;
}
