// Playback of the sound engine's voices: buffer refill and the per-step update (EN build; JP twin: sound3d.dll, not
// matched yet).
#include <string.h>

#include "sound_engine.h"
#include "sound_voice.h"

// First data byte after the wave header: loops seek back here when no loop start is set.
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
        if (position + firstBytes > dataEnd)
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
        else
            ReadData(first, firstBytes);
        if (second != 0)
        {
            position = GetPosition();
            if (position + secondBytes > dataEnd)
            {
                int remaining = dataEnd - position;
                if (remaining != 0)
                    ReadData(second, remaining);
                memset((char *)second + (dataEnd - position), 0, position - dataEnd + secondBytes);
            }
            else
                ReadData(second, secondBytes);
        }
    }
    else
    {
        int position = GetPosition();
        void *target = first;
        unsigned count = firstBytes;
        if (position + firstBytes > dataEnd)
        {
            ReadData(first, dataEnd - position);
            if (loopStartByte == SOUND_NO_LOOP_START)
                SeekTo(WAVE_HEADER_BYTES);
            else
                SeekTo(loopStartByte + WAVE_HEADER_BYTES);
            count = position - dataEnd + firstBytes;
            target = (char *)first + (dataEnd - position);
        }
        ReadData(target, count);
        if (second != 0)
        {
            position = GetPosition();
            target = second;
            count = secondBytes;
            if (position + secondBytes > dataEnd)
            {
                ReadData(second, dataEnd - position);
                SeekTo(WAVE_HEADER_BYTES);
                count = position - dataEnd + secondBytes;
                target = (char *)second + (dataEnd - position);
            }
            ReadData(target, count);
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
