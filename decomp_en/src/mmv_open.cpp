// Opening of a movie of the MMV player (EN build). Ported from the matched JP source decomp/src/mmv_texreg.cpp.
#include <stdio.h>

#include "engine_memory.h"
#include "game_file.h"
#include "mmv_player.h"

const unsigned short MMV_TRANSPARENT_COLOUR = 0xfbff;   // palette entry 255 of both palettes
const int MMV_FRAMES_OFFSET = 0x40a;              // offset of the first frame record in the file

// Loads movie `path` (looked for as .\bin\<base name>.MMV first) whole into memory: the size, the 255-colour palette
// (as RGB565 for the first palette and a 4-bit-per-channel form for the second, colour 255 set to the transparent
// colour), a frame buffer of width x height and the first frame decoded. When the frame buffer cannot be allocated the
// movie is dropped again. Called by Movie_PlayToTexture.
void MmvPlayer::Open(const char *path)
{
    char base[0x100];
    char fileName[0x100];
    loop = 0;
    state = MMV_STOPPED;
    isOpen = 0;
    GameFile movie;
    Path_GetBaseName(base, path);
    sprintf(fileName, ".\\bin\\%s.MMV", base);
    FILE *opened = movie.Open(fileName, "rb");
    if (opened == 0)
    {
        opened = movie.Open(path, "rb");
        if (opened == 0)
            return;
    }
    movie.Seek(0, SEEK_END);
    int length = movie.Size();
    movie.Seek(0, SEEK_SET);
    if (memory != 0)
        g_engineFree(memory);
    memory = (char *)g_engineAlloc(length, "MMV_ALL");
    if (memory == 0)
        movie.Close();
    else
    {
        movie.Read(memory, 1, length);
        movie.Close();
        const MmvFileHeader *header = (const MmvFileHeader *)memory;
        width = header->width;
        height = header->height;
        int i;
        for (i = 0; i < MMV_PALETTE_COLOURS; i++)
        {
            const MmvPaletteEntry &entry = ((const MmvFileHeader *)memory)->palette[i];
            palette[i] = (((unsigned short)entry.red & ~7) << 5 | ((unsigned short)entry.green & ~3)) << 3 |
                         (unsigned short)(entry.blue >> 3);
            const MmvPaletteEntry &second = ((const MmvFileHeader *)memory)->palette[i];
            secondPalette[i] = (((unsigned short)second.red & ~15) << 5 | ((unsigned short)second.green & ~7)) << 2 |
                               (unsigned short)(second.blue >> 4);
        }
        palette[i] = MMV_TRANSPARENT_COLOUR;
        secondPalette[i] = MMV_TRANSPARENT_COLOUR;
        if (frame != 0 && ownsFrame != 0)
            g_engineFree(frame);
        frame = g_engineAlloc(width * height, "MMV_DIB");
        if (frame == 0)
        {
            if (memory != 0)
                g_engineFree(memory);
            memory = 0;
        }
        else
        {
            ownsFrame = 1;
            int first = ((const MmvFileHeader *)memory)->firstFrameSize;
            data = ((MmvFileHeader *)memory)->frames;
            DecodeFrameRle(first);
            start = first + MMV_FRAMES_OFFSET;
            offset = 0;
            size = length;
            frameIndex = 0;
            playing = 1;
        }
    }
}
