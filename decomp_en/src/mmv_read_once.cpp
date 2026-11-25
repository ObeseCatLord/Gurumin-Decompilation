// One-shot movie playback (EN build).
#include "engine_memory.h"
#include "mmv_player.h"

// Decodes the next frame of a movie that plays once, from memory or streamed from the file. A frame record is its size
// followed by the frame data. When the last frame was read the movie is finished: state becomes MMV_FINISHED (when the
// caller keeps the movie) or the player is released. Does nothing for a short movie type after MMV_SHORT_FRAMES
// frames. Called once per frame by DecodeNextFrame. EN 0x7a9610 (JP 0x10160f00).
void MmvPlayer::ReadFrameOnce()
{
    if (playing == 0)
        return;
    if (type == MMV_TYPE_SHORT && frameIndex >= MMV_SHORT_FRAMES)
        return;
    if (isOpen == 0)
    {
        if (state == MMV_FINISHED)
            return;
        int recordSize = *(int *)(memory + start + offset);
        data = (unsigned char *)(memory + start + offset + 4);
        DecodeFrameRle(recordSize);
        frameIndex++;
        offset += recordSize + 4;
        if (start + offset < size)
            return;
        g_engineFree(memory);
        memory = 0;
    }
    else
    {
        if (state == MMV_FINISHED)
            return;
        int recordSize;
        file.Read(&recordSize, 1, 4);
        if ((unsigned int)(recordSize - 1) <= MMV_STREAM_MAX - 1)
        {
            file.Read(g_movieStreamBuffer, 1, recordSize);
            data = g_movieStreamBuffer;
            DecodeFrameRle(recordSize);
            offset += recordSize + 4;
            frameIndex++;
        }
        else
            offset += size;
        if (offset < size)
            return;
        file.Close();
    }
    if (state == MMV_STOPPED)
    {
        playing = 0;
        if (frame != 0 && ownsFrame != 0)
            g_engineFree(frame);
        frame = 0;
    }
    else
        state = MMV_FINISHED;
}
