// Ogg Vorbis reader of the sound engine (EN build; JP twin: sound3d.dll, not matched yet).
#include <stdlib.h>
#include <string.h>

#include "sound_ogg.h"

// Constructor: clears the buffer pointers, the file data pointer and the initialised flag. Called when a voice is
// set up for an Ogg source.
OggReader::OggReader()
{
    m_0004 = 0;
    m_000c = 0;
    pcmBuffer = 0;
    fileData = 0;
    initialised = 0;
}

// Frees the decoded and file buffers and, when the Vorbis states were set up, clears them. Returns 1. Called when
// the voice closes an Ogg source.
int OggReader::Free()
{
    if (pcmBuffer != 0)
        free(pcmBuffer);
    pcmBuffer = 0;
    if (fileData != 0)
        free(fileData);
    fileData = 0;
    if (initialised != 0)
    {
        ClearStreams();
        Close();
    }
    return 1;
}

// Clears the Ogg stream and sync states when the reader is initialised. Returns 1.
int OggReader::ClearStreams()
{
    if (initialised != 0)
    {
        ogg_stream_clear(&stream);
        ogg_sync_clear(&sync);
    }
    return 1;
}

// Clears every Vorbis state (stream, sync, DSP, block, comment, info) when initialised, then drops the flag. Returns 1.
int OggReader::Close()
{
    if (initialised != 0)
    {
        ogg_stream_clear(&stream);
        ogg_sync_clear(&sync);
        vorbis_block_clear(&block);
        vorbis_dsp_clear(&dsp);
        vorbis_comment_clear(&comment);
        vorbis_info_clear(&info);
    }
    initialised = 0;
    return 1;
}

// Sets the length of the sound and restarts the played count. Called when a voice seeks an Ogg source.
void OggReader::SetLength(int length)
{
    totalLength = length;
    consumed = 0;
}

// Hands the next chunk (up to OGG_FEED_BYTES) of the file in memory to libogg's sync buffer. Returns 1, or 0 when
// the whole file was handed over. Called while the voice decodes.
int OggReader::FeedSync()
{
    if ((readCursor - fileData) + OGG_FEED_BYTES * 2 >= fileSize)
    {
        int remaining = fileSize - (int)(readCursor - fileData);
        if (remaining <= 0)
            return 0;
        memcpy(ogg_sync_buffer(&sync, remaining), readCursor, remaining);
        readCursor += remaining;
        ogg_sync_wrote(&sync, remaining);
        return 1;
    }
    memcpy(ogg_sync_buffer(&sync, OGG_FEED_BYTES), readCursor, OGG_FEED_BYTES);
    readCursor += OGG_FEED_BYTES;
    ogg_sync_wrote(&sync, OGG_FEED_BYTES);
    return 1;
}

// Decodes the pages of the file into `output`: pulls pages, packets and PCM out of libogg/libvorbis, scales the float
// samples to clamped 16-bit values times the gain, interleaves the channels and copies each chunk (while it fits below
// OGG_OUTPUT_LIMIT). Returns the bytes written, or -1 on a stream error. Called when a voice refills its buffer.
int OggReader::Decode(char *output)
{
    int written = 0;
    for (;;)
    {
        int result = ogg_sync_pageout(&sync, &page);
        if (result == 0)
            return written;
        if (result < 0)
            return -1;
        ogg_stream_pagein(&stream, &page);
        result = ogg_stream_packetout(&stream, &packet);
        while (result != 0)
        {
            if (result < 0)
                return -1;
            if (vorbis_synthesis(&block, &packet) == 0)
                vorbis_synthesis_blockin(&dsp, &block);
            float **pcm;
            int frames = vorbis_synthesis_pcmout(&dsp, &pcm);
            while (frames > 0)
            {
                int count = frames < chunkFrames ? frames : chunkFrames;
                for (int channel = 0; channel < info.channels; channel++)
                {
                    float *samples = pcm[channel];
                    short *destination = pcmOut + channel;
                    for (int i = 0; i < count; i++)
                    {
                        int value = (int)(samples[i] * OGG_SAMPLE_SCALE);
                        if (value > OGG_SAMPLE_MAX)
                            value = OGG_SAMPLE_MAX;
                        else if (value < OGG_SAMPLE_MIN)
                            value = OGG_SAMPLE_MIN;
                        *destination = (short)(int)((float)value * gain);
                        destination += info.channels;
                    }
                }
                int bytes = info.channels * count * OGG_SAMPLE_BYTES;
                if (bytes + written < OGG_OUTPUT_LIMIT)
                    memcpy(output + written, pcmOut, bytes);
                written += info.channels * count * OGG_SAMPLE_BYTES;
                vorbis_synthesis_read(&dsp, count);
                frames = vorbis_synthesis_pcmout(&dsp, &pcm);
            }
            result = ogg_stream_packetout(&stream, &packet);
        }
        if (ogg_page_eos(&page) != 0)
            break;
    }
    return written;
}

// Rewinds to the start of the file, hands the first block to libogg, starts the logical stream and reads the three
// Vorbis header packets (identification, comment, setup); then sets how many sample frames fit one decoded chunk.
// Returns 1 on success, 0 when the data is not a Vorbis stream. Called when an Ogg source is opened or restarted.
int OggReader::ReadHeaders()
{
    readCursor = fileData;
    ogg_sync_reset(&sync);
    char *feed = ogg_sync_buffer(&sync, OGG_FEED_BYTES);
    memcpy(feed, readCursor, OGG_FEED_BYTES);
    readCursor += OGG_FEED_BYTES;
    if (ogg_sync_wrote(&sync, OGG_FEED_BYTES) != 0)
        return 0;
    if (ogg_sync_pageout(&sync, &page) != 1)
        return 0;
    if (ogg_stream_init(&stream, ogg_page_serialno(&page)) != 0)
        return 0;
    vorbis_comment_init(&comment);
    if (ogg_stream_pagein(&stream, &page) < 0)
        return 0;
    if (ogg_stream_packetout(&stream, &packet) != 1)
        return 0;
    if (vorbis_synthesis_headerin(&info, &comment, &packet) < 0)
        return 0;
    int headerPackets = 0;
    for (;;)
    {
        int result;
        for (;;)
        {
            result = ogg_sync_pageout(&sync, &page);
            if (result == 0)
                continue;
            if (result == 1)
                break;
        }
        ogg_stream_pagein(&stream, &page);
        while ((result = ogg_stream_packetout(&stream, &packet)) != 0)
        {
            if (result < 0)
                return 0;
            vorbis_synthesis_headerin(&info, &comment, &packet);
            headerPackets++;
            if (headerPackets < OGG_HEADER_PACKETS)
                continue;
            chunkFrames = OGG_FEED_BYTES / info.channels;
            return 1;
        }
    }
}

// Opens the Vorbis stream of a freshly loaded file: like ReadHeaders, but first initialises the info and comment, and
// after the headers sets up the DSP state and block and marks the reader initialised. Returns 1 on success, 0 when the
// data is not a Vorbis stream. Called once when an Ogg source is opened.
int OggReader::OpenStream()
{
    readCursor = fileData;
    ogg_sync_reset(&sync);
    char *feed = ogg_sync_buffer(&sync, OGG_FEED_BYTES);
    memcpy(feed, readCursor, OGG_FEED_BYTES);
    readCursor += OGG_FEED_BYTES;
    if (ogg_sync_wrote(&sync, OGG_FEED_BYTES) != 0)
        return 0;
    if (ogg_sync_pageout(&sync, &page) != 1)
        return 0;
    if (ogg_stream_init(&stream, ogg_page_serialno(&page)) != 0)
        return 0;
    vorbis_info_init(&info);
    vorbis_comment_init(&comment);
    if (ogg_stream_pagein(&stream, &page) < 0)
        return 0;
    if (ogg_stream_packetout(&stream, &packet) != 1)
        return 0;
    if (vorbis_synthesis_headerin(&info, &comment, &packet) < 0)
        return 0;
    int headerPackets = 0;
    for (;;)
    {
        int result;
        for (;;)
        {
            result = ogg_sync_pageout(&sync, &page);
            if (result == 0)
                continue;
            if (result == 1)
                break;
        }
        ogg_stream_pagein(&stream, &page);
        while ((result = ogg_stream_packetout(&stream, &packet)) != 0)
        {
            if (result < 0)
                return 0;
            vorbis_synthesis_headerin(&info, &comment, &packet);
            headerPackets++;
            if (headerPackets < OGG_HEADER_PACKETS)
                continue;
            chunkFrames = OGG_FEED_BYTES / info.channels;
            vorbis_synthesis_init(&dsp, &info);
            vorbis_block_init(&dsp, &block);
            initialised = 1;
            return 1;
        }
    }
}

// Copies `bytes` of the sound into `buffer`: first the 44-byte WAVE header (when the stream position is still inside
// it), then decoded PCM chunks (decoding the next chunk whenever the current one is used up). A position that moved
// since the last call restarts the stream and skips ahead to it. Returns `bytes`, or 0 at the end of the file.
// Called when a voice refills its buffer from an Ogg source.
int OggReader::ReadPcm(char *buffer, int bytes)
{
    int remaining = bytes;
    if (totalLength < OGG_WAVE_HEADER_BYTES)
    {
        memcpy(buffer, &waveHeader, sizeof(waveHeader));
        totalLength = OGG_WAVE_HEADER_BYTES;
        savedLength = OGG_WAVE_HEADER_BYTES;
        chunkUsed = 0;
        return OGG_WAVE_HEADER_BYTES;
    }
    if (totalLength == OGG_WAVE_HEADER_BYTES || savedLength != totalLength)
    {
        if (initialised != 0)
        {
            ogg_stream_clear(&stream);
            ogg_sync_clear(&sync);
        }
        ReadHeaders();
        int skipped = OGG_WAVE_HEADER_BYTES;
        int decoded = 0;
        if (totalLength > OGG_WAVE_HEADER_BYTES)
        {
            do
            {
                if (FeedSync() == 0)
                    return 0;
                decoded = Decode((char *)pcmBuffer);
                skipped += decoded;
                chunkBytes = decoded;
            } while (totalLength > skipped);
        }
        consumed = 0;
        chunkUsed = decoded - skipped + totalLength;
        savedLength = totalLength;
    }
    for (;;)
    {
        if (remaining < 1)
            return bytes;
        int step;
        if (chunkUsed == 0)
        {
            if (FeedSync() == 0)
                return 0;
            int decoded = Decode((char *)pcmBuffer);
            chunkBytes = decoded;
            char *destination = buffer + (bytes - remaining);
            if (remaining < decoded)
            {
                memcpy(destination, pcmBuffer, remaining);
                chunkUsed = remaining;
                totalLength += remaining;
                savedLength = totalLength;
                consumed = 0;
                return bytes;
            }
            memcpy(destination, pcmBuffer, decoded);
            step = -decoded;
            totalLength += decoded;
        }
        else
        {
            int available = chunkBytes - chunkUsed;
            char *destination = buffer + (bytes - remaining);
            if (remaining < available)
            {
                memcpy(destination, (char *)pcmBuffer + chunkUsed, remaining);
                consumed = 0;
                chunkUsed += remaining;
                totalLength += remaining;
                savedLength = totalLength;
                return bytes;
            }
            memcpy(destination, (char *)pcmBuffer + chunkUsed, available);
            step = chunkUsed - chunkBytes;
            totalLength += chunkBytes - chunkUsed;
        }
        consumed = 0;
        chunkUsed = 0;
        savedLength = totalLength;
        remaining += step;
    }
}
