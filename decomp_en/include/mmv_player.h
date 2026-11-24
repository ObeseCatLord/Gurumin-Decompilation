// MmvPlayer and MovieFile of the EN build (JP: decomp/include/mmv_player.h, mmv_texreg.h). Only the members EN code
// has used so far are named. This is the one declaration of these types in decomp_en.
#ifndef GURUMIN_EN_MMV_PLAYER_H
#define GURUMIN_EN_MMV_PLAYER_H

// A file the movie is streamed from.
struct MovieFile
{
    void *handle;     // 0x00: file handle
    char name[0x20];  // 0x04: file name

    void Close();     // EN 0x7a8aa0 (JP 0x101b2330)
};

struct MmvPlayer
{
    int frameIndex;        // 0x00: number of the next frame
    int start;             // 0x04: offset of the first frame record in `memory`
    int offset;            // 0x08: offset of the next frame record after the header
    int size;              // 0x0c: movie size
    int loop;              // 0x10: non-zero = the movie loops (ReadFrame), 0 = it plays once (ReadFrameOnce)
    char isOpen;           // 0x14: 1 = frames are streamed from `file`
    char pad_0015[3];
    MovieFile file;        // 0x18: the movie file
    void *m_003c;          // 0x3c: not yet named (non-zero while the file is open)
    char m_0040[0x4a - 0x40];  // 0x40: width, height, movie type
    char ownsFrame;        // 0x4a: 1 when `frame` was allocated by the player
    char playing;          // 0x4b: 1 while the player holds a movie
    char m_004c[0x450 - 0x4c]; // 0x4c: state and palette
    char *memory;          // 0x450: whole movie in memory (when not streamed from the file)
    void *frame;           // 0x454: decoded 8-bit frame (palette indexes)

    void ReadFrame();                    // EN 0x7a9750: next frame of a looping movie
    void ReadFrameOnce();                // EN 0x7a9610: next frame of a movie that plays once
    void DecodeNextFrame();              // EN 0x7a9850 (JP 0x10161050)
    void Close();                        // EN 0x7a9860 (JP 0x101609b0)
};

#endif
