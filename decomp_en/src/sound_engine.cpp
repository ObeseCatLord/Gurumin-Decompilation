// The sound engine's C interface: thin wrappers that check the engine state and the voice number, take the lock and
// call the voice (EN build; JP twin: sound3d.dll, not matched yet).
#include <stdlib.h>

#include "sound_arc.h"
#include "sound_engine.h"

// Starts the default one second fade out of a music stream. Returns 1, or 0 when the engine is not running or the
// stream does not exist. Called by the music code.
extern "C" int SoundEngine_FadeOutStream(unsigned stream)
{
    if (g_soundArcRunning == 0)
        return 0;
    if (stream <= SOUND_LAST_STREAM)
    {
        EnterCriticalSection(&g_soundStreamLock);
        int result = g_soundStreams[stream].StartFadeOut();
        LeaveCriticalSection(&g_soundStreamLock);
        return result;
    }
    return 0;
}

// Starts a fade out of `milliseconds` on a music stream. Returns 1, or 0 for a bad stream or a stopped engine.
extern "C" int SoundEngine_FadeOutStreamMs(unsigned stream, int milliseconds)
{
    if (g_soundArcRunning == 0)
        return 0;
    if (stream <= SOUND_LAST_STREAM)
    {
        EnterCriticalSection(&g_soundStreamLock);
        int result = g_soundStreams[stream].StartFadeOutMs(milliseconds);
        LeaveCriticalSection(&g_soundStreamLock);
        return result;
    }
    return 0;
}

// Returns the play position in samples of a stream that is playing; 0 otherwise. Called by the music code.
extern "C" int SoundEngine_GetStreamPosition(unsigned stream)
{
    if (g_soundArcRunning == 0 || stream > SOUND_LAST_STREAM || g_soundStreams[stream].playing == 0)
        return 0;
    return g_soundStreams[stream].GetPlayPosition();
}

// Plays a file on a music stream at the master volume at once. Returns the voice's result (0 for a bad stream or a
// stopped engine).
extern "C" int SoundEngine_PlayStreamAtOnce(unsigned stream, const char *path, int startSample,
                                            int loopStartSample, int loopEndSample, int looping)
{
    if (g_soundArcRunning == 0)
        return 0;
    if (stream <= SOUND_LAST_STREAM)
    {
        EnterCriticalSection(&g_soundStreamLock);
        int result = g_soundStreams[stream].PlayAtOnce(path, looping, startSample, loopStartSample, loopEndSample);
        LeaveCriticalSection(&g_soundStreamLock);
        return result;
    }
    return 0;
}

// Plays a file on a music stream with the default fade in. Returns the voice's result (0 for a bad stream).
extern "C" int SoundEngine_PlayStreamFadeInDefault(unsigned stream, const char *path, int startSample,
                                                   int loopStartSample, int loopEndSample, int looping)
{
    if (g_soundArcRunning == 0)
        return 0;
    if (stream <= SOUND_LAST_STREAM)
    {
        EnterCriticalSection(&g_soundStreamLock);
        int result = g_soundStreams[stream].PlayFadeInDefault(path, looping, startSample, loopStartSample,
                                                              loopEndSample);
        LeaveCriticalSection(&g_soundStreamLock);
        return result;
    }
    return 0;
}

// Plays a file on a music stream with a fade in of `fadeMs` milliseconds. Returns the voice's result.
extern "C" int SoundEngine_PlayStreamFadeIn(unsigned stream, const char *path, int startSample,
                                            int loopStartSample, int loopEndSample, int looping, int fadeMs)
{
    if (g_soundArcRunning == 0)
        return 0;
    if (stream <= SOUND_LAST_STREAM)
    {
        EnterCriticalSection(&g_soundStreamLock);
        int result = g_soundStreams[stream].PlayFadeIn(path, looping, startSample, loopStartSample, loopEndSample,
                                                       fadeMs);
        LeaveCriticalSection(&g_soundStreamLock);
        return result;
    }
    return 0;
}

// Stops a music stream. Returns 1 when it was playing, else 0. Called by the music code.
extern "C" int SoundEngine_StopStream(unsigned stream)
{
    if (g_soundArcRunning == 0)
        return 0;
    if (stream <= SOUND_LAST_STREAM)
    {
        EnterCriticalSection(&g_soundStreamLock);
        int result = g_soundStreams[stream].Stop();
        LeaveCriticalSection(&g_soundStreamLock);
        return result;
    }
    return 0;
}

// Sets the master music volume and applies it to every music stream. Returns 1, or 0 when the engine is stopped.
extern "C" int SoundEngine_SetMusicVolume(int volume)
{
    if (g_soundArcRunning == 0)
        return 0;
    EnterCriticalSection(&g_soundStreamLock);
    g_soundMusicVolume = volume;
    g_soundMusicVolumeTarget = volume;
    for (int stream = 0; stream < SOUND_STREAM_COUNT; stream++)
        g_soundStreams[stream].SetVolume(g_soundMusicVolume);
    LeaveCriticalSection(&g_soundStreamLock);
    return 1;
}

// Sets the loop range (samples) of a music stream. Called by the music code.
extern "C" void SoundEngine_SetStreamLoopRange(unsigned stream, int startSample, int endSample)
{
    if (g_soundArcRunning != 0 && stream <= SOUND_LAST_STREAM)
    {
        EnterCriticalSection(&g_soundStreamLock);
        g_soundStreams[stream].SetLoopRange(startSample, endSample);
        LeaveCriticalSection(&g_soundStreamLock);
    }
}

// Starts a volume fade on a music stream. Returns 1, or 0 for a bad stream or a stopped engine.
extern "C" int SoundEngine_FadeStreamVolume(unsigned stream, int target, int milliseconds)
{
    if (g_soundArcRunning == 0 || stream > SOUND_LAST_STREAM)
        return 0;
    g_soundStreams[stream].StartVolumeFade(target, milliseconds);
    return 1;
}

// Cancels the fade of a sound effect voice and sets its volume. Returns 1, or 0 for a bad voice.
extern "C" int SoundEngine_SetEffectVolume(unsigned effect, int volume)
{
    if (g_soundArcRunning == 0)
        return 0;
    if (effect <= SOUND_LAST_COMMAND_EFFECT)
    {
        EnterCriticalSection(&g_soundEffectLock);
        g_soundEffects[effect].CancelFade();
        g_soundEffects[effect].SetVolume(volume);
        g_soundEffects[effect].AfterVolumeSet();
        LeaveCriticalSection(&g_soundEffectLock);
        return 1;
    }
    return 0;
}

// Stops every sound effect that is playing. Returns 0. Called when a scene ends.
extern "C" int SoundEngine_StopAllEffects(void)
{
    if (g_soundArcRunning != 0)
    {
        EnterCriticalSection(&g_soundEffectLock);
        for (int effect = 0; effect < SOUND_EFFECT_COUNT; effect++)
        {
            if (g_soundEffects[effect].IsPlaying() != 0)
                g_soundEffects[effect].Stop();
        }
        LeaveCriticalSection(&g_soundEffectLock);
    }
    return 0;
}

// Stops one sound effect voice (the first half of the voices is addressable). Returns 1, or 0 for a bad voice.
extern "C" int SoundEngine_StopEffect(unsigned effect)
{
    if (g_soundArcRunning == 0)
        return 0;
    if (effect <= SOUND_LAST_COMMAND_EFFECT)
    {
        EnterCriticalSection(&g_soundEffectLock);
        g_soundEffects[effect].Stop();
        LeaveCriticalSection(&g_soundEffectLock);
        return 1;
    }
    return 0;
}

// Closes the system mixer when it is open. Returns 0. Called when the engine shuts down.
extern "C" int SoundEngine_CloseMixer(void)
{
    if (g_soundMixer != 0)
    {
        mixerClose(g_soundMixer);
        g_soundMixer = 0;
    }
    return 0;
}

// Stores the buffer mode (last digit of `bufferMode`) and the two digits of the sound option code. Returns 1. Called
// when the game starts its sound.
extern "C" int SoundEngine_SetOptions(int bufferMode, int optionCode)
{
    g_soundOptionTens = optionCode / 10;
    g_soundOptionOnes = optionCode % 10;
    g_soundBufferMode = bufferMode % 10;
    return 1;
}

// Sets the master effect volume (and the copy the effect code reads). Returns 1, or 0 when the engine is stopped.
extern "C" int SoundEngine_SetEffectMasterVolume(int volume)
{
    if (g_soundArcRunning == 0)
        return 0;
    g_soundEffectVolume = volume;
    g_soundEffectVolumeCopy = volume;
    return 1;
}

// Sets the 3D position (and velocity) of a sound effect voice. Returns 1, or 0 when the voice number is out of range.
// Called by the effect code every frame for moving sounds.
extern "C" int SoundEngine_SetEffectPosition(int effect, const float *x, const float *y, const float *z,
                                             const float *velocityX, const float *velocityY,
                                             const float *velocityZ)
{
    if (effect < 0)
        return 0;
    if (effect > SOUND_EFFECT_COUNT - 1)
        return 0;
    g_soundEffects[effect].SetPosition(x, y, z, velocityX, velocityY, velocityZ);
    return 1;
}

// Multimedia timer callback, called every 10 ms: runs the engine's update step. Installed by SoundEngine_Init.
extern "C" void CALLBACK SoundEngine_TimerCallback(UINT id, UINT message, DWORD_PTR user, DWORD_PTR param1,
                                                    DWORD_PTR param2)
{
    SoundEngine_Update(0);
}

// Interval and resolution (milliseconds) of the update timer.
static const UINT SOUND_TIMER_INTERVAL_MS = 10;
static const UINT SOUND_TIMER_RESOLUTION_MS = 5;
// Timer resolution requested from Windows for the whole time the engine runs.
static const UINT SOUND_TIMER_PERIOD_MS = 1;

// Starts the engine for `window`: creates the DirectSound device, initialises every voice from it, sets up the locks
// and starts the 10 ms update timer. Returns 1, or 0 when the engine already runs or the device cannot be created.
// Called once when the game starts.
extern "C" int SoundEngine_Init(HWND window)
{
    if (g_soundArcRunning == 1)
        return 0;
    MemoryTracker_Reset();
    for (int block = 0; block < SOUND_DATA_BLOCKS; block++)
        g_soundDataBlocks[block] = 0;
    if (g_soundWindow != 0)
        return 0;
    g_soundWindow = window;
    if (g_soundDevice.InitDevice(window, 0, 0) == 0)
        return 0;
    g_soundQueueReadIndex = 0;
    g_soundArcQueueNext = 0;
    g_soundQueuePending = 0;
    InitializeCriticalSection(&g_soundStreamLock);
    InitializeCriticalSection(&g_soundEffectLock);
    timeBeginPeriod(SOUND_TIMER_PERIOD_MS);
    g_soundShuttingDown = 0;
    for (int stream = 0; stream < SOUND_STREAM_COUNT; stream++)
        g_soundStreams[stream].InitDevice(g_soundWindow, g_soundDevice.directSound, g_soundDevice.primaryBuffer);
    for (int effect = 0; effect < SOUND_EFFECT_COUNT; effect++)
        g_soundEffects[effect].InitDevice(g_soundWindow, g_soundDevice.directSound, g_soundDevice.primaryBuffer);
    g_soundTimerId = timeSetEvent(SOUND_TIMER_INTERVAL_MS, SOUND_TIMER_RESOLUTION_MS, SoundEngine_TimerCallback, 0,
                                  TIME_PERIODIC);
    g_soundArcRunning = 1;
    return 1;
}

// Stops the engine: kills the timer, releases every voice and the device, deletes the locks and frees the loaded
// sound data blocks. Returns 1, or 0 when the engine was not running. Called when the game ends.
extern "C" int SoundEngine_Release(void)
{
    if (g_soundArcRunning == 0)
        return 0;
    g_soundArcRunning = 0;
    if (g_soundTimerId != (MMRESULT)-1)
        timeKillEvent(g_soundTimerId);
    g_soundTimerId = (MMRESULT)-1;
    g_soundShuttingDown = 1;
    for (int stream = 0; stream < SOUND_STREAM_COUNT; stream++)
        g_soundStreams[stream].Release();
    for (int effect = 0; effect < SOUND_EFFECT_COUNT; effect++)
        g_soundEffects[effect].Release();
    g_soundDevice.Release();
    g_soundWindow = 0;
    timeEndPeriod(SOUND_TIMER_PERIOD_MS);
    DeleteCriticalSection(&g_soundStreamLock);
    DeleteCriticalSection(&g_soundEffectLock);
    for (int block = 0; block < SOUND_DATA_BLOCKS; block++)
    {
        if (g_soundDataBlocks[block] != 0)
        {
            free(g_soundDataBlocks[block]);
            g_soundDataBlocks[block] = 0;
        }
    }
    return 1;
}

// Plays a sound effect on the first effect voice that is not playing. Returns the number of the voice used, or -1
// when every voice is busy or sound is disabled or stopped. Called by the game's effect code.
extern "C" int SoundEngine_PlayEffect(const char *path, const float *x, const float *y, const float *z,
                                      int positioned, int looping, float volume)
{
    if (g_soundArcDisabled != 0)
        return -1;
    if (g_soundArcRunning == 0)
        return -1;
    EnterCriticalSection(&g_soundEffectLock);
    int effect;
    for (effect = 0; effect < SOUND_EFFECT_COUNT; effect++)
    {
        if (g_soundEffects[effect].IsPlaying() == 0)
        {
            g_soundEffects[effect].PlayEffect(path, looping, x, y, z, 0, 0, 0, positioned, volume);
            break;
        }
    }
    LeaveCriticalSection(&g_soundEffectLock);
    if (effect == SOUND_EFFECT_COUNT)
        effect = -1;
    return effect;
}

// Plays a moving sound effect (position and velocity) on the first free effect voice. Returns -1 once it has looked
// for a voice, or 0 when sound is disabled or stopped. Called by the game's effect code.
extern "C" int SoundEngine_PlayMovingEffect(const char *path, const float *x, const float *y, const float *z,
                                            const float *velocityX, const float *velocityY, const float *velocityZ,
                                            int positioned, int looping, float volume)
{
    if (g_soundArcDisabled != 0)
        return 0;
    if (g_soundArcRunning == 0)
        return 0;
    EnterCriticalSection(&g_soundEffectLock);
    for (int effect = 0; effect < SOUND_EFFECT_COUNT; effect++)
    {
        if (g_soundEffects[effect].IsPlaying() == 0)
        {
            g_soundEffects[effect].PlayEffect(path, looping, x, y, z, velocityX, velocityY, velocityZ, positioned,
                                              volume);
            LeaveCriticalSection(&g_soundEffectLock);
            return -1;
        }
    }
    LeaveCriticalSection(&g_soundEffectLock);
    return -1;
}

// Lines per source in the mixer volume cache index.
const int SOUND_MIXER_LINES = 64;
// Line numbers the volume sliders are looked up for.
const int SOUND_MIXER_LINE_A = 5;
const int SOUND_MIXER_LINE_B = 6;
// Mixer control id of a slider that does not exist.
const DWORD SOUND_MIXER_NO_CONTROL = 0xffffffff;
// Sources that have volume sliders.
const int SOUND_MIXER_SOURCE_COUNT = 4;

// Reads the Windows mixer volume of a source and line and caches it (a pair is looked up only once, failures cache
// 0 for the volume). Reopens the mixer handle when it went stale. Returns the volume magnitude, or 0 when the mixer or
// the control is missing. Not called by the game code that survived in this build.
extern "C" int SoundEngine_GetMixerVolume(int source, int line)
{
    int cacheIndex = source * SOUND_MIXER_LINES + line;
    if (g_soundMixerCacheValid[cacheIndex] != 0)
        return g_soundMixerCacheVolume[cacheIndex];
    g_soundMixerCacheValid[cacheIndex] = 1;
    if (g_soundMixer != 0)
    {
        MIXERCAPS caps;
        memset(&caps, 0, sizeof(caps));
        MMRESULT result = mixerGetDevCaps((UINT_PTR)g_soundMixer, &caps, sizeof(caps));
        if (result == MMSYSERR_INVALHANDLE)
        {
            if (mixerOpen(&g_soundMixer, 0, 0, 0, 0) == 0)
                return g_soundMixerCacheVolume[cacheIndex];
        }
        else if (result == MMSYSERR_NOERROR)
        {
            MIXERCONTROLDETAILS details;
            memset(&details, 0, sizeof(details));
            details.dwControlID = SOUND_MIXER_NO_CONTROL;
            switch (source)
            {
            case 0:
                if (line != SOUND_MIXER_LINE_A)
                    return 0;
                details.dwControlID = g_soundMixerControlSource0;
                break;
            case 1:
                if (line != SOUND_MIXER_LINE_A)
                    return 0;
                details.dwControlID = g_soundMixerControlSource1;
                break;
            case 2:
                if (line != SOUND_MIXER_LINE_A)
                    return 0;
                details.dwControlID = g_soundMixerControlSource2;
                break;
            case 3:
                switch (line)
                {
                case SOUND_MIXER_LINE_A:
                    details.dwControlID = g_soundMixerControlSource3Line5;
                    break;
                case SOUND_MIXER_LINE_B:
                    details.dwControlID = g_soundMixerControlSource3Line6;
                    break;
                default:
                    return 0;
                }
                break;
            default:
                return 0;
            }
            if (details.dwControlID != SOUND_MIXER_NO_CONTROL)
            {
                int raw;
                EnterCriticalSection(&g_soundStreamLock);
                details.cbStruct = sizeof(details);
                details.paDetails = &raw;
                details.cbDetails = sizeof(raw);
                details.cChannels = 1;
                int volume;
                if (mixerGetControlDetails((HMIXEROBJ)g_soundMixer, &details, MIXER_GETCONTROLDETAILSF_VALUE) == 0)
                {
                    volume = raw;
                    if (raw < 0)
                        volume = -raw;
                }
                else
                    volume = 0;
                LeaveCriticalSection(&g_soundStreamLock);
                g_soundMixerCacheVolume[cacheIndex] = volume;
                return volume;
            }
        }
    }
    return 0;
}
