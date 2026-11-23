// Per-frame update of the background music: starts the track the game asked for, keeps the playback position
// (in 44.1 kHz samples) and eases the bar heights shown for the music towards the spectrum frame at that position.

#include <windows.h>

#include "bgm_track.h"
#include "bgm_spectrum.h"

#include "sound_vars.h"
#include "game_vars.h"
#include "win_vars.h"
#include "char_vars.h"

extern "C" LRESULT FUN_10001c80(LPARAM param);              // soundarc command 0x225 (playback position)

#define SAMPLES_PER_MILLISECOND 44.1f
#define SOUNDARC_POSITION_OFFSET 0x9c4                      // soundarc's position lags the audible music by this much
#define SPECTRUM_FRAMES_LIMIT 0xaf000                       // size limit of the spectrum table, in bytes
#define BAR_RISE 0.8f                                       // part of the distance a bar climbs per update
#define BAR_FALL 0.1f                                       // part of the distance a bar drops per update

static inline float FallTowards(float current, float target)
{
    return current - (current - target) * BAR_FALL;
}

static inline void EaseBar(float &bar, unsigned char level)
{
    float target = level;
    if (bar < target)
        bar = (target - bar) * BAR_RISE + bar;
    else
        bar = FallTowards(bar, target);
}

// Eases the eight bars from `band` on towards the levels at `source` + `band`.
#define EASE_EIGHT_BARS(bars, band, levels, source)     do     {         EaseBar((bars)[(band) + 0], (levels)[(source) + (band) + 0]);         EaseBar((bars)[(band) + 1], (levels)[(source) + (band) + 1]);         EaseBar((bars)[(band) + 2], (levels)[(source) + (band) + 2]);         EaseBar((bars)[(band) + 3], (levels)[(source) + (band) + 3]);         EaseBar((bars)[(band) + 4], (levels)[(source) + (band) + 4]);         EaseBar((bars)[(band) + 5], (levels)[(source) + (band) + 5]);         EaseBar((bars)[(band) + 6], (levels)[(source) + (band) + 6]);         EaseBar((bars)[(band) + 7], (levels)[(source) + (band) + 7]);     } while (0)

// 0x1005c680 (Bgm_Update). Starts the requested track (DAT_104c012c, unless sounds are muted), then computes the
// playback position DAT_10254088 in samples: from soundarc when it plays the music, else from the time since the
// track started (wrapping a looped track back by its loop length). Then moves the left and right bar heights
// DAT_104bff20 towards the spectrum frame at that position. Returns 1.
extern "C" int FUN_1005c680(void)
{
    if (DAT_104c012c > -1 && DAT_10202358 != DAT_104c012c && DAT_104bfe6c == 0)
        Bgm_Play(DAT_104c012c, 0);
    if (DAT_1020aee4 == 0 || DAT_104c014c == 0)
    {
        DAT_10254088 = (int)((float)(GetTickCount() - DAT_104c00e4) * SAMPLES_PER_MILLISECOND);
        if (DAT_10254088 >= DAT_108db358[DAT_10202358].b)
        {
            DAT_10254088 += DAT_108db358[DAT_10202358].a - DAT_108db358[DAT_10202358].b;
            DAT_104c00e4 = (DWORD)((float)GetTickCount() - (float)DAT_10254088 / SAMPLES_PER_MILLISECOND);
        }
    }
    else
        DAT_10254088 = FUN_10001c80(DAT_104bfd58) + SOUNDARC_POSITION_OFFSET + DAT_104c00c0;
    if ((DAT_10254088 / BGM_SPECTRUM_SAMPLES_PER_FRAME) * 0x10 + 5 > SPECTRUM_FRAMES_LIMIT)
        DAT_104c00e4 = GetTickCount();
    if (DAT_10254088 < 0)
    {
        DAT_104c00e4 = GetTickCount();
        DAT_10254088 = 0;
    }
    if (DAT_1020aee4 != 0)
    {
        int frameOffset = (DAT_10254088 / BGM_SPECTRUM_SAMPLES_PER_FRAME) * BGM_SPECTRUM_FRAME_SIZE;
        for (int band = 0; band < BGM_SPECTRUM_BANDS; band += 8)
            EASE_EIGHT_BARS(DAT_104bff20, band, DAT_102564a8, frameOffset);
        for (int band = 0; band < BGM_SPECTRUM_BANDS; band += 8)
            EASE_EIGHT_BARS(DAT_104bff20, BGM_SPECTRUM_BANDS + band, DAT_102564a8,
                            frameOffset + BGM_SPECTRUM_RIGHT_OFFSET - BGM_SPECTRUM_BANDS);
    }
    return 1;
}
