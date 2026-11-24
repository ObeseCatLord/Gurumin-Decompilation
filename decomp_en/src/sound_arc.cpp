// Sound engine command queue (EN build). JP twin: app_0002.cpp.
#include <string.h>
#include <windows.h>

#include "sound_arc.h"

// Queues "play `path` on music stream `stream`" with four playback parameters. Returns -1 when queued; 0 when the
// sound engine is not running or the stream does not exist. Called by the music code.
extern "C" int SoundArc_PlayStream(int stream, const char *path, int a, int b, int c, int d)
{
    if (g_soundArcRunning == 0)
        return 0;
    if ((unsigned)stream <= SOUND_ARC_LAST_STREAM)
    {
        unsigned char slot = g_soundArcQueueNext;
        strcpy(g_soundArcQueue[slot].path, path);
        g_soundArcQueueNext++;
        g_soundArcQueue[slot].command = SOUND_ARC_COMMAND_PLAY_STREAM;
        g_soundArcQueue[slot].stream = stream;
        g_soundArcQueue[slot].params[0] = a;
        g_soundArcQueue[slot].params[1] = b;
        g_soundArcQueue[slot].params[2] = c;
        g_soundArcQueue[slot].params[3] = d;
        g_soundArcQueue[slot].queuedAt = 0;
        return -1;
    }
    return 0;
}

// Queues "play 3D sound effect" with the sound id and nine values (position, volume, pitch and so on), stamped with
// the current time. Returns 0. Called by the sound effect code.
extern "C" int SoundArc_Play3D(float id, float a, float b, float c, float d, float e, float f, float g, float h, float i)
{
    if (g_soundArcRunning != 0)
    {
        g_soundArcQueue[g_soundArcQueueNext].command = SOUND_ARC_COMMAND_PLAY_3D;
        g_soundArcQueue[g_soundArcQueueNext].values[0] = a;
        g_soundArcQueue[g_soundArcQueueNext].values[1] = b;
        g_soundArcQueue[g_soundArcQueueNext].values[2] = c;
        g_soundArcQueue[g_soundArcQueueNext].values[3] = d;
        g_soundArcQueue[g_soundArcQueueNext].values[4] = e;
        g_soundArcQueue[g_soundArcQueueNext].values[5] = f;
        g_soundArcQueue[g_soundArcQueueNext].values[6] = g;
        g_soundArcQueue[g_soundArcQueueNext].values[7] = h;
        g_soundArcQueue[g_soundArcQueueNext].values[8] = i;
        g_soundArcQueue[g_soundArcQueueNext].values[9] = id;
        g_soundArcQueue[g_soundArcQueueNext].queuedAt = GetTickCount();
        g_soundArcQueueNext++;
    }
    return 0;
}

// Queues "play file on music stream" (fading variant) with four playback parameters. Returns -1 when queued; 0 when the
// sound engine is not running or the stream does not exist. Called by the music code.
extern "C" int SoundArc_PlayStreamFade(int stream, const char *path, int a, int b, int c, int d)
{
    if (g_soundArcRunning == 0)
        return 0;
    if ((unsigned)stream <= SOUND_ARC_LAST_STREAM)
    {
        unsigned char slot = g_soundArcQueueNext;
        strcpy(g_soundArcQueue[slot].path, path);
        g_soundArcQueueNext++;
        g_soundArcQueue[slot].command = SOUND_ARC_COMMAND_PLAY_STREAM_FADE;
        g_soundArcQueue[slot].stream = stream;
        g_soundArcQueue[slot].params[0] = a;
        g_soundArcQueue[slot].params[1] = b;
        g_soundArcQueue[slot].params[2] = c;
        g_soundArcQueue[slot].params[3] = d;
        g_soundArcQueue[slot].queuedAt = 0;
        return -1;
    }
    return 0;
}

// Same as SoundArc_PlayStreamFade with a fifth playback parameter. Called by the music code.
extern "C" int SoundArc_PlayStreamFadeEx(int stream, const char *path, int a, int b, int c, int d, int e)
{
    if (g_soundArcRunning == 0)
        return 0;
    if ((unsigned)stream <= SOUND_ARC_LAST_STREAM)
    {
        unsigned char slot = g_soundArcQueueNext;
        strcpy(g_soundArcQueue[slot].path, path);
        g_soundArcQueueNext++;
        g_soundArcQueue[slot].command = SOUND_ARC_COMMAND_PLAY_STREAM_FADE;
        g_soundArcQueue[slot].stream = stream;
        g_soundArcQueue[slot].params[0] = a;
        g_soundArcQueue[slot].params[1] = b;
        g_soundArcQueue[slot].params[2] = c;
        g_soundArcQueue[slot].params[3] = d;
        g_soundArcQueue[slot].params[4] = e;
        g_soundArcQueue[slot].queuedAt = 0;
        return -1;
    }
    return 0;
}

// Queues "play sound file" (the whole path, a position when `positioned` is set, a flag, and a float value). When
// `now` is set the command is stamped as due at once (time 0). Returns -1 (also when the command was ignored because
// sound is disabled or the engine is not running). Called by the game's sound effect code.
extern "C" int SoundArc_PlayFile(const char *path, const float *x, const float *y, const float *z, int positioned, int now,
                                 float value)
{
    if (g_soundArcDisabled == 0 && g_soundArcRunning != 0)
    {
        unsigned char slot = g_soundArcQueueNext;
        strcpy(g_soundArcQueue[slot].path, path);
        g_soundArcQueue[slot].command = SOUND_ARC_COMMAND_PLAY_FILE;
        if (positioned == 0 || x == 0)
        {
            g_soundArcQueue[slot].values[0] = 0;
            g_soundArcQueue[slot].values[1] = 0;
            g_soundArcQueue[slot].values[2] = 0;
        }
        else
        {
            g_soundArcQueue[slot].values[0] = *x;
            g_soundArcQueue[slot].values[1] = *y;
            g_soundArcQueue[slot].values[2] = *z;
        }
        g_soundArcQueue[slot].params[2] = positioned;
        g_soundArcQueue[slot].params[3] = now;
        g_soundArcQueue[slot].values[5] = value;
        g_soundArcQueue[slot].queuedAt = GetTickCount();
        if (now != 0)
            g_soundArcQueue[slot].queuedAt = 0;
        g_soundArcQueueNext++;
    }
    return -1;
}
