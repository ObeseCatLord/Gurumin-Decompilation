// Playback of the sound engine's voices: buffer refill and the per-step update (EN build; JP twin: sound3d.dll, not
// matched yet).
#include <string.h>

#include "sound_engine.h"
#include "sound_voice.h"

// loopStartByte value of a voice that has no loop start (a loop then restarts at the first data byte).
const unsigned SOUND_NO_LOOP_START = 0xffffffff;

// Fills `length` bytes of the DirectSound buffer at `offset` with the next sound data. A one-shot sound that ends
// inside the block is padded with silence; when nothing is left the buffer is unlocked, the source closed and the
// voice stopped. A looping sound seeks back to its loop start (or the first data byte) and continues. Returns 0 when
// the lock fails, else 1. Called by the update step when the play cursor enters a new third of the buffer.
int SoundVoice::FillBuffer(unsigned length, int reserved, unsigned offset)
{
    buffer->Restore();
    void *first;
    DWORD firstBytes;
    void *second;
    DWORD secondBytes;
    if (buffer->Lock(offset, length, &first, &firstBytes, &second, &secondBytes, 0) != 0)
        return 0;
    if (looping == 0)
    {
        int position = GetPosition();
        if (position + firstBytes <= dataEnd)
            ReadData(first, firstBytes);
        else
        {
            int remaining = dataEnd - position;
            if (remaining != 0)
                ReadData(first, remaining);
            remaining = dataEnd - position;
            if (remaining <= 0)
            {
                buffer->Unlock(first, firstBytes, second, secondBytes);
                CloseSource();
                buffer->Stop();
                playing = 0;
                return 1;
            }
            memset((char *)first + remaining, 0, position - dataEnd + firstBytes);
        }
        if (second != 0)
        {
            position = GetPosition();
            if (position + secondBytes <= dataEnd)
                ReadData(second, secondBytes);
            else
            {
                int remaining = dataEnd - position;
                if (remaining != 0)
                    ReadData(second, remaining);
                memset((char *)second + (dataEnd - position), 0, position - dataEnd + secondBytes);
            }
        }
    }
    else
    {
        int position = GetPosition();
        if (position + firstBytes <= dataEnd)
            ReadData(first, firstBytes);
        else
        {
            ReadData(first, dataEnd - position);
            if (loopStartByte != SOUND_NO_LOOP_START)
                SeekTo(loopStartByte + WAVE_HEADER_BYTES);
            else
                SeekTo(WAVE_HEADER_BYTES);
            ReadData((char *)first + (dataEnd - position), position - dataEnd + firstBytes);
        }
        if (second != 0)
        {
            position = GetPosition();
            if (position + secondBytes <= dataEnd)
                ReadData(second, secondBytes);
            else
            {
                ReadData(second, dataEnd - position);
                SeekTo(WAVE_HEADER_BYTES);
                ReadData((char *)second + (dataEnd - position), position - dataEnd + secondBytes);
            }
        }
    }
    buffer->Unlock(first, firstBytes, second, secondBytes);
    return 1;
}

// One update step of a voice, run every timer tick: advances a running fade (stopping the voice when a fade out
// ends), then compares the play cursor with the three one-second thirds of the buffer and refills the third that
// was just played. Returns 0 when the voice has no buffer or is not playing, else 1. Called by the engine's update
// step for every music stream.
int SoundVoice::UpdateBuffer()
{
    if (directSound == 0)
        return 0;
    if (primaryBuffer == 0 || buffer == 0 || playing == 0)
        return 0;
    if (fadeRemainingMs != 0)
    {
        DWORD now = GetTickCount();
        int elapsed = now - fadeLastTick;
        int remaining = fadeRemainingMs - elapsed;
        fadeLastTick = now;
        if (remaining <= 0)
        {
            fadeRemainingMs = 0;
            if (fadeStopAtEnd != 0)
            {
                Stop();
                return 1;
            }
            SetVolume(fadeTarget);
        }
        else
        {
            int level = ((fadeTarget - volume) * elapsed) / fadeRemainingMs + volume;
            fadeRemainingMs = remaining;
            SetVolume(level);
        }
    }
    DWORD playCursorNow;
    DWORD writeCursor;
    buffer->GetCurrentPosition(&playCursorNow, &writeCursor);
    unsigned third = bufferFormat.wf.nAvgBytesPerSec;
    playCursor = playCursorNow;
    if (playCursorNow < third && thirdFlag[0] == 0)
    {
        FillBuffer(third, 0, third * 2);
        thirdFlag[0] = 1;
        thirdFlag[1] = 0;
        thirdFlag[2] = 1;
    }
    third = bufferFormat.wf.nAvgBytesPerSec;
    if (playCursorNow >= third && playCursorNow < third * 2 && thirdFlag[1] == 0)
    {
        FillBuffer(third, 0, 0);
        thirdFlag[0] = 1;
        thirdFlag[1] = 1;
        thirdFlag[2] = 0;
    }
    third = bufferFormat.wf.nAvgBytesPerSec;
    if (playCursorNow >= third * 2 && playCursorNow < third * 3 && thirdFlag[2] == 0)
    {
        FillBuffer(third, 0, third);
        thirdFlag[0] = 0;
        thirdFlag[1] = 1;
        thirdFlag[2] = 1;
    }
    return 1;
}

// DirectSound buffer flags: position tracking that follows the real play cursor, volume and pan control; the
// placement bits follow the engine's buffer mode (2 = hardware, 1 = software, 0 = driver's choice).
const DWORD SOUND_BUFFER_FLAGS_BASE = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;
const int SOUND_BUFFER_MODE_HARDWARE = 2;
const int SOUND_BUFFER_MODE_SOFTWARE = 1;
// Seconds of sound the buffer holds (it is refilled a third at a time).
const int SOUND_BUFFER_SECONDS = 3;
// Bits per byte.
const int SOUND_BITS_PER_BYTE = 8;

// Starts playing the sound file `path`: stops what the voice played, opens the file and reads its wave header,
// creates the DirectSound buffer for the format (three seconds long), converts the loop and start positions from
// samples to bytes, fills the first two thirds of the buffer and starts it looping. Nothing plays when
// `loopEndSample` is 0. Returns 1 when playing, else 0. Called by the play commands (at once or with a fade).
int SoundVoice::Play(const char *path, int looping, int startSample, int loopStartSample, unsigned loopEndSample)
{
    if (directSound != 0 && primaryBuffer != 0)
    {
        if (IsPlaying() != 0)
            Stop();
        if (loopEndSample > 0)
        {
            if (decoderOpen == 0)
                InitDecoder(0);
            if (OpenSource(path) != 0)
            {
                if (ReadData(&waveHeader, WAVE_HEADER_BYTES) != 0)
                {
                    DSBUFFERDESC description;
                    memset(&bufferFormat, 0, sizeof(bufferFormat));
                    bufferFormat.wf.nSamplesPerSec = waveHeader.sampleRate;
                    bufferFormat.wf.wFormatTag = WAVE_FORMAT_PCM;
                    bufferFormat.wf.nChannels = (waveHeader.channels > 1) + 1;
                    bufferFormat.wf.nBlockAlign = waveHeader.blockAlign;
                    bufferFormat.wf.nAvgBytesPerSec = waveHeader.blockAlign * waveHeader.sampleRate;
                    bufferFormat.wBitsPerSample = waveHeader.bitsPerSample;
                    memset(&description, 0, sizeof(description));
                    description.dwSize = sizeof(DSBUFFERDESC);
                    description.dwBufferBytes = bufferFormat.wf.nAvgBytesPerSec;
                    description.lpwfxFormat = (LPWAVEFORMATEX)&bufferFormat;
                    if (g_soundBufferMode == SOUND_BUFFER_MODE_HARDWARE)
                        description.dwFlags = SOUND_BUFFER_FLAGS_BASE | DSBCAPS_LOCHARDWARE;
                    else
                        description.dwFlags = SOUND_BUFFER_FLAGS_BASE |
                                              (g_soundBufferMode == SOUND_BUFFER_MODE_SOFTWARE) * DSBCAPS_LOCSOFTWARE;
                    description.dwBufferBytes = description.dwBufferBytes * SOUND_BUFFER_SECONDS;
                    if (buffer == 0)
                    {
                        IDirectSoundBuffer *temporary;
                        directSound->CreateSoundBuffer(&description, &temporary, 0);
                        if (temporary == 0)
                            return 0;
                        temporary->QueryInterface(IID_IDirectSoundBuffer8, (void **)&buffer);
                        temporary->Release();
                        if (buffer == 0)
                            return 0;
                    }
                    unsigned bits = bufferFormat.wBitsPerSample;
                    unsigned channels = bufferFormat.wf.nChannels;
                    loopStartByte = (bits * loopStartSample / SOUND_BITS_PER_BYTE) * channels;
                    unsigned endByte = (bits * loopEndSample / SOUND_BITS_PER_BYTE) * channels;
                    if (dataSize < endByte)
                        endByte = dataSize;
                    loopEndByte = endByte;
                    dataEnd = endByte + WAVE_HEADER_BYTES;
                    if (dataEnd == WAVE_HEADER_BYTES)
                        *(int *)0 = 1;
                    SeekTo((bits * startSample / SOUND_BITS_PER_BYTE) * channels + WAVE_HEADER_BYTES);
                    this->looping = looping;
                    buffer->SetCurrentPosition(0);
                    FillBuffer(bufferFormat.wf.nAvgBytesPerSec * 2, 0, 0);
                    playStartTick = GetTickCount();
                    thirdFlag[0] = 0;
                    thirdFlag[1] = 1;
                    thirdFlag[2] = 1;
                    playing = 1;
                    buffer->Restore();
                    primaryBuffer->Restore();
                    buffer->Play(0, 0, DSBPLAY_LOOPING);
                    return 1;
                }
                CloseSource();
            }
        }
    }
    return 0;
}
