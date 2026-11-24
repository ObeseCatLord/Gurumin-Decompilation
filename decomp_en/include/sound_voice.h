// The sound engine's playback objects (EN build; JP: sound3d.dll). Declared once here: include this header, never
// redeclare these types. A voice is one DirectSound secondary buffer plus its decoder (wave file, ACM codec or
// Ogg Vorbis reader). The engine owns two music stream voices and 24 sound effect voices.
#ifndef GURUMIN_EN_SOUND_VOICE_H
#define GURUMIN_EN_SOUND_VOICE_H

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <dsound.h>

#include "sound_file.h"

// Volume scale of the engine: 0 (silent) to 10000 (full).
const int SOUND_VOLUME_FULL = 10000;
// DirectSound attenuation (hundredths of a decibel) of a silent voice (0xffffd8f0).
const int SOUND_DS_VOLUME_SILENT = -10000;
// Longest fade the engine accepts, in milliseconds (a minute).
const int SOUND_FADE_MAX_MS = 60000;
// Fade length of the plain "fade out" command, in milliseconds.
const int SOUND_FADE_DEFAULT_MS = 1000;

// Listener axis value an all-zero front vector (y) or top vector (z) is replaced by, and the doppler factor.
const float LISTENER_DEFAULT_AXIS = -1.0f;
const float LISTENER_DOPPLER_FACTOR = 1.0f;

// Number of music stream voices and of sound effect voices.
const int SOUND_STREAM_COUNT = 2;
const int SOUND_EFFECT_COUNT = 24;
// Highest voice numbers the commands accept (streams 0..1; effect commands use the first twelve effect voices).
const unsigned SOUND_LAST_STREAM = SOUND_STREAM_COUNT - 1;
const unsigned SOUND_LAST_COMMAND_EFFECT = 11;

struct OggReader;

// Size of the canonical wave file header in front of the sound data.
const int WAVE_HEADER_BYTES = 0x2c;
// SoundSource::sourceKind values.
const int SOUND_SOURCE_ACM = 4;     // data converted by an ACM codec
const int SOUND_SOURCE_OGG = 5;     // Ogg Vorbis data decoded by an OggReader

// The data source of a voice (0x1a4 bytes): file, ACM decoder or Ogg Vorbis reader. Fields not used by a ported
// function yet are placeholders.
struct SoundSource
{
    int decoderOpen;                // 0x000: non-zero while the ACM decoder buffers and streams exist
    WAVEFORMATEX *adpcmFormat;      // 0x004: ADPCM format of the compressed data (allocated with the decoder)
    WAVEFORMATEX *pcmFormat;        // 0x008: PCM format the ADPCM data decodes to
    WAVEFORMATEX *streamAdpcmFormat;    // 0x00c: ADPCM format suggested for the streamed PCM format
    WAVEFORMATEX *streamPcmFormat;  // 0x010: PCM format of the streamed data
    char m_0014[0x40 - 0x14];       // 0x014: not used by ported code yet
    HACMSTREAM acmStreamA;          // 0x040: ACM stream paired with headerA
    HACMSTREAM acmStreamB;          // 0x044: ACM stream paired with headerB
    HACMSTREAM acmStreamC;          // 0x048: ACM stream paired with headerC
    ACMSTREAMHEADER headerC;        // 0x04c
    ACMSTREAMHEADER headerA;        // 0x0a0
    ACMSTREAMHEADER headerB;        // 0x0f4
    void *acmDestBuffer;            // 0x148: converted data
    void *acmSourceBuffer;          // 0x14c: compressed data read from the file
    void *progressCallback;         // 0x150: conversion progress callback
    int readPosition;               // 0x154: position the voice reads next
    int sourceKind;                 // 0x158: kind of source (4 and 5 = Ogg Vorbis)
    MmioFile file;                  // 0x15c: the sound file (a plain file or a section of an archive)
    unsigned dataSize;              // 0x16c: size of the sound data
    unsigned decodedPosition;       // 0x170: position within the decoded data
    int m_0174;                     // 0x174: set once the data size is known (cleared while opening)
    int isFileSource;               // 0x178: non-zero when the data is read straight from the file
    int m_017c;                     // 0x17c: not used by ported code yet
    void *pcmBuffer;                // 0x180: decoded sound data
    int m_0184;                     // 0x184: cleared when a position is set
    int m_0188;                     // 0x188: set when a position is set
    int isCompressed;               // 0x18c: set when the data is in the compressed layout
    int compressedParam;            // 0x190: value stored at the start of compressed data (sets its size ratio)
    int dataOffset;                 // 0x194: offset of the data in the file
    int m_0198;                     // 0x198: not used by ported code yet
    int m_019c;                     // 0x19c: not used by ported code yet
    OggReader *oggReader;           // 0x1a0: Ogg Vorbis reader of an Ogg source

    // Constructor: clears the file and decoder fields. EN 0x7be810.
    SoundSource();
    // Destructor: frees the decoder and the decoded data, closes the file. EN 0x7be8e0.
    ~SoundSource();
    // Allocates and fills the ACM formats and stream headers of the decoder (not when `skip` is set). Returns 1,
    // or 0 when skipped. EN 0x7bfa80.
    int InitDecoder(int skip);
    // Opens a sound file and decides by the end of its name how it is read (wave, compressed, Ogg, raw). EN 0x7bfe80.
    int OpenSource(const char *path);
    // Opens a file in the compressed layout and measures its decoded size. EN 0x7c0550.
    int OpenCompressed(const char *path);
    // Copies the next `count` decoded bytes of the sound to `destination` (decoding as needed). EN 0x7c0850.
    int ReadData(void *destination, unsigned count);
    // Closes the source (decoder and file). EN 0x7be980.
    int CloseSource();
    // Frees the buffers and ACM streams of the decoder. EN 0x7c1230.
    int FreeDecoder();
    // Closes the file handle and forgets the cached position (used by CloseSource).
    void CloseFile();
    // Moves the read position to `position` bytes of the data. EN 0x7c1360.
    void SeekTo(unsigned position);
    // Returns how far the source has been read. EN 0x7c1460.
    int GetPosition();
    // True when the whole sound has been read. EN 0x7bf120.
    bool IsAtEnd();
};

// Header of the decoded wave data in canonical RIFF/WAVE layout (0x2c bytes).
struct WaveFileHeader
{
    char riff[4];               // 0x00: "RIFF"
    DWORD riffSize;             // 0x04
    char wave[4];               // 0x08: "WAVE"
    char fmt[4];                // 0x0c: "fmt "
    DWORD fmtSize;              // 0x10
    WORD formatTag;             // 0x14
    WORD channels;              // 0x16
    DWORD sampleRate;           // 0x18
    DWORD bytesPerSecond;       // 0x1c
    WORD blockAlign;            // 0x20
    WORD bitsPerSample;         // 0x22
    char data[4];               // 0x24: "data"
    DWORD dataSize;             // 0x28
};

// One playback object (0x264 bytes): a source plus its DirectSound buffer, position and fade.
struct SoundVoice : SoundSource
{
    float position[3];              // 0x1a4: 3D position
    float velocity[3];              // 0x1b0: 3D velocity
    float distanceScale;            // 0x1bc: scales the distance to the listener
    int is3D;                       // 0x1c0: non-zero for a positioned sound
    int playing;                    // 0x1c4: non-zero while the voice is playing
    int m_01c8;                     // 0x1c8: set while a playback is starting
    int looping;                    // 0x1cc: non-zero when the sound loops
    unsigned dataEnd;               // 0x1d0: end of the data in the file
    unsigned loopStartByte;         // 0x1d4: loop start (bytes)
    unsigned loopEndByte;           // 0x1d8: loop end (bytes)
    int volume;                     // 0x1dc: current volume (0..SOUND_VOLUME_FULL)
    int fadeStopAtEnd;              // 0x1e0: non-zero when the fade ends by stopping the voice
    int fadeTarget;                 // 0x1e4: volume the fade moves to
    DWORD fadeLastTick;             // 0x1e8: GetTickCount at the last fade step
    int fadeRemainingMs;            // 0x1ec: milliseconds the fade still has to run (0 = no fade)
    int thirdFlag[3];               // 0x1f0: state of each third of the DirectSound buffer (refill bookkeeping)
    int m_01fc;                     // 0x1fc: not used by ported code yet
    DWORD playStartTick;            // 0x200: GetTickCount when playback started
    int m_0204;                     // 0x204: not used by ported code yet
    PCMWAVEFORMAT bufferFormat;     // 0x208: format of the DirectSound buffer (the buffer holds three seconds)
    WaveFileHeader waveHeader;      // 0x218: header of the sound data being played
    unsigned playCursor;            // 0x244: play cursor of the DirectSound buffer at the last update
    IDirectSound8 *directSound;     // 0x248: the DirectSound device
    int usesSharedDevice;           // 0x24c: non-zero when the device belongs to another voice (not released here)
    IDirectSoundBuffer *primaryBuffer;   // 0x250
    int usesSharedPrimary;          // 0x254: non-zero when the primary buffer belongs to another voice
    IDirectSoundBuffer8 *buffer;    // 0x258: the secondary buffer being played
    IDirectSound3DBuffer *buffer3d; // 0x25c
    IDirectSound3DListener *listener;    // 0x260

    // Constructor: clears the voice. EN 0x7c1a60.
    SoundVoice();
    // Sets the volume and applies it to the buffer when the voice is playing. EN 0x7c4680.
    int SetVolume(int newVolume);
    // Applies a volume (clamped to 0..SOUND_VOLUME_FULL) to the DirectSound buffer. EN 0x7c4510.
    int ApplyVolume(int newVolume, int force);
    // Starts a fade to a volume. EN 0x7c4620.
    int StartVolumeFade(int target, int milliseconds);
    // Starts the default one second fade to silence that stops the voice. EN 0x7c1ae0.
    int StartFadeOut();
    // Starts a fade to silence of the given length that stops the voice. EN 0x7c1b20.
    int StartFadeOutMs(int milliseconds);
    // Cancels a running fade, keeping the current volume. EN 0x7c3e30.
    int CancelFade();
    // Sets the loop range from sample positions. EN 0x7c3fc0.
    int SetLoopRange(int startSample, int endSample);
    // True while the buffer plays or a playback is starting. EN 0x7c1e80.
    int IsPlaying();
    // Stops playback and closes the source. EN 0x7c45b0.
    int Stop();
    // Hook called after an effect's volume was set; does nothing and returns 1. EN 0x7c1ad0.
    int AfterVolumeSet();
    // Sets the 3D position (and optionally velocity) of the voice. EN 0x7c4020.
    int SetPosition(const float *x, const float *y, const float *z, const float *velocityX, const float *velocityY,
                    const float *velocityZ);
    // Sets the DirectSound cooperative level of the voice's device to exclusive. EN 0x7c3ae0.
    int SetCooperativeLevel(HWND window);
    // Places the 3D listener: rolloff factor, position, front vector and top vector. An all-zero front vector
    // gets y = -1, a zero top z gets -1. Also remembers the position and front vector. EN 0x7c3e60.
    int SetListener(float rolloff, float x, float y, float z, float frontX, float frontY, float frontZ, float topX,
                    float topY, float topZ);
    // Pushes the stored position to DirectSound and updates the pan and volume from the listener. EN 0x7c41c0.
    int UpdatePosition();
    // Derives volume and pan of a software-positioned voice from the listener. EN 0x7c42d0.
    int UpdatePanFromListener();
    // Creates (or adopts the given) DirectSound device and primary buffer and sets up the 3D listener. EN 0x7c1ce0.
    int InitDevice(HWND window, IDirectSound8 *sharedDevice, IDirectSoundBuffer *sharedPrimary);
    // Starts a sound effect: file, looping flag, position and velocity (pointers to three floats each), a
    // "positioned" flag and the volume. Not ported. EN 0x7c2ac0.
    int PlayEffect(const char *path, int looping, const float *x, const float *y, const float *z, const float *vx,
                   const float *vy, const float *vz, int positioned, float volume);
    // Stops the voice and releases the DirectSound objects it holds. EN 0x7c3d30.
    int Release();
    // Returns the play position in samples. EN 0x7c1bd0.
    int GetPlayPosition();
    // Starts playing the sound file `path` (looping flag, start sample, loop start and end samples). EN 0x7c25b0.
    int Play(const char *path, int looping, int startSample, int loopStartSample, unsigned loopEndSample);
    // Fills part of the DirectSound buffer with the next sound data: locks `length` bytes at `offset`, reads (looping
    // back to the loop start at the end of the data, or padding with silence and stopping at the end of a one-shot
    // sound) and unlocks. `reserved` is not used. Returns 0 when the lock fails, else 1. EN 0x7c3b00.
    int FillBuffer(unsigned length, int reserved, unsigned offset);
    // One update step of a playing voice: advances its fade and refills the third of the buffer the play cursor just
    // left. Returns 0 when the voice is not playing. EN 0x7c1ef0.
    int UpdateBuffer();
    // Plays at the master volume at once (no fade). EN 0x7c29e0.
    int PlayAtOnce(const char *path, int looping, int startSample, int loopStartSample, int loopEndSample);
    // Plays with the default one second fade in to the master volume. EN 0x7c2830.
    int PlayFadeInDefault(const char *path, int looping, int startSample, int loopStartSample, int loopEndSample);
    // Plays with a fade in of `fadeMs` milliseconds to the master volume. EN 0x7c28f0.
    int PlayFadeIn(const char *path, int looping, int startSample, int loopStartSample, int loopEndSample,
                   int fadeMs);
};

#endif
