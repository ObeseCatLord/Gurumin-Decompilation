// MmvPlayer and MovieFile of the EN build (JP: decomp/include/mmv_player.h, mmv_texreg.h). Only the members EN code
// has used so far are named. This is the one declaration of these types in decomp_en.
#ifndef GURUMIN_EN_MMV_PLAYER_H
#define GURUMIN_EN_MMV_PLAYER_H

#include <stddef.h>

struct TextureObject;

// MmvPlayer::state value of a movie that is loaded but not playing, and of one that played its last frame.
const int MMV_STOPPED = 0;
const int MMV_FINISHED = 2;
// Movie type that is cut short, and how many frames it plays.
const int MMV_TYPE_SHORT = 6;
const int MMV_SHORT_FRAMES = 7;
// Largest frame record that can be streamed from a file, and the buffer it is read into (EN 0x02201100).
const unsigned int MMV_STREAM_MAX = 0x25800;

// A file the movie is streamed from.
struct MovieFile
{
    void *handle;     // 0x00: file handle
    char name[0x20];  // 0x04: file name

    void Close();     // EN 0x7a8aa0 (JP 0x101b2330)
    size_t Read(void *buffer, size_t size, size_t count);   // EN 0x7a8b20: fread from the open file
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
    short width;           // 0x40: movie width in pixels
    short height;          // 0x42: movie height in pixels
    char m_0044[0x48 - 0x44];  // 0x44: not yet named
    short type;            // 0x48: movie type (type 6 plays only its first MMV_TYPE6_FRAMES frames)
    char ownsFrame;        // 0x4a: 1 when `frame` was allocated by the player
    char playing;          // 0x4b: 1 while the player holds a movie
    char state;            // 0x4c: playback state (MMV_STOPPED after Open)
    char pad_004d;         // 0x4d: padding
    unsigned short palette[0x100];        // 0x4e: first palette (RGB565)
    unsigned short secondPalette[0x100];  // 0x24e: second palette (RGB565)
    char m_044e[0x450 - 0x44e];           // 0x44e: not yet named
    char *memory;          // 0x450: whole movie in memory (when not streamed from the file)
    void *frame;           // 0x454: decoded 8-bit frame (palette indexes)
    unsigned char *data;   // 0x458: start of the frame records inside `memory`

    void ReadFrame();                    // EN 0x7a9750: next frame of a looping movie
    void ReadFrameOnce();                // EN 0x7a9610: next frame of a movie that plays once
    void DecodeFrameRle(int firstRecord);   // EN 0x7a8f20 (JP 0x10160a20): decodes the first frame record
    void Open(const char *path);         // EN 0x7a8f90 (JP 0x10160a90): opens the movie file
    // EN 0x7a8ec0 (JP 0x10160960): copies the current frame into `target` using the first palette, or the second one
    // when `secondPalette` is non-zero.
    int DrawToTexture(const TextureObject *target, int secondPalette);
    void DecodeNextFrame();              // EN 0x7a9850 (JP 0x10161050)
    void Close();                        // EN 0x7a9860 (JP 0x101609b0)
};

extern "C" MmvPlayer g_moviePlayer;       // EN 0x00da7480 (JP DAT_10f08ab0): the movie player of the scene movies
extern "C" unsigned char g_movieStreamBuffer[MMV_STREAM_MAX];   // EN 0x02201100: one frame record streamed from the movie file
extern "C" int g_movieTextureSlot;        // EN 0x00908130 (JP DAT_10202478): texture slot the scene movie is drawn into
extern "C" int g_movieFrameCount;         // EN 0x0091fe18 (JP DAT_104bfe74): frame count of the scene movie

// Opens movie `name`, decodes its first frame into the movie texture slot and records the frame count. Returns 1. EN 0x603a10.
extern "C" int Movie_PlayToTexture(const char *name, int secondPalette);

const int MMV_PALETTE_COLOURS = 0xff;             // palette entries stored in the file (the 256th is the transparent colour)

#pragma pack(push, 1)
// The start of an MMV file: the size, the palette, the size of the first frame record and then the frame records.
struct MmvPaletteEntry
{
    unsigned char blue;     // 0x00: blue component
    unsigned char green;    // 0x01: green component
    unsigned char red;      // 0x02: red component
    unsigned char pad;      // 0x03: unused
};

struct MmvFileHeader
{
    char m_0000[2];                                    // 0x00: not yet named
    unsigned short width;                              // 0x02: movie width in pixels
    unsigned short height;                             // 0x04: movie height in pixels
    MmvPaletteEntry palette[MMV_PALETTE_COLOURS];      // 0x06: palette of the movie
    char m_0402[4];                                    // 0x402: not yet named
    int firstFrameSize;                                // 0x406: size of the first frame record
    unsigned char frames[1];                           // 0x40a: the frame records
};
#pragma pack(pop)

#endif
