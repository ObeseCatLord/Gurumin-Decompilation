// Packs image/archive codec of the EN build (JP: Packs.dll, linked into the exe in EN). Declarations used by
// src/packs_image.cpp and src/packs_lz.cpp. This is the one declaration of these globals and constants.
#ifndef GURUMIN_EN_PACKS_CODEC_H
#define GURUMIN_EN_PACKS_CODEC_H

#include "game_file.h"

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

// ---- Image plane levels (src/packs_levels.cpp) ----

// Rebuilds one resolution level of a decoded sample plane: `width` x `height` is the size of the level, `fullWidth` the
// width of the full plane. Not ported yet. EN 0x7b9ab0 (1184 bytes, called by the image decoder at EN 0x7b8f30).
extern "C" void Packs_ReconstructLevel(short *plane, short *scratch, int width, int height, int fullWidth);
// Rebuilds the `levelCount` levels of a plane from the last (smallest) to the first, given the level sizes in `widths` and
// `heights`. EN 0x7b9f60.
extern "C" void Packs_ReconstructLevels(short *plane, short *scratch, int *widths, int *heights, int levelCount);
// Rebuilds all levels of the three planes of an image. EN 0x7b9fb0.
extern "C" void Packs_ReconstructPlanes(short *planeA, short *planeB, short *planeC, short *scratch, int *widths,
                                        int *heights, int levelCount);
// Splits one level of a sample plane into its coarser level and detail (the encoder's counterpart of
// Packs_ReconstructLevel). Not ported yet. EN 0x7bc4d0 (2028 bytes, called by the image encoder at EN 0x7b8af0).
extern "C" void Packs_DecomposeLevel(short *plane, short *scratch, int width, int height, int fullWidth);
// Splits the `levelCount` levels of a plane from the first to the last. EN 0x7bc480.
extern "C" void Packs_DecomposeLevels(short *plane, short *scratch, int *widths, int *heights, int levelCount);

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
// Longest run of equal bytes the compressor encodes in one token.
const unsigned int PACKS_LZ_RUN_MAX = 0xfff;

// Hash chains over the bytes being compressed.
struct PacksLzState
{
    int m_0000;                       // 0x00: not yet named
    unsigned char *data;              // 0x04: next byte to index (the hash is the 16-bit pair at this address)
    unsigned short *links;            // 0x08: next position in the chain, indexed by position & PACKS_LZ_WINDOW_MASK
    unsigned short *chainTail;        // 0x0c: last position of each chain, indexed by hash
    unsigned short *chainHead;        // 0x10: first position of each chain, indexed by hash
    int m_0014[2];                    // 0x14: not yet named
    int length;                       // 0x1c: total number of bytes being compressed
};

extern "C" int g_packsLzPosition;           // EN 0x0222b1c0: position of the next byte to index
extern "C" PacksLzState *g_packsLzState;    // EN 0x0222b1c4
extern "C" int g_packsLzPending;            // EN 0x0222b1d0: bytes still to index

// Longest run of equal bytes at PacksLzState::data, at most PACKS_LZ_RUN_MAX and at most the bytes left (see
// packs_lz_run.cpp). Returns the run length. EN 0x7bccc0. Called by the compressor at EN 0x7bd240.
extern "C" unsigned int Packs_LzCountRun(void);

// Indexes g_packsLzPending bytes into the hash chains, dropping the entry that fell out of reach. EN 0x7bd790.
extern "C" void Packs_LzIndexBytes();

// ---- Image container loader (src/packs_bmp.cpp) ----

// File mode, magic numbers (colour image only / colour image followed by an alpha image) and bytes per pixel of the
// Packs image container.
#define PACKS_OPEN_MODE "rb"
const int PACKS_MAGIC_COLOUR = 0x378;
const int PACKS_MAGIC_COLOUR_ALPHA = 0x379;
const int PACKS_IMAGE_BYTES_PER_PIXEL = 3;

// Decodes a JPEG stream from an open file into an RGB buffer and stores its size. EN 0x7be1c0.
int Packs_DecodeJpeg(FILE *file, unsigned char *pixels, int flags, int *width, int *height, int reserved);
// Reads one or two images of a Packs image file (see packs_bmp.cpp). EN 0x7bdd20.
int Packs_LoadImageFile(const char *path, unsigned char *colourPixels, unsigned char *alphaPixels,
                                   int decodeJpeg);

// ---- Plane rescaling (src/packs_scale.cpp) ----

// Largest magnitude of a rescaled sample, fraction bits of the ratio and the unit the denominator is counted in.
const int PACKS_SCALE_LIMIT = 0x7fff;
const int PACKS_SCALE_FRACTION_BITS = 8;
const int PACKS_SCALE_DIVISOR_UNIT = 16;

// Rescales three sample planes (see packs_scale.cpp). EN 0x7bab70.
void Packs_ScalePlanes(short *planeFirst, short *planeSecond, short *planeThird, int columns, int rows, int stride,
                       int denominator, int numerator, int biasFirst, int biasSecond, int biasThird);

#endif
