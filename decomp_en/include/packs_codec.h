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
    unsigned short *outputWord;       // 0x00: next 16-bit flag word of the compressed output
    unsigned char *data;              // 0x04: next byte to index (the hash is the 16-bit pair at this address)
    unsigned short *links;            // 0x08: next position in the chain, indexed by position & PACKS_LZ_WINDOW_MASK
    unsigned short *chainTail;        // 0x0c: last position of each chain, indexed by hash
    unsigned short *chainHead;        // 0x10: first position of each chain, indexed by hash
    int m_0014;                       // 0x14: not yet named
    int outputBytes;                  // 0x18: bytes of compressed output written so far
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

// ---- Compressor driver (src/packs_compress.cpp) ----

// Bytes of input compressed per block, the sum added to a length before the block count is taken, and the size of one
// hash table piece the compressor clears before each block.
const unsigned int PACKS_BLOCK_INPUT = 0x7ff0;
const unsigned int PACKS_BLOCK_COUNT_BIAS = 0xffef;
const int PACKS_LZ_CLEAR_PIECE = 0x8000;
// Bytes in the header of the packed stream (the unpacked length and the block count).
const int PACKS_COMPRESSED_HEADER_BYTES = 8;
// Start value of PacksLzState::m_0014.
const int PACKS_LZ_STATE_START = 8;
// Progress ratio of the compressor: (blocks done * PACKS_PROGRESS_BLOCK + step * PACKS_PROGRESS_STEP) / (weight *
// PACKS_PROGRESS_WEIGHT + blocks * PACKS_PROGRESS_PER_BLOCK).
const unsigned int PACKS_PROGRESS_BLOCK = 0x319c;
const unsigned int PACKS_PROGRESS_STEP = 0x64000;
const unsigned int PACKS_PROGRESS_WEIGHT = 0x1000;
const unsigned int PACKS_PROGRESS_PER_BLOCK = 0x7f;

// Buffers of the compressor: the block being compressed (EN 0x025eb508), its compressed form with the 16-bit size first
// (EN 0x025ab518), the hash chain links (EN 0x0262b4f8, 0x10000 bytes), and the chain tails and heads (EN 0x0266b4f8 and
// 0x026eb4f8, 0x20000 bytes each).
extern "C" unsigned char g_packsLzInput[];
extern "C" unsigned char g_packsLzOutput[];
extern "C" unsigned char g_packsLzLinks[];
extern "C" unsigned char g_packsLzChainTail[];
extern "C" unsigned char g_packsLzChainHead[];
// Progress callback of the compressor (EN 0x0222b20c): called after each block; non-zero stops compressing.
extern "C" PacksProgressCallback g_packsCompressProgress;

// Compresses one block described by `state` (EN 0x7bd1e0). Returns 0.
extern "C" int Packs_LzEncode(PacksLzState *state);
// Compresses `length` bytes at `input` into `output` as blocks of PACKS_BLOCK_INPUT bytes (header: length and block
// count, then per block the size, the compressed data and a flag for more blocks). Returns the bytes written. EN 0x7bcd10.
extern "C" int Packs_Compress(const unsigned char *input, int *output, int length);

// ---- Arithmetic decoder (src/packs_arith.cpp) ----

// State of the arithmetic decoder: the range limits that trigger a shift (quarter, half, three quarters of the code
// range), the top of the range, the code value read from the bit stream, the low and high end of the current interval
// and the number of bits consumed.
extern "C" unsigned int g_arithQuarter;          // EN 0x0222b154
extern "C" unsigned int g_arithHalf;             // EN 0x0222b158
extern "C" unsigned int g_arithThreeQuarters;    // EN 0x0222b15c
extern "C" unsigned int g_arithTop;              // EN 0x0222b160
extern "C" int g_arithBitsRead;                  // EN 0x0222b170
extern "C" unsigned int g_arithValue;            // EN 0x0222b174
extern "C" unsigned int g_arithLow;              // EN 0x0222b178
extern "C" unsigned int g_arithHigh;             // EN 0x0222b17c

// Rounding added before the interval is split and the shift that divides by the total frequency range.
const unsigned int PACKS_ARITH_ROUND = 0x8000;
const int PACKS_ARITH_SHIFT = 16;
// Decodes one binary decision of probability `probability` (16 bits) from `data`: returns 0 as the low half of the
// result when the decision was 0, otherwise the number of decisions taken. EN 0x7ba1f0 (called by the image decoder
// at EN 0x7b9830).
extern "C" unsigned int Packs_ArithDecode(const unsigned char *data, unsigned int probability);

// ---- Coefficient band decoder (src/packs_band.cpp) ----

// Highest value of the arithmetic code range (the decoder's initial high end). EN 0x0222b164.
extern "C" unsigned int g_arithMax;
// Bits of the code value read before the first decision. EN 0x00909508.
extern "C" int g_arithValueBits;
// Cells in one scan block (a 16 x 16 block of coefficients) and the offsets of each cell from the block's corner.
const int PACKS_BAND_CELLS = 256;
const int PACKS_BAND_BLOCK = 16;
extern "C" int g_packsCellX[PACKS_BAND_CELLS];   // EN 0x00909510: column offset of each cell, in scan order
extern "C" int g_packsCellY[PACKS_BAND_CELLS];   // EN 0x00909910: row offset of each cell, in scan order
// Largest probability handed to Packs_ArithDecode and the divisor offset of the probability formula.
const unsigned int PACKS_BAND_PROBABILITY_MAX = 0xfffe;
const unsigned int PACKS_BAND_PROBABILITY_BIAS = 0x10000;
// Weight scale of a band's activity (bytes of the band header are out of 256).
const int PACKS_BAND_WEIGHT_ONE = 0x100;

// Header of one coefficient band: the activity of its first cell and two weights (out of 256).
struct PacksBandHeader
{
    unsigned char scale;     // 0x00: 0 = the band is empty, else its starting activity is scale to the fourth power
    unsigned char gain;      // 0x01: how much a decoded coefficient raises the activity (out of 256)
    unsigned char keep;      // 0x02: how much of the previous activity is kept (out of 256)
    unsigned char bias;      // 0x03: rescale bias of the band (the fourth byte of a PacksBandEntry), not touched by the band coder
};

// Decodes one band of width x height coefficients from `data` into `output` (rows `stride` apart). An empty band is
// filled with zeros. Cells are visited block by block in scan order; the probability of each one follows an activity
// that is updated from the previous coefficient. Leaves the bit position of the data in g_packsReadPosition. EN 0x7b9830,
// called by the plane decoders at EN 0x7ba000 and 0x7bc1c0.
extern "C" void Packs_DecodeBand(const unsigned char *data, short *output, int width, int height, int stride,
                                 const PacksBandHeader *band);

// ---- Coefficient band encoder (src/packs_encode_band.cpp) ----

// State of the arithmetic encoder (the counterpart of the decoder state above): the range limits, the top and the largest
// value of the code range, the number of undecided bits waiting after a shift, and the low and high end of the interval.
extern "C" unsigned int g_encodeQuarter;       // EN 0x0222b188
extern "C" unsigned int g_encodeHalf;          // EN 0x0222b18c
extern "C" unsigned int g_encodeThreeQuarters; // EN 0x0222b190
extern "C" unsigned int g_encodeTop;           // EN 0x0222b194
extern "C" unsigned int g_encodeMax;           // EN 0x0222b198
extern "C" int g_encodePending;                // EN 0x0222b1a0: bits waiting for the interval to settle
extern "C" unsigned int g_encodeLow;           // EN 0x0222b1a4
extern "C" unsigned int g_encodeHigh;          // EN 0x0222b1a8
extern "C" int g_encodeExtraBits;              // EN 0x0222b184: bits added to the pending count when a band ends
// Runs of zeros are coded as a count, other values as a 4-bit run length minus one plus an 11-bit magnitude and a sign.
const int PACKS_RUN_BITS = 4;
const unsigned int PACKS_RUN_MASK = 0xf;
const unsigned int PACKS_MAGNITUDE_MASK = 0x7ff;
const int PACKS_SIGN_SHIFT = 15;
const int PACKS_MAGNITUDE_SHIFT = 11;

// Measures the packed coefficients `data` (`count` cells) and fills the band header `band` (scale, gain and keep weights).
// EN 0x7ba440 (JP Packs.dll 0x10001000).
extern "C" int Packs_MeasureBand(PacksBandHeader *band, const unsigned short *data, int count);
// Encodes one decision `bit` of probability `probability` (16 bits) into `output`. EN 0x7b9400.
extern "C" void Packs_EncodeBit(int bit, unsigned int probability, unsigned char *output);
// Ends the code stream: writes the bits that settle the interval. EN 0x7bb580.
extern "C" void Packs_EncodeFlush(int bit, unsigned char *output);
// Encodes `count` packed coefficients `data` into `output`, preceded by the band header: runs of zeros and values
// follow the same activity model as Packs_DecodeBand. Returns the number of packed cells used. EN 0x7bb630, called by the
// plane encoders at EN 0x7bb280 and 0x7bb8d0.
extern "C" int Packs_EncodeBand(const unsigned short *data, unsigned char *output, int count, int reserved,
                                PacksBandHeader *band);

// The three planes whose bands one block row holds.
const int PACKS_PLANE_COUNT = 3;
// Packed cells the plane encoder has used so far in each plane's cell buffer. EN 0x0222b1ac (plane 0), 0x0222b1b0, 0x0222b1b4.
extern "C" int g_packsEncodedCells[PACKS_PLANE_COUNT];
// Encodes the band of each of the three planes (`first`, `second`, `third`, each read from the plane's current cell count
// on) with Packs_EncodeBand, into `output`, with the band headers `bands[0..2]`, and advances the three cell counts.
// Returns 0. EN 0x7bb8d0 (packs_encode_planes.cpp).
extern "C" int Packs_EncodePlaneBands(const unsigned short *first, const unsigned short *second,
                                      const unsigned short *third, unsigned char *output, int count, int reserved,
                                      PacksBandHeader *bands);

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

// One entry of the band table the image header carries (four bytes); only the last byte, the rescale bias offset by
// PACKS_BIAS_ORIGIN, is read by Packs_ScaleLevels.
struct PacksBandEntry
{
    unsigned char kind;       // 0x00: 0 = band empty, else the band stores data
    unsigned char detail[2];  // 0x01: stored values of the band (not read by the rescaler)
    unsigned char bias;       // 0x03: rescale bias of the band, stored with PACKS_BIAS_ORIGIN added
};

// Levels (and level scale factors) the codec supports, band table entries used per level, the factor of level 0, and the
// value the stored bias is offset by.
const int PACKS_LEVEL_COUNT = 9;
const int PACKS_BANDS_PER_LEVEL = 9;
const int PACKS_SCALE_ONE = 0x10000;
const int PACKS_BIAS_ORIGIN = 0x80;

// Rescales the detail bands of every level of three sample planes: level `level` is scaled by a factor that halves per
// level but never drops below `minScale`, with the band biases of `bands` (nine entries per level). EN 0x7bad40, called by
// the image decoder (EN 0x7b8f30).
void Packs_ScaleLevels(short *planeFirst, short *planeSecond, short *planeThird, int *widths, int *heights, int minScale,
                       int levelCount, const PacksBandEntry *bands);

// ---- Forward sample transform (src/packs_planes.cpp) ----

// Writes the three transformed samples of one pixel at `index` of the three planes. EN 0x7bab10.
extern "C" void Packs_TransformPixel(short *planeA, short *planeB, short *planeC, int index, int first, int second,
                                     int third);

// ---- Bit coders (src/packs_bits.cpp) ----

// Byte-wise bit reader: mask of the next bit, the byte being read and the position of that byte in the data.
extern "C" unsigned char g_packsReadMask;       // EN 0x0222b168
extern "C" unsigned char g_packsReadByte;       // EN 0x0222b169
extern "C" int g_packsReadPosition;             // EN 0x0222b16c
// Byte-wise bit writer: the byte being built, the mask of the next bit and the position of the next byte in the output.
extern "C" unsigned char g_packsWriteByte;      // EN 0x0222b16a
extern "C" unsigned char g_packsWriteMask;      // EN 0x0222b16b
extern "C" int g_packsWritePosition;            // EN 0x0222b19c
// Word-wise flag writer of the compressor: mask of the next flag bit and the flag word being built.
struct PacksFlagWriter
{
    unsigned short mask;      // 0x00: next flag bit (0 once the 16 bits of the word are used)
    unsigned short m_0002;    // 0x02: not yet named
    unsigned short *word;     // 0x04: the flag word being built
};
extern "C" PacksFlagWriter g_packsFlagWriter;   // EN 0x0222b1e4
// Word-wise bit reader of the block decoder: bytes of the packed data used, the 16 bits being read and how many are left.
struct PacksWordReader
{
    int inputBytes;           // 0x00: bytes of the packed data used
    unsigned short buffer;    // 0x04: the 16 bits being read (low bit first)
    unsigned char count;      // 0x06: bits of `buffer` not read yet
};
extern "C" PacksWordReader g_packsWordReader;   // EN 0x0222b200

// Mask of the first bit of a byte and of the last one.
const unsigned char PACKS_FIRST_BIT = 0x01;
const unsigned char PACKS_LAST_BIT = 0x80;
// Bits in one packed word.
const unsigned char PACKS_WORD_BITS = 16;
// Bytes in one packed word.
const int PACKS_WORD_BYTES = 2;

// Returns the next bit of `data` (1 or 0) and moves on; at the end of a byte the next byte is loaded. EN 0x7ba1a0.
extern "C" int Packs_ReadBit(const unsigned char *data);
// Adds `bit` to the byte being built; a full byte is stored at the next position of `output`. EN 0x7bb4b0.
extern "C" void Packs_WriteBit(int bit, unsigned char *output);
// Starts the next flag bit of the compressor's flag word (a new word at the output when the last one is full) and
// clears it. EN 0x7bd070. Called by the compressor at EN 0x7bd240.
extern "C" void Packs_LzWriteZeroFlag(void);
// Returns the next bit of the packed stream, refilling 16 bits at a time. EN 0x7bd190. Called by Packs_DecodeBlock.
extern "C" int Packs_ReadStreamBit(void);
// Starts the next flag bit of the compressor's flag word like Packs_LzWriteZeroFlag, but sets it. EN 0x7bd0c0. Called by the
// compressor at EN 0x7bd240 and EN 0x7bcff0.
extern "C" void Packs_LzWriteOneFlag(void);
// Writes the lowest `count` bits of `value` as one/zero flags, highest of them first. EN 0x7bcff0 (packs_bits.cpp).
extern "C" void Packs_LzWriteLowBits(unsigned char value, int count);
// Reads `count` bits of the packed stream with Packs_ReadStreamBit, first bit highest, and returns them as a number. EN
// 0x7bd120. Called by Packs_DecodeBlock.
extern "C" int Packs_ReadStreamBits(int count);

// Start of the bytes the compressor is matching in (candidate positions are offsets into it). EN 0x0222b1bc.
extern "C" unsigned char *g_packsLzBlockStart;
// Bytes the hash of a candidate already guarantees to be equal, where the match comparison starts.
const unsigned int PACKS_LZ_HASHED_BYTES = 2;
// Length of the match between the bytes at `candidate` of g_packsLzBlockStart and the bytes at PacksLzState::data: counts equal
// bytes from PACKS_LZ_HASHED_BYTES on, at most up to `maxLength` and not past the end of the input. EN 0x7bd730.
extern "C" unsigned int Packs_LzMatchLength(unsigned short candidate, unsigned short maxLength);

// Decodes the three bands of one block row, one into each of the three planes (the plane offset `offset` is counted in
// samples), with the first three band entries of `bands`. EN 0x7ba000.
extern "C" void Packs_DecodePlaneBands(const unsigned char *data, short *planeFirst, short *planeSecond, short *planeThird,
                                       int offset, int width, int height, int stride, const PacksBandEntry *bands);

#endif
