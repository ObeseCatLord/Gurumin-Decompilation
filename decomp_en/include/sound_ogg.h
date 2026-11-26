// Ogg Vorbis reader of the sound engine (EN build; JP: sound3d.dll). Declared once here. The Xiph libogg/libvorbis
// code (0x7c6220-0x7e1a3b) is statically linked and not ported; its states are declared as opaque blocks of the
// size the reader gives them, and its entry points as extern "C" functions.
#ifndef GURUMIN_EN_SOUND_OGG_H
#define GURUMIN_EN_SOUND_OGG_H

#include "sound_file.h"

// Opaque libogg / libvorbis state blocks (sizes from the OggReader layout).
// The WAVE header of the decoded sound (RIFF, fmt and data chunk headers).
struct OggWaveHeader
{
    unsigned riffId;            // 0x00: "RIFF"
    unsigned riffSize;          // 0x04: size of the decoded data
    unsigned waveId;            // 0x08: "WAVE"
    unsigned fmtId;             // 0x0c: "fmt "
    unsigned fmtSize;           // 0x10: size of the format chunk
    unsigned short formatTag;   // 0x14: format tag
    unsigned short channels;    // 0x16: channels
    unsigned sampleRate;        // 0x18: samples per second
    unsigned bytesPerSecond;    // 0x1c
    unsigned short blockAlign;  // 0x20
    unsigned short bitsPerSample;   // 0x22
    unsigned dataId;            // 0x24: "data" (not filled in by the reader)
    unsigned dataSize;          // 0x28: size of the sound data
};
struct OggSyncState { char bytes[0x1c]; };      // ogg_sync_state
struct OggPage { char bytes[0x14]; };           // ogg_page
struct OggStreamState { char bytes[0x168]; };   // ogg_stream_state
struct OggPacket { char bytes[0x20]; };         // ogg_packet
struct VorbisBlock { char bytes[0x70]; };       // vorbis_block
struct VorbisDspState { char bytes[0x70]; };    // vorbis_dsp_state
// vorbis_info: stream parameters.
struct VorbisInfo
{
    int version;            // 0x00: bitstream version
    int channels;           // 0x04: number of channels
    long rate;              // 0x08: sample rate
    long bitrateUpper;      // 0x0c: upper bitrate bound
    long bitrateNominal;    // 0x10: nominal bitrate
    long bitrateLower;      // 0x14: lower bitrate bound
    long bitrateWindow;     // 0x18: window size of the bitrate
    void *codecSetup;       // 0x1c: codec set up data
};
struct VorbisComment { char bytes[0x10]; };     // vorbis_comment

// Library entry points (statically linked, not ported).
extern "C" void ogg_stream_clear(OggStreamState *state);        // EN 0x7cd640
extern "C" void ogg_sync_clear(OggSyncState *state);            // EN 0x7ce230
extern "C" char *ogg_sync_buffer(OggSyncState *state, long size);   // EN 0x7ce180
extern "C" int ogg_sync_wrote(OggSyncState *state, long bytes);     // EN 0x7ce5e0
extern "C" void vorbis_dsp_clear(VorbisDspState *state);        // EN 0x7c81c0
extern "C" void vorbis_block_clear(VorbisBlock *block);         // EN 0x7c80b0
extern "C" void vorbis_comment_clear(VorbisComment *comment);   // EN 0x7cf3c0
extern "C" void vorbis_info_clear(VorbisInfo *info);            // EN 0x7cf790
extern "C" int ogg_sync_reset(OggSyncState *sync);                      // EN 0x7ce2b0
extern "C" int ogg_page_serialno(OggPage *page);                        // EN 0x7cd5e0
extern "C" int ogg_stream_init(OggStreamState *stream, int serial);     // EN 0x7cd9f0
extern "C" void vorbis_comment_init(VorbisComment *comment);            // EN 0x7cf430
extern "C" int vorbis_synthesis_headerin(VorbisInfo *info, VorbisComment *comment, OggPacket *packet);  // EN 0x7cf940
extern "C" void vorbis_info_init(VorbisInfo *info);                     // EN 0x7cf910
extern "C" int vorbis_synthesis_init(VorbisDspState *dsp, VorbisInfo *info);    // EN 0x7c8a90
extern "C" int vorbis_block_init(VorbisDspState *dsp, VorbisBlock *block);      // EN 0x7c8130
extern "C" int ogg_sync_pageout(OggSyncState *sync, OggPage *page);     // EN 0x7ce2e0
extern "C" int ogg_stream_pagein(OggStreamState *stream, OggPage *page);    // EN 0x7cdd40
extern "C" int ogg_stream_packetout(OggStreamState *stream, OggPacket *packet);     // EN 0x7cdce0
extern "C" int ogg_page_eos(OggPage *page);                             // EN 0x7cd4d0
extern "C" int vorbis_synthesis(VorbisBlock *block, OggPacket *packet);     // EN 0x7dc870
extern "C" int vorbis_synthesis_blockin(VorbisDspState *dsp, VorbisBlock *block);   // EN 0x7c8400
extern "C" int vorbis_synthesis_pcmout(VorbisDspState *dsp, float ***pcm);  // EN 0x7c8e10
extern "C" int vorbis_synthesis_read(VorbisDspState *dsp, int samples);     // EN 0x7c8e70

// Output block size above which the decoder stops copying samples.
const int OGG_OUTPUT_LIMIT = 240000;
// Largest 16-bit sample value, and its negative bound.
const int OGG_SAMPLE_MAX = 0x7fff;
const int OGG_SAMPLE_MIN = -0x8000;
// Float to 16-bit scale.
const float OGG_SAMPLE_SCALE = 32767.0f;
// Bytes of one 16-bit sample.
const int OGG_SAMPLE_BYTES = 2;
// Size of the WAVE header in front of the decoded data, and the number of Vorbis header packets after the first.
const int OGG_WAVE_HEADER_BYTES = 0x2c;
const int OGG_HEADER_PACKETS = 2;
// Bytes the reader hands to libogg at a time.
const int OGG_FEED_BYTES = 0x1000;
// 16-bit values in the reader's output block (all channels).
const int OGG_PCM_OUT_SHORTS = 0x1000;

// Bytes of the path copy kept in the reader, and of the decoded chunk buffer allocated for it.
const int OGG_PATH_BYTES = 0x400;
const int OGG_PCM_BUFFER_BYTES = 0x3a980;
// Fields of the WAVE header the reader writes: "RIFF", "WAVE" and "fmt " as little-endian words, the size of the
// format chunk, the two format words (tag 0 and 2 channels, block align 4 and 16 bits), the CD sample rate and its
// byte rate (44100 * 4).
const unsigned OGG_RIFF_ID = 0x46464952;
const unsigned OGG_WAVE_ID = 0x45564157;
const unsigned OGG_FMT_ID = 0x20746d66;
const unsigned OGG_FMT_SIZE = 0x10;
const unsigned short OGG_FORMAT_TAG = 0;
const unsigned short OGG_CHANNELS = 2;
const unsigned OGG_SAMPLE_RATE = 44100;
const unsigned OGG_BYTES_PER_SECOND = 0x2b110;
const unsigned short OGG_BLOCK_ALIGN = 4;
const unsigned short OGG_BITS_PER_SAMPLE = 16;
// Extra seconds added to the length estimated from the bitrate, and the factor between the data size and the size
// the header declares.
const int OGG_LENGTH_SLACK_SECONDS = 3;

// Reads and decodes an Ogg Vorbis file held in memory (0x2748 bytes).
struct OggReader
{
    int totalBytes;                 // 0x0000: size of the decoded sound with its WAVE header
    MmioFile file;                  // 0x0004: the Ogg file while it is read (closed afterwards)
    void *pcmBuffer;                // 0x0014: decoded sound data
    OggWaveHeader waveHeader;       // 0x0018: the 44-byte WAVE header handed out before the sound data
    char m_0044[0x48 - 0x44];       // 0x0044: not used by ported code yet
    VorbisDspState dsp;             // 0x0048
    VorbisBlock block;              // 0x00b8
    OggSyncState sync;              // 0x0128
    OggPage page;                   // 0x0144
    OggStreamState stream;          // 0x0158
    OggPacket packet;               // 0x02c0
    VorbisInfo info;                // 0x02e0
    VorbisComment comment;          // 0x0300
    short pcmOut[OGG_PCM_OUT_SHORTS];  // 0x0310: interleaved decoded samples
    int chunkFrames;                // 0x2310: most sample frames converted at a time
    char m_2314;                    // 0x2314: not used by ported code yet
    char initialised;               // 0x2315: non-zero once the Vorbis states were set up
    char m_2316[0x2324 - 0x2316];   // 0x2316: not used by ported code yet
    char path[OGG_PATH_BYTES];      // 0x2324: copy of the path the reader was opened with
    float gain;                     // 0x2724: output gain
    int totalLength;                // 0x2728: length of the sound
    int savedLength;                // 0x272c: copy of totalLength taken when the stream position is known
    int chunkUsed;                  // 0x2730: bytes of the current decoded chunk already handed out
    int consumed;                   // 0x2734: amount already played
    int chunkBytes;                 // 0x2738: bytes in the current decoded chunk
    char *fileData;                  // 0x273c: the Ogg file in memory
    const char *readCursor;         // 0x2740: next byte to hand to libogg
    int fileSize;                   // 0x2744: size of the Ogg file

    // Constructor: clears the buffers and the initialised flag. EN 0x7be8b0.
    OggReader();
    // Opens the Ogg file `path` (a plain file or a section of an archive), loads it into memory, reads its headers
    // and fills in the WAVE header from the file size and bitrate. Returns 1, or 0 when the file cannot be opened.
    // EN 0x7c0200.
    int Open(const char *path);
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
    // Decodes pages into `output` (16-bit interleaved, at most OGG_OUTPUT_LIMIT bytes). Returns the bytes written, or -1
    // on a stream error. EN 0x7bed30.
    int Decode(char *output);
    // Rewinds to the start of the file and reads the three Vorbis header packets, then sets the chunk size. Returns 1
    // on success, 0 when the file is not a Vorbis stream. EN 0x7bf740.
    int ReadHeaders();
    // Like ReadHeaders on a fresh reader: also initialises the info, the DSP state and the block, and marks the
    // reader initialised. Returns 1 on success, else 0. EN 0x7bf8c0.
    int OpenStream();
    // Copies `bytes` of the sound (the WAVE header first, then decoded PCM) into `buffer`. Returns `bytes`, or 0 at the
    // end of the file. EN 0x7c0f50.
    int ReadPcm(char *buffer, int bytes);
};

#endif
