// Starting a music file on one of the two music streams (EN build). Ported from decomp/src/bgm_stream.cpp.
#include "bgm.h"
#include "engine.h"
#include "game_vars.h"

// Bgm_StartStream: plays music file `name`: sets the master music level from the volume option (0..10), stops or
// fades out (2 s) the stream that is playing, switches to the other stream and starts the new track on it. `a`..`d`
// are the loop and fade parameters passed to the sound engine's play command. Called by Bgm_Play.
extern "C" void Bgm_StartStream(const char *name, int a, int b, int c, int d, int fade)
{
    g_soundArcSetMusicAttenuation((BGM_VOLUME_STEPS - g_musicSetting) * BGM_ATTENUATION_PER_STEP);
    if (g_bgmStreamActive != 0)
    {
        if (fade != 0)
            g_soundArcFadeStream(g_bgmStream, BGM_FADE_OUT_MS);
        else
            g_soundArcStopStream(g_bgmStream);
    }
    g_bgmStreamActive = 1;
    g_bgmStream = (g_bgmStream - 1) & BGM_STREAM_MASK;
    g_soundArcPlayStreamCommand(g_bgmStream, MakeDataPath(name), c, a, b, d);
}
