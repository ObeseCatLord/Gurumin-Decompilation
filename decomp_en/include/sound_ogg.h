// Ogg Vorbis reader of the sound engine (EN build; JP: sound3d.dll). Declared once here. The Xiph libogg/libvorbis
// code (0x7c6220-0x7e1a3b) is statically linked and not ported; its states are declared as opaque blocks of the
// size the reader gives them, and its entry points as extern "C" functions.
#ifndef GURUMIN_EN_SOUND_OGG_H
#define GURUMIN_EN_SOUND_OGG_H

// Opaque libogg / libvorbis state blocks (sizes from the OggReader layout).
struct OggSyncState { char bytes[0x1c]; };      // ogg_sync_state
struct OggPage { char bytes[0x14]; };           // ogg_page
struct OggStreamState { char bytes[0x168]; };   // ogg_stream_state
struct OggPacket { char bytes[0x20]; };         // ogg_packet
struct VorbisBlock { char bytes[0x70]; };       // vorbis_block
struct VorbisDspState { char bytes[0x70]; };    // vorbis_dsp_state
struct VorbisInfo { char bytes[0x20]; };        // vorbis_info
struct VorbisComment { char bytes[0x10]; };     // vorbis_comment

// Library entry points (statically linked, not ported).
extern "C" void ogg_stream_clear(OggStreamState *state);        // EN 0x7cd640
extern "C" void ogg_sync_clear(OggSyncState *state);            // EN 0x7ce230
extern "C" char *ogg_sync_buffer(OggSyncState *state, long size);   // EN 0x7ce180
extern "C" int ogg_sync_wrote(OggSyncState *state, long bytes);     // EN 0x7ce5e0
extern "C" void vorbis_dsp_clear(VorbisDspState *state);        // EN 0x7c80b0
extern "C" void vorbis_block_clear(VorbisBlock *block);         // EN 0x7c81c0
extern "C" void vorbis_comment_clear(VorbisComment *comment);   // EN 0x7cf3c0
extern "C" void vorbis_info_clear(VorbisInfo *info);            // EN 0x7cf790

// Bytes the reader hands to libogg at a time.
const int OGG_FEED_BYTES = 0x1000;
// 16-bit values in the reader's output block (all channels).
const int OGG_PCM_OUT_SHORTS = 0x1000;

// Reads and decodes an Ogg Vorbis file held in memory (0x2748 bytes).
struct OggReader
{
    char m_0000[4];                 // 0x0000: not used by ported code yet
    void *m_0004;                   // 0x0004: cleared by the constructor
    char m_0008[4];                 // 0x0008: not used by ported code yet
    void *m_000c;                   // 0x000c: cleared by the constructor
    char m_0010[4];                 // 0x0010: not used by ported code yet
    void *pcmBuffer;                // 0x0014: decoded sound data
    char m_0018[0x48 - 0x18];       // 0x0018: not used by ported code yet
    VorbisBlock block;              // 0x0048
    VorbisDspState dsp;             // 0x00b8
    OggSyncState sync;              // 0x0128
    OggPage page;                   // 0x0144
    OggStreamState stream;          // 0x0158
    OggPacket packet;               // 0x02c0
    VorbisInfo info;                // 0x02e0
    VorbisComment comment;          // 0x0300
    short pcmOut[OGG_PCM_OUT_SHORTS];  // 0x0310: interleaved decoded samples
    int m_2310;                     // 0x2310: not used by ported code yet
    char m_2314;                    // 0x2314: not used by ported code yet
    char initialised;               // 0x2315: non-zero once the Vorbis states were set up
    char m_2316[0x2724 - 0x2316];   // 0x2316: not used by ported code yet
    float gain;                     // 0x2724: output gain
    int totalLength;                // 0x2728: length of the sound
    char m_272c[0x2734 - 0x272c];   // 0x272c: not used by ported code yet
    int consumed;                   // 0x2734: amount already played
    char m_2738[4];                 // 0x2738: not used by ported code yet
    char *fileData;                  // 0x273c: the Ogg file in memory
    const char *readCursor;         // 0x2740: next byte to hand to libogg
    int fileSize;                   // 0x2744: size of the Ogg file

    // Constructor: clears the buffers and the initialised flag. EN 0x7be8b0.
    OggReader();
    // Frees the buffers and the Vorbis states. EN 0x7bea30.
    int Free();
    // Clears the stream and sync states when initialised. EN 0x7c1190.
    int ClearStreams();
    // Clears every Vorbis state when initialised. EN 0x7c11c0.
    int Close();
    // Sets the length of the sound and restarts the played count. EN 0x7c1440.
    void SetLength(int length);
    // Hands the next chunk of the file to libogg. Returns 0 at the end of the file. EN 0x7c1520.
    int FeedSync();
};

#endif
