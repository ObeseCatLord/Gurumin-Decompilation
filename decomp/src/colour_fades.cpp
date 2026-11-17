// Colour and fog parameters that change over time.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Set(float x, float y, float z, float w);  // 0x101ac9c0
    Vec4 operator+(const Vec4 &other);              // 0x101ac9e0 (component-wise sum)
    void Copy(const Vec4 &source);                  // 0x101acc20
};

extern "C" int DAT_10f08d30[2];   // frames left for each of the two colour blends (0 = idle)
extern "C" Vec4 DAT_10f66978[2];  // current colours
extern "C" Vec4 DAT_10f657c4[2];  // per-frame colour steps
extern "C" Vec4 DAT_10f65828[2];  // target colours (set exactly on the last frame to avoid rounding drift)

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

extern "C" int DAT_10ff1df0;   // 1 = the effect is on
extern "C" int DAT_11018730;   // effect mode
extern "C" int DAT_11018624;   // effect timer
extern "C" Vec4 DAT_11af3f48;  // effect colour (0..255 per channel, stored as floats)

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
