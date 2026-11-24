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
        vorbis_dsp_clear(&dsp);
        vorbis_block_clear(&block);
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
