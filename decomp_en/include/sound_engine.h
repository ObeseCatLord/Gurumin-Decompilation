// The sound engine's global state and its C interface (EN build; JP: sound3d.dll). Declared once here.
#ifndef GURUMIN_EN_SOUND_ENGINE_H
#define GURUMIN_EN_SOUND_ENGINE_H

#include <windows.h>

#include "sound_voice.h"

// Voice that owns the DirectSound device (and primary buffer) the other voices share. EN 0x029900c8.
extern "C" SoundVoice g_soundDevice;
// The music stream voices. EN 0x02990330.
extern "C" SoundVoice g_soundStreams[SOUND_STREAM_COUNT];
// The sound effect voices. EN 0x029907f8.
extern "C" SoundVoice g_soundEffects[SOUND_EFFECT_COUNT];
// Master music volume (0..SOUND_VOLUME_FULL) applied to the streams. EN 0x0297408c.
extern "C" int g_soundMusicVolume;
// Copy of the music volume that new playbacks fade up to. EN 0x0276b528.
extern "C" int g_soundMusicVolumeTarget;
// Master effect volume. EN 0x02974088.
extern "C" int g_soundEffectVolume;
// Copy of the effect volume kept for the effect code. EN 0x0276b524.
extern "C" int g_soundEffectVolumeCopy;
// Protects the music streams. EN 0x02974094.
extern "C" CRITICAL_SECTION g_soundStreamLock;
// Protects the sound effects. EN 0x029740ac.
extern "C" CRITICAL_SECTION g_soundEffectLock;

// Position of the listener, as last set by the game. EN 0x0276b500 (x, y, z).
extern "C" float g_soundListenerPosition[3];
// Front vector of the listener, as last set by the game. EN 0x0276b50c (x, y, z).
extern "C" float g_soundListenerFront[3];
// Handle of the system mixer used to read the Windows volume controls (0 when closed). EN 0x02973ef0.
extern "C" HMIXER g_soundMixer;
// Buffer mode chosen by the game (0, 1 or 2; selects the DirectSound buffer flags). EN 0x0276b520.
extern "C" int g_soundBufferMode;
// Tens and ones digits of the sound option code. EN 0x0276b518 and 0x0276b51c.
extern "C" int g_soundOptionTens;
extern "C" int g_soundOptionOnes;

// Window the sound engine was started with (0 when stopped). EN 0x02974084.
extern "C" HWND g_soundWindow;
// Set while the engine shuts down so the update thread leaves the voices alone. EN 0x029740c6.
extern "C" unsigned char g_soundShuttingDown;
// Index of the next command the update step reads from the queue. EN 0x029740c4.
extern "C" unsigned char g_soundQueueReadIndex;
// Number of commands waiting in the queue. EN 0x02974080.
extern "C" int g_soundQueuePending;
// Multimedia timer that runs the update step (-1 when none). EN 0x0090a510.
extern "C" MMRESULT g_soundTimerId;
// Pointers to loaded sound data blocks, freed when the engine stops. EN 0x0296b5f0 (48 entries).
const int SOUND_DATA_BLOCKS = 0x30;
extern "C" void *g_soundDataBlocks[SOUND_DATA_BLOCKS];

// Resets the engine's memory tracker. Not ported. EN 0x77bb90.
extern "C" int MemoryTracker_Reset(void);
// The update step: runs queued commands and refills the streams. Not ported. EN 0x7c51e0.
extern "C" void SoundEngine_Update(int unused);
// Multimedia timer callback (every 10 ms): runs the update step. EN 0x7c6210.
extern "C" void CALLBACK SoundEngine_TimerCallback(UINT id, UINT message, DWORD_PTR user, DWORD_PTR param1,
                                                    DWORD_PTR param2);
// Starts the engine for a window: creates the DirectSound device and all voices and the update timer. Returns 1, or
// 0 when already running or on failure. EN 0x7c5610.
extern "C" int SoundEngine_Init(HWND window);
// Stops the engine and frees every voice and data block. Returns 1, or 0 when it was not running. EN 0x7c5d10.
extern "C" int SoundEngine_Release(void);

// Number of (source, line) pairs the mixer volume cache holds: 64 sources of 64 lines.
const int SOUND_MIXER_CACHE_ENTRIES = 64 * 64;
// Which mixer volumes were already read: one byte per source * 64 + line. EN 0x02972ef0.
extern "C" char g_soundMixerCacheValid[SOUND_MIXER_CACHE_ENTRIES];
// The mixer volumes that were read. EN 0x0296eef0.
extern "C" int g_soundMixerCacheVolume[SOUND_MIXER_CACHE_ENTRIES];
// Mixer control ids of the volume sliders, by (source, line); 0xffffffff when the control does not exist.
extern "C" DWORD g_soundMixerControlSource0;    // EN 0x02974058: source 0, line 5
extern "C" DWORD g_soundMixerControlSource1;    // EN 0x02974070: source 1, line 5
extern "C" DWORD g_soundMixerControlSource2;    // EN 0x02974064: source 2, line 5
extern "C" DWORD g_soundMixerControlSource3Line5;   // EN 0x02973f8c: source 3, line 5
extern "C" DWORD g_soundMixerControlSource3Line6;   // EN 0x02973ef4: source 3, line 6
// Reads (and caches) the Windows mixer volume of a source and line; 0 when unavailable. EN 0x7c4760.
extern "C" int SoundEngine_GetMixerVolume(int source, int line);
// Closes the system mixer when open. Returns 0. EN 0x7c48e0.
extern "C" int SoundEngine_CloseMixer(void);
// Stores the buffer mode and the sound option code split into digits. Returns 1. EN 0x7c5f80.
extern "C" int SoundEngine_SetOptions(int bufferMode, int optionCode);
// Sets the master effect volume. Returns 1, or 0 when the engine is stopped. EN 0x7c6030.
extern "C" int SoundEngine_SetEffectMasterVolume(int volume);
// Starts a one second fade out of a music stream. Returns 1, or 0 for a bad stream. EN 0x7c5530.
extern "C" int SoundEngine_FadeOutStream(unsigned stream);
// Starts a fade out of `milliseconds` on a music stream. EN 0x7c5580.
extern "C" int SoundEngine_FadeOutStreamMs(unsigned stream, int milliseconds);
// Returns the play position (samples) of a playing stream, else 0. EN 0x7c55d0.
extern "C" int SoundEngine_GetStreamPosition(unsigned stream);
// Plays a file on a stream at the master volume at once. EN 0x7c57f0.
extern "C" int SoundEngine_PlayStreamAtOnce(unsigned stream, const char *path, int startSample, int loopStartSample,
                                            int loopEndSample, int looping);
// Plays a file on a stream with the default fade in. EN 0x7c58f0.
extern "C" int SoundEngine_PlayStreamFadeInDefault(unsigned stream, const char *path, int startSample,
                                                   int loopStartSample, int loopEndSample, int looping);
// Plays a file on a stream with a fade in of `fadeMs`. EN 0x7c5950.
extern "C" int SoundEngine_PlayStreamFadeIn(unsigned stream, const char *path, int startSample,
                                            int loopStartSample, int loopEndSample, int looping, int fadeMs);
// Stops a music stream. EN 0x7c5ca0.
extern "C" int SoundEngine_StopStream(unsigned stream);
// Sets the master music volume on every stream. EN 0x7c5e00.
extern "C" int SoundEngine_SetMusicVolume(int volume);
// Sets the loop range (samples) of a stream. EN 0x7c5f30.
extern "C" void SoundEngine_SetStreamLoopRange(unsigned stream, int startSample, int endSample);
// Starts a volume fade on a stream. EN 0x7c6180.
extern "C" int SoundEngine_FadeStreamVolume(unsigned stream, int target, int milliseconds);
// Sets the volume of a sound effect voice and cancels its fade. EN 0x7c6060.
extern "C" int SoundEngine_SetEffectVolume(unsigned effect, int volume);
// Sets the 3D position and velocity of a sound effect voice (0..23). Returns 1, or 0 for a bad voice. EN 0x7c5ff0.
extern "C" int SoundEngine_SetEffectPosition(int effect, const float *x, const float *y, const float *z,
                                             const float *velocityX, const float *velocityY,
                                             const float *velocityZ);
// Plays a sound effect on the first free effect voice (position given by pointers when `positioned`). Returns the
// voice number, or -1 when all are busy or sound is disabled. EN 0x7c5b50.
extern "C" int SoundEngine_PlayEffect(const char *path, const float *x, const float *y, const float *z,
                                      int positioned, int looping, float volume);
// Same with a velocity. Returns -1 when the effect was started or not (0 when disabled). EN 0x7c5bf0.
extern "C" int SoundEngine_PlayMovingEffect(const char *path, const float *x, const float *y, const float *z,
                                            const float *velocityX, const float *velocityY, const float *velocityZ,
                                            int positioned, int looping, float volume);
// Stops every playing sound effect. Returns 0. EN 0x7c60c0.
extern "C" int SoundEngine_StopAllEffects(void);
// Stops one sound effect voice. EN 0x7c6110.
extern "C" int SoundEngine_StopEffect(unsigned effect);

#endif
