// Packs image/archive codec of the EN build (JP: Packs.dll, linked into the exe in EN). Declarations used by
// src/packs_image.cpp and src/packs_lz.cpp. This is the one declaration of these globals and constants.
#ifndef GURUMIN_EN_PACKS_CODEC_H
#define GURUMIN_EN_PACKS_CODEC_H

// Largest magnitude of a stored signed sample, the shift that places it in the top 12 bits of a 16-bit slot and the
// sign bit of such a slot.
const int PACKS_SAMPLE_MAX = 0x7ff;
const int PACKS_SAMPLE_SHIFT = 4;
const int PACKS_SAMPLE_SIGN_BIT = 0x8000;

// Index of the next slot written by Packs_StoreSignedSample. EN 0x0222b180.
extern "C" int g_packsOutputIndex;

// Stores one signed sample (see packs_image.cpp). EN 0x7bb500.
extern "C" void Packs_StoreSignedSample(unsigned short *output, unsigned short base, short delta);
// Converts three sample planes to interleaved bytes (see packs_image.cpp). EN 0x7ba070.
extern "C" void Packs_PlanesToComponents(short *planeA, short *planeB, short *planeC, int *width, int *height,
                                         unsigned char *output, int *outputLength);

// ---- LZ decompressor (src/packs_lz.cpp) ----

// Largest unpacked size Packs_Unpack accepts.
const unsigned int PACKS_MAX_UNPACKED_SIZE = 0x3ee100;
// Result of Packs_UnpackBlocks when a block cannot be decoded.
const int PACKS_BLOCK_ERROR = 0x29a;
// Bytes of an unpacked block that are copied out per progress step, and the fields of the progress ratio.
const int PACKS_PROGRESS_STEP_BYTES = 0x319c;
const int PACKS_PROGRESS_STEP_UNIT = 0x64000;
const int PACKS_PROGRESS_WEIGHT_UNIT = 0x1000;
const int PACKS_PROGRESS_SIZE_UNIT = 0x7f;

// Header of a packed stream.
struct PacksPackedHeader
{
    unsigned int unpackedSize;   // 0x00: size of the data once unpacked (0 or above PACKS_MAX_UNPACKED_SIZE = invalid)
    int checksumSeed;            // 0x04: value used by the progress ratio
    unsigned char blocks[1];     // 0x08: the packed blocks
};

// Read state of the block decoder.
struct PacksUnpackStream
{
    unsigned char *window;        // 0x00: next byte of the block window to write
    unsigned char *input;         // 0x04: next byte of the packed data to read
    int decodedLength;            // 0x08: bytes of the current block decoded so far
    unsigned int inputLength;     // 0x0c: length of the packed data
    unsigned int blockLength;     // 0x10: length field of the current block
    int m_0014;                   // 0x14: not yet named (the local copy in Packs_Unpack is 0x18 bytes)
};

// Progress callback of the unpacker: (context, bytes decoded so far, permille-style ratio); returns non-zero to abort.
typedef int (*PacksProgressCallback)(int context, int decodedBytes, unsigned int ratio);

// Window the blocks are decoded into. EN 0x0242b410.
extern "C" unsigned char g_packsWindow[];
// Unpacker input length and the stream being decoded. EN 0x0222b1ec, 0x0222b1fc.
extern "C" unsigned int g_packsUnpackedSize;       // EN 0x0222b1ec
extern "C" unsigned char *g_packsWindowStart;      // EN 0x0222b1f0: start of the window (copies are made from it)
extern "C" unsigned char *g_packsOutput;           // EN 0x0222b1f4: next byte of the output buffer
extern "C" int g_packsAbort;                       // EN 0x0222b1f8: non-zero once the progress callback asked to stop
extern "C" PacksUnpackStream *g_packsStream;       // EN 0x0222b1fc
extern "C" unsigned int g_packsChecksumSeed;                // EN 0x0222b208
extern "C" int g_packsProgressContext;             // EN 0x0222b1b8
extern "C" unsigned int g_packsProgressStep;                // EN 0x0222b1dc
extern "C" unsigned int g_packsProgressWeight;              // EN 0x0222b1e0
extern "C" PacksProgressCallback g_packsProgress;  // EN 0x025ab510

// Decodes one block into the window; returns 0 on bad data. EN 0x7bd990.
extern "C" int Packs_DecodeBlock();
// Decodes every block of the stream, copying them to the output buffer. Returns 1 when all were decoded, 0 when the
// progress callback aborted or the last block ended the stream, PACKS_BLOCK_ERROR on bad data. EN 0x7bd870.
extern "C" int Packs_UnpackBlocks(PacksUnpackStream *stream);
// Unpacks a packed stream into `output`. Returns the unpacked size, or 0 when the header or data is invalid. EN 0x7bcf70.
extern "C" unsigned int Packs_Unpack(const PacksPackedHeader *packed, unsigned char *output, int packedLength);

// ---- LZ compressor hash chains (src/packs_lz.cpp) ----

// Positions are kept in a window of this many entries (links are indexed by position & mask); 0xffff = no entry.
const unsigned int PACKS_LZ_WINDOW_MASK = 0x7fff;
const unsigned int PACKS_LZ_REACH = 0x1fff;
const unsigned short PACKS_LZ_NO_ENTRY = 0xffff;

// Hash chains over the bytes being compressed.
struct PacksLzState
{
    int m_0000;                       // 0x00: not yet named
    unsigned char *data;              // 0x04: next byte to index (the hash is the 16-bit pair at this address)
    unsigned short *links;            // 0x08: next position in the chain, indexed by position & PACKS_LZ_WINDOW_MASK
    unsigned short *chainTail;        // 0x0c: last position of each chain, indexed by hash
    unsigned short *chainHead;        // 0x10: first position of each chain, indexed by hash
};

extern "C" int g_packsLzPosition;           // EN 0x0222b1c0: position of the next byte to index
extern "C" PacksLzState *g_packsLzState;    // EN 0x0222b1c4
extern "C" int g_packsLzPending;            // EN 0x0222b1d0: bytes still to index

// Indexes g_packsLzPending bytes into the hash chains, dropping the entry that fell out of reach. EN 0x7bd790.
extern "C" void Packs_LzIndexBytes();

#endif
