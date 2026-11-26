// Music volume fades and a rectangle test (EN build). JP twin: volume_fade.cpp.
#include "render.h"
#include "volume_fade.h"

// Starts a fade of the tracks numbered from 100 (id + 100) to `value` over `frames` frames; with 0 frames the volume
// changes at once. Called through the script command table. Returns 0.
extern "C" int Bgm_FadeGroup100(int id, int value, int frames)
{
    g_bgmFadeTarget = (float)value;
    g_bgmFadeTrack = id + BGM_GROUP_100;
    if (frames == 0)
    {
        g_bgmFadeStep = 0.0f;
        g_bgmFadeVolume = (float)value;
        g_bgmFadeTarget = (float)value;
        g_bgmFadeFramesLeft = 0;
        return 0;
    }
    g_bgmFadeFramesLeft = frames;
    g_bgmFadeStep = (g_bgmFadeTarget - g_bgmFadeVolume) / frames;
    return 0;
}

// Returns 1 when the point (x, y) lies inside the rectangle (left, top)-(right, bottom), edges included, else 0.
extern "C" int __stdcall Point_InRect(int x, int y, int left, int top, int right, int bottom)
{
    if (x >= left && y >= top && x <= right && y <= bottom)
        return 1;
    return 0;
}

// Starts a fade of the tracks of group 0 (the id itself) to `value` over `frames` frames; with 0 frames the volume
// changes at once. Called through the script command table. Returns 0.
extern "C" int Bgm_FadeGroup0(int id, int value, int frames)
{
    g_bgmFadeTrack = id;
    g_bgmFadeTarget = (float)value;
    if (frames == 0)
    {
        g_bgmFadeStep = 0.0f;
        g_bgmFadeVolume = (float)value;
        g_bgmFadeTarget = (float)value;
        g_bgmFadeFramesLeft = 0;
        return 0;
    }
    g_bgmFadeFramesLeft = frames;
    g_bgmFadeStep = (g_bgmFadeTarget - g_bgmFadeVolume) / frames;
    return 0;
}

// Starts a fade of the tracks numbered from 300 (id + 300) to `value` over `frames` frames (0 = at once) and stores
// `colour` as the colour of screen fade kind 3. Called through the script command table. Returns 0.
extern "C" int Bgm_FadeGroup300(int id, int value, int frames, int colour)
{
    g_bgmFadeTarget = (float)value;
    g_bgmFadeTrack = id + BGM_GROUP_300;
    g_fadeColourKind3 = colour;
    if (frames == 0)
    {
        g_bgmFadeStep = 0.0f;
        g_bgmFadeVolume = (float)value;
        g_bgmFadeTarget = (float)value;
        g_bgmFadeFramesLeft = 0;
        return 0;
    }
    g_bgmFadeFramesLeft = frames;
    g_bgmFadeStep = (g_bgmFadeTarget - g_bgmFadeVolume) / frames;
    return 0;
}
