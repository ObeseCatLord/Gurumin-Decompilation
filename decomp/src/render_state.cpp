// Cached Direct3D state setters. Each skips the device call when the cached value already matches, and
// only calls Set... when the device's current value really differs. Hooking these is the easiest way to force
// filtering, anisotropy or blending changes in a rendering mod (docs/MODDING_MAP.md).

#include "d3d9_min.h"

extern "C" IDirect3DDevice9 *DAT_10f1fcf0;
extern "C" unsigned long DAT_10f623d0[0x100];
extern "C" unsigned long DAT_10f08d40[8][0x100];
extern "C" unsigned long DAT_10f141e0[8][0x100];

// Render_SetRenderState: returns 1 when the cached value changed.
extern "C" int FUN_1013ef30(unsigned long state, unsigned long value)
{
    if (DAT_10f623d0[state] == value)
        return 0;
    unsigned long current;
    DAT_10f1fcf0->GetRenderState(state, &current);
    if (current != value)
        DAT_10f1fcf0->SetRenderState(state, value);
    DAT_10f623d0[state] = value;
    return 1;
}

// Render_SetTextureStageState (8 stages x 256 state types cached).
extern "C" int FUN_1013ef90(unsigned long stage, unsigned long type, unsigned long value)
{
    unsigned long *cached = &DAT_10f08d40[stage][type];
    if (*cached == value)
        return 0;
    unsigned long current;
    DAT_10f1fcf0->GetTextureStageState(stage, type, &current);
    if (current != value)
        DAT_10f1fcf0->SetTextureStageState(stage, type, value);
    *cached = value;
    return 1;
}

// Render_SetSamplerState (8 samplers x 256 state types cached).
extern "C" int FUN_1013f000(unsigned long sampler, unsigned long type, unsigned long value)
{
    unsigned long *cached = &DAT_10f141e0[sampler][type];
    if (*cached == value)
        return 0;
    unsigned long current;
    DAT_10f1fcf0->GetSamplerState(sampler, type, &current);
    if (current != value)
        DAT_10f1fcf0->SetSamplerState(sampler, type, value);
    *cached = value;
    return 1;
}
