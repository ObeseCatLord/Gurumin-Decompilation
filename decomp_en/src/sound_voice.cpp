// Sound engine voices (EN build; JP twin: sound3d.dll, not matched yet).
#include <stddef.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "engine_memory.h"
#include "sound_engine.h"
#include "sound_ogg.h"
#include "sound_voice.h"

static_assert(offsetof(SoundVoice, volume) == 0x1dc, "SoundVoice layout");
static_assert(offsetof(SoundVoice, buffer) == 0x258, "SoundVoice layout");
static_assert(offsetof(SoundVoice, listener) == 0x260, "SoundVoice layout");
static_assert(sizeof(ACMSTREAMHEADER) == 0x54, "ACMSTREAMHEADER size");

// Applies a volume (0..SOUND_VOLUME_FULL, clamped) to the DirectSound buffer on a cubic curve. Unless `force` is set
// it does nothing for a voice that is not playing. Returns 1 when applied, 0 when skipped. Called by the fade code.
int SoundVoice::ApplyVolume(int newVolume, int force)
{
    if (directSound == 0)
        return 0;
    if (primaryBuffer == 0 || buffer == 0)
        return 0;
    if (newVolume < 0)
        newVolume = 0;
    else if (newVolume > SOUND_VOLUME_FULL)
        newVolume = SOUND_VOLUME_FULL;
    if (force == 0 && IsPlaying() == 0)
        return 0;
    float level = (float)newVolume / (float)SOUND_VOLUME_FULL;
    buffer->SetVolume((int)-(level * level * level * (float)SOUND_VOLUME_FULL));
    return 1;
}

// Sets the loop range of the voice from sample positions; converts them to byte positions of the data. Returns 1.
// Called by the "set loop position" command.
int SoundVoice::SetLoopRange(int startSample, int endSample)
{
    endSample = waveHeader.bitsPerSample * endSample;
    startSample = waveHeader.bitsPerSample * startSample;
    loopStartByte = (startSample / 8) * bufferFormat.wf.nChannels;
    int endByte = (endSample / 8) * bufferFormat.wf.nChannels;
    loopEndByte = endByte;
    dataEnd = endByte + WAVE_HEADER_BYTES;
    return 1;
}

// Returns 1 while the DirectSound buffer is playing (or a playback is starting), else 0. Restores a lost buffer.
// Called all over the voice code before it touches the buffer.
int SoundVoice::IsPlaying()
{
    if (buffer != 0)
    {
        DWORD status;
        buffer->GetStatus(&status);
        if (status & DSBSTATUS_BUFFERLOST)
            buffer->Restore();
        if (playing != 0)
            return 1;
        if (m_01c8 != 0 && (status & DSBSTATUS_TERMINATED) == 0)
        {
            if (status & DSBSTATUS_PLAYING)
                return 1;
            if (status & DSBSTATUS_LOOPING)
                return 1;
            if (m_0204 != 0)
                GetTickCount();
        }
    }
    return 0;
}

// Sets the voice volume and applies it to the buffer when the voice is playing. Returns 1. Called by the fade code
// and by the global volume commands.
int SoundVoice::SetVolume(int newVolume)
{
    volume = newVolume;
    if (IsPlaying() != 0)
        ApplyVolume(newVolume, 0);
    return 1;
}

// Starts a fade of the volume to `target` over `milliseconds`; with 0 the volume changes at once. Returns 1.
// Called by the "volume fade" command.
int SoundVoice::StartVolumeFade(int target, int milliseconds)
{
    fadeStopAtEnd = 0;
    fadeTarget = target;
    fadeRemainingMs = milliseconds;
    fadeLastTick = GetTickCount();
    if (milliseconds == 0)
    {
        volume = target;
        if (IsPlaying() != 0)
            ApplyVolume(target, 0);
    }
    return 1;
}

// Starts the default fade to silence that stops the voice at its end. Returns 1. Called by the "fade out" command.
int SoundVoice::StartFadeOut()
{
    fadeStopAtEnd = 0;
    fadeTarget = SOUND_VOLUME_FULL;
    fadeRemainingMs = SOUND_FADE_DEFAULT_MS;
    fadeLastTick = GetTickCount();
    fadeStopAtEnd = 1;
    return 1;
}

// Starts a fade to silence of `milliseconds` (clamped to 1..SOUND_FADE_MAX_MS) that stops the voice at its end.
// Returns 1. Called by the "fade out with time" command.
int SoundVoice::StartFadeOutMs(int milliseconds)
{
    if (milliseconds < 1)
        milliseconds = 1;
    else if (milliseconds > SOUND_FADE_MAX_MS)
        milliseconds = SOUND_FADE_MAX_MS;
    fadeStopAtEnd = 0;
    fadeTarget = SOUND_VOLUME_FULL;
    fadeRemainingMs = milliseconds;
    fadeLastTick = GetTickCount();
    if (milliseconds == 0)
    {
        volume = SOUND_VOLUME_FULL;
        if (IsPlaying() != 0 && directSound != 0 && primaryBuffer != 0 && buffer != 0)
        {
            if (IsPlaying() != 0)
                buffer->SetVolume(SOUND_DS_VOLUME_SILENT);
        }
    }
    fadeStopAtEnd = 1;
    return 1;
}

// Cancels a running fade so the voice keeps its current volume. Returns 0. Called before a new volume is set.
int SoundVoice::CancelFade()
{
    fadeTarget = volume;
    fadeStopAtEnd = 0;
    fadeRemainingMs = 0;
    return 0;
}

// Constructor of a source: clears the file and decoder fields. Called by the voice constructor.
SoundSource::SoundSource()
{
    file.handle = 0;
    file.isPlain = 0;
    m_0174 = 0;
    decodedPosition = 0;
    decodeBuffer[0] = 0;
    decodeBuffer[2] = 0;
    decodeBuffer[1] = 0;
    decodeBuffer[3] = 0;
    m_019c = 0;
    acmStreamA = 0;
    acmStreamB = 0;
    acmStreamC = 0;
    pcmBuffer = 0;
    acmDestBuffer = 0;
    acmSourceBuffer = 0;
}

// Constructor of a voice: clears the DirectSound pointers and flags (the source part first). Called when the engine
// starts.
SoundVoice::SoundVoice()
{
    usesSharedDevice = 0;
    directSound = 0;
    usesSharedPrimary = 0;
    primaryBuffer = 0;
    buffer = 0;
    buffer3d = 0;
    playing = 0;
    m_01c8 = 0;
    listener = 0;
}

// Frees the decoder buffers and closes the ACM streams (with their headers). Returns 1, or 0 when the decoder was
// never opened. Called when a source is closed or released.
int SoundSource::FreeDecoder()
{
    if (decoderOpen == 0)
        return 0;
    if (acmStreamB != 0)
    {
        acmStreamUnprepareHeader(acmStreamB, &headerB, 0);
        acmStreamClose(acmStreamB, 0);
        acmStreamB = 0;
    }
    if (acmStreamA != 0)
    {
        acmStreamUnprepareHeader(acmStreamA, &headerA, 0);
        acmStreamClose(acmStreamA, 0);
        acmStreamA = 0;
    }
    if (acmStreamC != 0)
    {
        acmStreamUnprepareHeader(acmStreamC, &headerC, 0);
        acmStreamClose(acmStreamC, 0);
        acmStreamC = 0;
    }
    if (decodeBuffer[1] != 0)
    {
        g_engineFree(decodeBuffer[1]);
        decodeBuffer[1] = 0;
    }
    if (decodeBuffer[0] != 0)
    {
        g_engineFree(decodeBuffer[0]);
        decodeBuffer[0] = 0;
    }
    if (decodeBuffer[3] != 0)
    {
        g_engineFree(decodeBuffer[3]);
        decodeBuffer[3] = 0;
    }
    if (decodeBuffer[2] != 0)
    {
        g_engineFree(decodeBuffer[2]);
        decodeBuffer[2] = 0;
    }
    if (acmSourceBuffer != 0)
    {
        g_engineFree(acmSourceBuffer);
        acmSourceBuffer = 0;
    }
    if (acmDestBuffer != 0)
    {
        g_engineFree(acmDestBuffer);
        acmDestBuffer = 0;
    }
    return 1;
}

// Closes the file (when open) and forgets the cached position. Part of CloseSource.
inline void SoundSource::CloseFile()
{
    file.Close();
    m_0174 = 0;
}

// Closes the source: for an Ogg source frees its reader; otherwise frees the decoded data; then closes the file.
// Returns the reader's result for Ogg, 1 when a file was closed, else 0. Called when playback stops or a voice is
// released.
int SoundSource::CloseSource()
{
    if (isFileSource != 0)
    {
        if (file.handle == 0)
            return 0;
    }
    else
    {
        if (sourceKind == SOUND_SOURCE_OGG)
        {
            int result = oggReader->Free();
            if (oggReader != 0)
                free(oggReader);
            oggReader = 0;
            return result;
        }
        if (file.handle == 0)
            return 0;
        if (pcmBuffer != 0)
            free(pcmBuffer);
        pcmBuffer = 0;
    }
    CloseFile();
    return 1;
}

// Moves the read position to `position` bytes into the sound data (for a file source also seeks the file).
// Called when a sound starts and when it loops.
void SoundSource::SeekTo(unsigned position)
{
    if (isFileSource != 0)
    {
        if (position < WAVE_HEADER_BYTES)
        {
            file.Seek(position, 0);
            readPosition = position;
            return;
        }
        file.Seek(dataOffset + position, 0);
        readPosition = dataOffset + position;
        return;
    }
    if (sourceKind == SOUND_SOURCE_OGG)
    {
        OggReader *reader = oggReader;
        reader->totalLength = position;
        reader->consumed = 0;
        readPosition = position;
    }
    if (sourceKind == SOUND_SOURCE_ACM)
        readPosition = position;
    decodedPosition = position;
    readPosition = position;
    m_0188 = 1;
    m_0184 = 0;
}

// Returns how many bytes of the sound have been read so far (relative to the start of the data). Called by the
// stream update and the position queries.
int SoundSource::GetPosition()
{
    if (isFileSource != 0)
    {
        if (file.Tell() - dataOffset < 0)
            return 0;
        return file.Tell() - dataOffset;
    }
    if (sourceKind == SOUND_SOURCE_OGG)
        return oggReader->totalLength - oggReader->consumed;
    return decodedPosition;
}

// True when the whole sound has been read. Called by the stream update to detect the end of a sound.
bool SoundSource::IsAtEnd()
{
    if (isFileSource != 0)
        return dataSize <= (unsigned)GetPosition();
    return decodedPosition >= dataSize;
}

// Stops the buffer and closes the source. Returns 1, or 0 when there is nothing to stop. Called by the stop commands.
int SoundVoice::Stop()
{
    if (directSound != 0 && primaryBuffer != 0 && buffer != 0 && IsPlaying() != 0)
    {
        buffer->Stop();
        if (playing != 0)
            CloseSource();
        playing = 0;
        m_01c8 = 0;
        return 1;
    }
    return 0;
}

// Hook called after an effect's volume was set; the original did nothing. Returns 1. Called by the effect volume
// command.
int SoundVoice::AfterVolumeSet()
{
    return 1;
}

// Returns the play position in samples: how far the source was read, corrected by how much of the three-second
// DirectSound buffer is still queued ahead of the play cursor. Returns 0 when the voice has no buffer or decoder.
// Called by the music code through the stream position query.
int SoundVoice::GetPlayPosition()
{
    if (directSound == 0 || primaryBuffer == 0 || buffer == 0 || decoderOpen == 0)
        return 0;
    unsigned block = bufferFormat.wf.nAvgBytesPerSec;
    unsigned cursor = playCursor;
    int queued;
    if (cursor < block && thirdFlag[0] == 0)
        queued = cursor - block * 2;
    if (cursor < block && thirdFlag[0] == 1)
        queued = cursor - block * 3;
    if (cursor >= block && cursor < block * 2 && thirdFlag[1] == 0)
        queued = cursor - block * 3;
    if (cursor >= block && cursor < block * 2 && thirdFlag[1] == 1)
        queued = cursor - block * 4;
    if (cursor >= block * 2 && cursor < block * 3 && thirdFlag[2] == 0)
        queued = cursor - block * 4;
    if (cursor >= block * 2 && cursor < block * 3 && thirdFlag[2] == 1)
        queued = cursor - block * 5;
    unsigned position = readPosition;
    if (position < block * 2)
        position += dataEnd;
    return ((position + queued) / bufferFormat.wf.nChannels << 3) / waveHeader.bitsPerSample;
}

// Starts a sound at the master music volume at once (no fade): silences the buffer for the start, plays, then sets the
// volume to the master target. Returns Play's result. Called by the "play on stream" command.
int SoundVoice::PlayAtOnce(const char *path, int looping, int startSample, int loopStartSample, int loopEndSample)
{
    fadeTarget = volume;
    fadeStopAtEnd = 0;
    fadeRemainingMs = 0;
    volume = SOUND_VOLUME_FULL;
    if (IsPlaying() != 0 && directSound != 0 && primaryBuffer != 0 && buffer != 0)
    {
        if (IsPlaying() != 0)
            buffer->SetVolume(SOUND_DS_VOLUME_SILENT);
    }
    int result = Play(path, looping, startSample, loopStartSample, loopEndSample);
    int target = g_soundMusicVolumeTarget;
    fadeStopAtEnd = 0;
    fadeTarget = target;
    fadeRemainingMs = 0;
    fadeLastTick = GetTickCount();
    volume = target;
    if (IsPlaying() != 0)
        ApplyVolume(target, 0);
    return result;
}

// Starts a sound with the default one second fade in to the master music volume. Returns Play's result.
int SoundVoice::PlayFadeInDefault(const char *path, int looping, int startSample, int loopStartSample,
                                  int loopEndSample)
{
    fadeTarget = volume;
    fadeStopAtEnd = 0;
    fadeRemainingMs = 0;
    volume = SOUND_VOLUME_FULL;
    if (IsPlaying() != 0 && directSound != 0 && primaryBuffer != 0 && buffer != 0)
    {
        if (IsPlaying() != 0)
            buffer->SetVolume(SOUND_DS_VOLUME_SILENT);
    }
    int result = Play(path, looping, startSample, loopStartSample, loopEndSample);
    int target = g_soundMusicVolumeTarget;
    fadeStopAtEnd = 0;
    fadeTarget = target;
    fadeRemainingMs = SOUND_FADE_DEFAULT_MS;
    fadeLastTick = GetTickCount();
    return result;
}

// Starts a sound with a fade in of `fadeMs` milliseconds to the master music volume (0 = at once). Returns Play's
// result.
int SoundVoice::PlayFadeIn(const char *path, int looping, int startSample, int loopStartSample, int loopEndSample,
                           int fadeMs)
{
    fadeTarget = volume;
    fadeStopAtEnd = 0;
    fadeRemainingMs = 0;
    volume = SOUND_VOLUME_FULL;
    if (IsPlaying() != 0 && directSound != 0 && primaryBuffer != 0 && buffer != 0)
    {
        if (IsPlaying() != 0)
            buffer->SetVolume(SOUND_DS_VOLUME_SILENT);
    }
    int result = Play(path, looping, startSample, loopStartSample, loopEndSample);
    int target = g_soundMusicVolumeTarget;
    fadeStopAtEnd = 0;
    fadeTarget = target;
    fadeRemainingMs = fadeMs;
    fadeLastTick = GetTickCount();
    if (fadeMs == 0)
    {
        volume = target;
        if (IsPlaying() != 0)
            ApplyVolume(target, 0);
    }
    return result;
}

// Stops the voice, frees the decoder and releases the listener, the buffers and (when this voice created it) the
// DirectSound device, logging the device's remaining reference count. Called for every voice when the engine shuts
// down and before a voice is set up again. Returns 0.
int SoundVoice::Release()
{
    if (listener != 0)
        listener->Release();
    listener = 0;
    Stop();
    if (decoderOpen != 0)
        FreeDecoder();
    if (usesSharedPrimary != 0)
        primaryBuffer = 0;
    if (buffer != 0)
        buffer->Release();
    buffer = 0;
    if (buffer3d != 0)
        buffer3d->Release();
    buffer3d = 0;
    if (primaryBuffer != 0)
        primaryBuffer->Stop();
    if (primaryBuffer != 0)
        primaryBuffer->Release();
    primaryBuffer = 0;
    if (usesSharedDevice != 0)
        directSound = 0;
    if (directSound != 0)
    {
        char message[256];
        sprintf(message, "%d", directSound->Release());
        OutputDebugStringA(message);
    }
    directSound = 0;
    return 0;
}

// Distance from the listener at which a 3D sound starts to fade, and where it is inaudible.
static const float SOUND_3D_MIN_DISTANCE = 50.0f;
static const float SOUND_3D_MAX_DISTANCE = 800.0f;
// Cone angle in degrees: a full sphere, so the sound is not directional.
static const DWORD SOUND_3D_CONE_ANGLE = 360;

// Sets the position of the voice (and its velocity when given, else zero). With a DirectSound 3D buffer the new
// parameters are pushed to it, scaling the distance to the listener by `distanceScale`; without one, a positioned
// voice updates its pan and volume itself. Returns 1. Called by the effect position command.
int SoundVoice::SetPosition(const float *x, const float *y, const float *z, const float *velocityX,
                            const float *velocityY, const float *velocityZ)
{
    if (buffer3d == 0)
    {
        if (is3D != 0)
        {
            position[0] = *x;
            position[1] = *y;
            position[2] = *z;
            UpdatePanFromListener();
        }
        return 1;
    }
    DS3DBUFFER parameters;
    parameters.dwSize = sizeof(DS3DBUFFER);
    buffer3d->GetAllParameters(&parameters);
    parameters.dwMode = DS3DMODE_NORMAL;
    parameters.flMinDistance = SOUND_3D_MIN_DISTANCE;
    parameters.flMaxDistance = SOUND_3D_MAX_DISTANCE;
    parameters.dwInsideConeAngle = SOUND_3D_CONE_ANGLE;
    parameters.dwOutsideConeAngle = SOUND_3D_CONE_ANGLE;
    if (velocityX == 0)
    {
        velocity[0] = 0;
        velocity[1] = 0;
        velocity[2] = 0;
    }
    else
    {
        velocity[0] = *velocityX;
        velocity[1] = *velocityY;
        velocity[2] = *velocityZ;
    }
    parameters.vVelocity.x = velocity[0];
    position[0] = *x;
    parameters.vVelocity.y = velocity[1];
    position[1] = *y;
    parameters.vVelocity.z = velocity[2];
    position[2] = *z;
    parameters.vPosition.x = (position[0] - g_soundListenerPosition[0]) * distanceScale + g_soundListenerPosition[0];
    parameters.vPosition.y = (position[1] - g_soundListenerPosition[1]) * distanceScale + g_soundListenerPosition[1];
    parameters.vPosition.z = (position[2] - g_soundListenerPosition[2]) * distanceScale + g_soundListenerPosition[2];
    buffer3d->SetAllParameters(&parameters, DS3D_IMMEDIATE);
    return 1;
}

// Pushes the stored position and velocity to the DirectSound 3D buffer (scaled around the listener), then refreshes
// the software pan. Returns 0 without a buffer, else 1. Called when the listener moves.
int SoundVoice::UpdatePosition()
{
    if (buffer == 0)
        return 0;
    if (buffer3d != 0)
    {
        DS3DBUFFER parameters;
        parameters.dwSize = sizeof(DS3DBUFFER);
        buffer3d->GetAllParameters(&parameters);
        parameters.vVelocity.x = velocity[0];
        parameters.vPosition.x = (position[0] - g_soundListenerPosition[0]) * distanceScale + g_soundListenerPosition[0];
        parameters.vVelocity.y = velocity[1];
        parameters.vPosition.y = (position[1] - g_soundListenerPosition[1]) * distanceScale + g_soundListenerPosition[1];
        parameters.vVelocity.z = velocity[2];
        parameters.vPosition.z = (position[2] - g_soundListenerPosition[2]) * distanceScale + g_soundListenerPosition[2];
        parameters.dwMode = DS3DMODE_NORMAL;
        parameters.flMinDistance = SOUND_3D_MIN_DISTANCE;
        parameters.flMaxDistance = SOUND_3D_MAX_DISTANCE;
        parameters.dwInsideConeAngle = SOUND_3D_CONE_ANGLE;
        parameters.dwOutsideConeAngle = SOUND_3D_CONE_ANGLE;
        buffer3d->SetAllParameters(&parameters, DS3D_IMMEDIATE);
    }
    UpdatePanFromListener();
    return 1;
}

// Destructor: frees the decoder, then the decoded data, and closes the file (the file member closes itself). Called
// when a voice is destroyed.
SoundSource::~SoundSource()
{
    FreeDecoder();
    if (pcmBuffer != 0)
        free(pcmBuffer);
    pcmBuffer = 0;
}

// Output format of the primary buffer: 16-bit stereo at 44.1 kHz.
const int PRIMARY_CHANNELS = 2;
const int PRIMARY_SAMPLE_RATE = 44100;
const int PRIMARY_BITS = 16;
// Rolloff factor of the listener a new device starts with.
static const float LISTENER_START_ROLLOFF = 0.2f;

// Starts the voice's DirectSound objects: releases what it held, then creates the device (or adopts the shared
// `sharedDevice`) and the primary buffer (or adopts `sharedPrimary`). A device it creates gets the exclusive cooperative
// level; a primary buffer it creates gets 3D control, the 44.1 kHz 16-bit stereo format, a 3D listener with default
// orientation and starts looping. Returns 1, or 0 when a DirectSound call fails. Called for the engine's own device
// voice with nothing shared and for every other voice with the device voice's objects.
int SoundVoice::InitDevice(HWND window, IDirectSound8 *sharedDevice, IDirectSoundBuffer *sharedPrimary)
{
    Release();
    if (sharedDevice == 0)
    {
        if (DirectSoundCreate8(0, &directSound, 0) != 0)
            return 0;
        if (directSound->SetCooperativeLevel(window, DSSCL_EXCLUSIVE) != 0)
        {
            directSound->Release();
            return 0;
        }
    }
    else
    {
        directSound = sharedDevice;
        usesSharedDevice = 1;
    }
    if (sharedPrimary == 0)
    {
        DSBUFFERDESC description;
        memset(&description, 0, sizeof(description));
        description.dwSize = sizeof(DSBUFFERDESC);
        description.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
        if (directSound->CreateSoundBuffer(&description, &primaryBuffer, 0) != 0)
        {
            directSound->Release();
            return 0;
        }
        WAVEFORMATEX format;
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = PRIMARY_CHANNELS;
        format.wBitsPerSample = PRIMARY_BITS;
        format.nSamplesPerSec = PRIMARY_SAMPLE_RATE;
        format.nAvgBytesPerSec = PRIMARY_SAMPLE_RATE * PRIMARY_CHANNELS * PRIMARY_BITS / 8;
        format.nBlockAlign = PRIMARY_CHANNELS * PRIMARY_BITS / 8;
        format.cbSize = 0;
        primaryBuffer->SetFormat(&format);
        primaryBuffer->GetFormat(&format, sizeof(WAVEFORMATEX), 0);
        primaryBuffer->QueryInterface(IID_IDirectSound3DListener, (void **)&listener);
        if (listener != 0)
        {
            DS3DLISTENER parameters;
            parameters.dwSize = sizeof(DS3DLISTENER);
            listener->GetAllParameters(&parameters);
            parameters.flRolloffFactor = LISTENER_START_ROLLOFF;
            parameters.vPosition.x = 0;
            parameters.vPosition.y = 0;
            parameters.vPosition.z = 0;
            parameters.vVelocity.x = 0;
            parameters.vVelocity.y = 0;
            parameters.vVelocity.z = 0;
            parameters.vOrientFront.x = 0;
            parameters.vOrientFront.y = LISTENER_DEFAULT_AXIS;
            parameters.vOrientFront.z = 0;
            parameters.vOrientTop.x = 0;
            parameters.vOrientTop.y = 0;
            parameters.vOrientTop.z = 1.0f;
            parameters.flDopplerFactor = LISTENER_DOPPLER_FACTOR;
            listener->SetAllParameters(&parameters, DS3D_IMMEDIATE);
        }
        primaryBuffer->Restore();
        primaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    }
    else
    {
        primaryBuffer = sharedPrimary;
        usesSharedPrimary = 1;
    }
    return 1;
}

// Mode string the sound files are opened with.
static const char SOUND_OPEN_READ_BINARY[] = "rb";
// Factor of the decoded size: decoded bytes = compressed bytes * this / (compressedParam + 8).
const int SOUND_COMPRESSED_SIZE_SCALE = 0x13e20;
// Added to the compressed parameter before it divides the size.
const int SOUND_COMPRESSED_PARAM_BIAS = 8;

// Opens a sound file in the compressed layout (a wave header, a 32-bit parameter, then compressed data) and works out
// the decoded size from the file length and the parameter. Returns 1, or 0 when a file is already open or cannot be
// opened. Called by the opener for files whose name ends in 'c'.
int SoundSource::OpenCompressed(const char *path)
{
    m_0174 = 0;
    if (file.handle != 0)
        return 0;
    if (file.Open(path, SOUND_OPEN_READ_BINARY) == 0)
        return 0;
    decodedPosition = 0;
    m_019c = 0;
    file.Seek(WAVE_HEADER_BYTES, SEEK_SET);
    file.Read((char *)&compressedParam, 1, ARCHIVE_FIELD_BYTES);
    file.Seek(0, SEEK_END);
    int length = file.Tell();
    int ratio = compressedParam + SOUND_COMPRESSED_PARAM_BIAS;
    unsigned __int64 scaled = (unsigned __int64)(unsigned)(length - WAVE_HEADER_BYTES) * SOUND_COMPRESSED_SIZE_SCALE;
    dataSize = (int)((__int64)scaled / ratio) + WAVE_HEADER_BYTES;
    m_0174 = 1;
    isCompressed = 1;
    return 1;
}
