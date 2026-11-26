// Per-frame update of the background music (EN build). Ported from the JP source decomp/src/bgm_update.cpp
// (APP.dll 0x1005c680, unmatched) and the EN bytes.
#include "bgm.h"
#include "switches.h"

// Samples per millisecond at 44.1 kHz.
const float BGM_SAMPLES_PER_MILLISECOND = 44.1f;
// soundarc's position lags the audible music by this many samples.
const int BGM_SOUNDARC_POSITION_OFFSET = 0x9c4;
// Size limit of the spectrum table in bytes.
const int BGM_SPECTRUM_LIMIT = 0xaf000;
// Part of the distance a bar climbs and drops per update.
const float BAR_RISE = 0.8f;
const float BAR_FALL = 0.1f;

// Moves one bar towards the spectrum level: quickly up, slowly down.
static inline void EaseBar(float &bar, unsigned char level)
{
    float target = level;
    if (bar < target)
        bar = (target - bar) * BAR_RISE + bar;
    else
        bar = bar - (bar - target) * BAR_FALL;
}

// Starts the requested track (unless sounds are muted), computes the playback position g_bgmPosition in samples
// (from soundarc when it plays the music, else from the time since the track started, wrapping a looped track back
// by its loop length) and moves the left and right bar heights towards the spectrum frame at that position.
// Returns 1. Called every frame. EN 0x5a8d70.
extern "C" int Bgm_Update(void)
{
    if (g_bgmRequested >= 0 && g_bgmPlaying != g_bgmRequested && g_soundsMuted == 0)
        Bgm_Play(g_bgmRequested, 0);
    if (g_bgmBarsShown == 0 || g_bgmPositionFromSoundArc == 0)
    {
        g_bgmPosition = (int)((float)(GetTickCount() - g_bgmStartTick) * BGM_SAMPLES_PER_MILLISECOND);
        if (g_bgmPosition >= g_bgmTracks[g_bgmPlaying].b)
        {
            g_bgmPosition += g_bgmTracks[g_bgmPlaying].a - g_bgmTracks[g_bgmPlaying].b;
            g_bgmStartTick = (DWORD)((float)GetTickCount() - (float)g_bgmPosition / BGM_SAMPLES_PER_MILLISECOND);
        }
    }
    else
        g_bgmPosition = g_soundArcGetPosition(g_bgmStream) + BGM_SOUNDARC_POSITION_OFFSET + g_bgmStartOffset;
    if ((g_bgmPosition / BGM_SPECTRUM_SAMPLES_PER_FRAME) * 0x10 + 5 > BGM_SPECTRUM_LIMIT)
        g_bgmStartTick = GetTickCount();
    if (g_bgmPosition < 0)
    {
        g_bgmStartTick = GetTickCount();
        g_bgmPosition = 0;
    }
    if (g_bgmBarsShown != 0)
    {
        int frameOffset = (g_bgmPosition / BGM_SPECTRUM_SAMPLES_PER_FRAME) * BGM_SPECTRUM_FRAME_SIZE;
        const unsigned char *level = g_bgmSpectrum + frameOffset;
        float *bar = g_bgmBars;
        do
            EaseBar(*bar++, *level++);
        while (bar < g_bgmBars + BGM_SPECTRUM_BANDS);
        level = g_bgmSpectrum + frameOffset + BGM_SPECTRUM_RIGHT_OFFSET;
        do
            EaseBar(*bar++, *level++);
        while (bar < g_bgmBars + 2 * BGM_SPECTRUM_BANDS);
    }
    return 1;
}
