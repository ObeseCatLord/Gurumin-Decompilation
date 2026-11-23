// MmvPlayer: the MMV movie player (0x25c bytes). The game has one, DAT_10f08ab0. Methods: mmv_texreg.cpp.
#ifndef GURUMIN_MMV_PLAYER_H
#define GURUMIN_MMV_PLAYER_H

#include "mmv_texreg.h"

struct TextureObject;   // texture_object.h

// Values of MmvPlayer::state.
#define MMV_STOPPED 0
#define MMV_PLAYING 1
#define MMV_HOLD_LAST_FRAME 2   // reached the end: keeps showing the last frame

struct MmvPlayer
{
    int frameIndex;        // 0x00: number of the next frame
    int start;             // 0x04: offset of the first frame record in `memory`
    int offset;            // 0x08: offset of the next frame record after the header
    int size;              // 0x0c: movie size
    int loop;              // 0x10: non-zero = the movie loops (ReadFrame), 0 = it plays once (ReadFrameOnce)
    char isOpen;           // 0x14: 1 = frames are streamed from `file`
    char pad_0015[3];
    MovieFile file;
    void *m_003c;
    short width;
    short height;
    char pad_0044[4];
    short m_0048;          // 0x48: movie type (type 6 shows only its first 7 frames)
    char m_004a;
    char m_004b;
    char state;            // 0x4c: MMV_STOPPED, MMV_PLAYING or MMV_HOLD_LAST_FRAME
    char pad_004d;
    unsigned short palette[0x101]; // 0x4e: the movie's 255 colours as RGB565, entry 255 = 0xfbff
    char *memory;          // 0x250: whole movie in memory (when not streamed from the file)
    void *frame;           // 0x254: decoded 8-bit frame (palette indexes), updated in place
    unsigned char *data;   // 0x258: compressed data of the current frame

    MmvPlayer();
    ~MmvPlayer();
    void Close();                                       // 0x101609b0
    int DrawToTexture(const TextureObject *target);     // 0x10160960: copies the current frame into `target`
    void ReadFrame();
    void ReadFrameOnce();
    void M_10161050();                                  // decodes the next frame
    void DecodeFrameRle(int count);
    void Open(const char *path);                        // 0x10160a90
};

extern "C" MmvPlayer DAT_10f08ab0;   // the movie player

#endif
