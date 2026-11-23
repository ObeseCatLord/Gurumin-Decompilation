// Colour and fog parameters that change over time.

#include "vec4.h"

#include "camera_vars.h"
#include "render_vars.h"

// Advances the two colour blends (light/ambient colours) by one frame each.
extern "C" void FUN_1013f9a0(void)
{
    for (int i = 0; i < 2; i++)
    {
        if (DAT_10f08d30[i] != 0)
        {
            DAT_10f08d30[i]--;
            DAT_10f66978[i].Copy(DAT_10f66978[i] + DAT_10f657c4[i]);
            if (DAT_10f08d30[i] == 0)
                DAT_10f66978[i].Copy(DAT_10f65828[i]);
        }
    }
}


// 0x1013fa10. Starts colour blend `index` towards (x, y, z, w) over `frames` frames; with 1 frame or fewer the
// colour is set at once. Note the argument order: z, y, x, w. Callers: the device setup and the scene code.
extern "C" void FUN_1013fa10(int index, int frames, float z, float y, float x, float w)
{
    if (x >= 530.0f)
        DAT_10203424++;
    if (frames <= 1)
    {
        DAT_10f66978[index].Set(x, y, z, w);
        DAT_10f08d30[index] = 0;
        return;
    }
    DAT_10f65828[index].Set(x, y, z, w);
    float step = 1.0f / (float)frames;
    DAT_10f657c4[index].Copy((DAT_10f65828[index] - DAT_10f66978[index]) * step);
    DAT_10f08d30[index] = frames;
}


// Turns the colour effect on with `mode` and colour (r, g, b), or off when mode is negative. Returns 1.
extern "C" int FUN_1016afa0(int mode, int r, int g, int b)
{
    if (mode < 0)
    {
        DAT_10ff1df0 = 0;
        return 1;
    }
    DAT_11018730 = mode;
    DAT_11018624 = 0;
    DAT_10ff1df0 = 1;
    DAT_11af3f48.Set((float)r, (float)g, (float)b, 0);
    return 1;
}
