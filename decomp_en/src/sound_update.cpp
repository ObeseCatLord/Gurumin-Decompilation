// The sound engine's update step (EN build; JP twin: sound3d.dll 10002f90, not matched).
#include <string.h>

#include "sound_arc.h"
#include "sound_engine.h"

// SoundEngine_Update: the engine thread's step (every 10 ms from the multimedia timer). Runs the commands queued since
// the last step (music stream plays, 3D listener moves, positioned sound effects, dropping commands older than a second
// and earlier copies of the same effect), then refills every music stream. A shutdown request leaves the step at once
// and acknowledges it. Called by SoundEngine_TimerCallback and once by SoundEngine_Init.
extern "C" void SoundEngine_Update(int unused)
{
    unsigned char end = g_soundArcQueueNext;
    unsigned char read = g_soundQueueReadIndex;
    while (end != read)
    {
        if (g_soundArcQueue[read].queuedAt != 0)
        {
            if (g_soundArcQueue[read].queuedAt < GetTickCount() - SOUND_ARC_COMMAND_LIFETIME_MS)
                g_soundArcQueue[read].command = 0;
        }
        if (g_soundArcQueue[read].command == SOUND_ARC_COMMAND_PLAY_STREAM_FADE_MS)
        {
            EnterCriticalSection(&g_soundStreamLock);
            g_soundStreams[g_soundArcQueue[read].stream].PlayFadeIn(g_soundArcQueue[read].path, g_soundArcQueue[read].params[3], g_soundArcQueue[read].params[0],
                                                       g_soundArcQueue[read].params[1], g_soundArcQueue[read].params[2], g_soundArcQueue[read].params[4]);
            g_soundStreams[g_soundArcQueue[read].stream].UpdateBuffer();
            g_soundArcQueue[g_soundQueueReadIndex].command = 0;
            LeaveCriticalSection(&g_soundStreamLock);
        }
        else if (g_soundArcQueue[read].command == SOUND_ARC_COMMAND_PLAY_STREAM_FADE)
        {
            EnterCriticalSection(&g_soundStreamLock);
            g_soundStreams[g_soundArcQueue[read].stream].PlayFadeInDefault(g_soundArcQueue[read].path, g_soundArcQueue[read].params[3], g_soundArcQueue[read].params[0],
                                                              g_soundArcQueue[read].params[1], g_soundArcQueue[read].params[2]);
            g_soundStreams[g_soundArcQueue[read].stream].UpdateBuffer();
            g_soundArcQueue[read].command = 0;
            LeaveCriticalSection(&g_soundStreamLock);
        }
        else if (g_soundArcQueue[read].command == SOUND_ARC_COMMAND_PLAY_STREAM)
        {
            EnterCriticalSection(&g_soundStreamLock);
            g_soundStreams[g_soundArcQueue[read].stream].PlayAtOnce(g_soundArcQueue[read].path, g_soundArcQueue[read].params[3], g_soundArcQueue[read].params[0],
                                                       g_soundArcQueue[read].params[1], g_soundArcQueue[read].params[2]);
            g_soundStreams[g_soundArcQueue[read].stream].UpdateBuffer();
            g_soundArcQueue[read].command = 0;
            LeaveCriticalSection(&g_soundStreamLock);
        }
        else if (g_soundArcQueue[read].command == SOUND_ARC_COMMAND_PLAY_3D)
        {
            g_soundDevice.SetListener(g_soundArcQueue[read].values[9], g_soundArcQueue[read].values[0], g_soundArcQueue[read].values[1], g_soundArcQueue[read].values[2],
                                      g_soundArcQueue[read].values[3], g_soundArcQueue[read].values[4], g_soundArcQueue[read].values[5], g_soundArcQueue[read].values[6],
                                      g_soundArcQueue[read].values[7], g_soundArcQueue[read].values[8]);
            g_soundArcQueue[read].command = 0;
            for (int effect = 0; effect < SOUND_EFFECT_COUNT; effect++)
                g_soundEffects[effect].UpdatePosition();
        }
        else if (g_soundArcQueue[read].command == SOUND_ARC_COMMAND_PLAY_FILE)
        {
            for (unsigned char later = read; end != later; later++)
            {
                if (later != read && g_soundArcQueue[later].command == SOUND_ARC_COMMAND_PLAY_FILE &&
                    _stricmp(g_soundArcQueue[read].path, g_soundArcQueue[later].path) == 0)
                    g_soundArcQueue[later].command = 0;
            }
            if (g_soundArcQueue[read].params[2] != 0)
                SoundEngine_PlayEffect(g_soundArcQueue[read].path, &g_soundArcQueue[read].values[0],
                                       &g_soundArcQueue[read].values[1], &g_soundArcQueue[read].values[2],
                                       g_soundArcQueue[read].params[2], g_soundArcQueue[read].params[3],
                                       g_soundArcQueue[read].values[5]);
            else
                SoundEngine_PlayEffect(g_soundArcQueue[read].path, 0, 0, 0, 0, g_soundArcQueue[read].params[3],
                                       g_soundArcQueue[read].values[5]);
            g_soundArcQueue[read].command = 0;
        }
        if (g_soundShuttingDown != 0)
        {
            g_soundShuttingDown = 2;
            return;
        }
        read++;
    }
    g_soundQueueReadIndex = read;
    if (g_soundArcRunning != 0)
    {
        for (int stream = 0; stream < SOUND_STREAM_COUNT; stream++)
        {
            EnterCriticalSection(&g_soundStreamLock);
            g_soundStreams[stream].UpdateBuffer();
            LeaveCriticalSection(&g_soundStreamLock);
        }
    }
}
