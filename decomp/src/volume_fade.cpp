// Screen fades. A fade has a target amount (DAT_104bf8e8), the current amount (DAT_104bf8f0, 0-100), a per-frame
// step (DAT_1032d4b0) and a frame counter (DAT_104bf8e4); the per-frame update elsewhere adds the step until the
// counter runs out. DAT_104bf8ec is the fade kind: its hundreds pick the colour (0 black, 1 white, 2 red, 3 the
// colour in DAT_104c02e0), the rest an iris wipe (drawn by Fade_Draw, screen_fade.cpp). Script built-ins call these.

extern "C" float DAT_104bf8e8;
extern "C" int DAT_104bf8ec;
extern "C" float DAT_104bf8f0;
extern "C" float DAT_1032d4b0;
extern "C" int DAT_104bf8e4;
extern "C" int DAT_104c02e0;

// Starts a fade of track `id` to `value` over `frames` frames (0 = jump there at once).
// The int-to-float conversion of `value` is rounded once (/Op) and reused for every store.
extern "C" int FUN_1000da40(int id, int value, int frames)
{
    DAT_104bf8e8 = value;
    DAT_104bf8ec = id;
    if (frames == 0)
    {
        DAT_104bf8f0 = value;
        DAT_104bf8e8 = value;
        DAT_1032d4b0 = 0.0f;
        DAT_104bf8e4 = 0;
        return 0;
    }
    DAT_104bf8e4 = frames;
    DAT_1032d4b0 = (DAT_104bf8e8 - DAT_104bf8f0) / frames;
    return 0;
}

// Same fade for the tracks numbered from 300 (id + 300), also storing a fade mode in DAT_104c02e0.
extern "C" int FUN_1000dab0(int id, int value, int frames, int mode)
{
    DAT_104bf8e8 = value;
    DAT_104bf8ec = id + 300;
    DAT_104c02e0 = mode;
    if (frames == 0)
    {
        DAT_104bf8f0 = value;
        DAT_104bf8e8 = value;
        DAT_1032d4b0 = 0.0f;
        DAT_104bf8e4 = 0;
        return 0;
    }
    DAT_104bf8e4 = frames;
    DAT_1032d4b0 = (DAT_104bf8e8 - DAT_104bf8f0) / frames;
    return 0;
}

// Same fade for the tracks numbered from 100 (id + 100).
extern "C" int FUN_1000db30(int id, int value, int frames)
{
    DAT_104bf8e8 = value;
    DAT_104bf8ec = id + 100;
    if (frames == 0)
    {
        DAT_104bf8f0 = value;
        DAT_104bf8e8 = value;
        DAT_1032d4b0 = 0.0f;
        DAT_104bf8e4 = 0;
        return 0;
    }
    DAT_104bf8e4 = frames;
    DAT_1032d4b0 = (DAT_104bf8e8 - DAT_104bf8f0) / frames;
    return 0;
}
