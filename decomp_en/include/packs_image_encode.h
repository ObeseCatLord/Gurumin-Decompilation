// The Packs image encoder (EN build; JP: Packs.dll 0x10004fb0). Declared once here: the encoder and the stages it calls
// that are not ported yet or whose declaration lives only in their own source file.
#ifndef GURUMIN_EN_PACKS_IMAGE_ENCODE_H
#define GURUMIN_EN_PACKS_IMAGE_ENCODE_H

#include "packs_codec.h"

// Levels the encoder may split an image into (the level tables hold one more entry for the full-size level).
const int PACKS_MAX_LEVELS = 8;
// Smallest level size (in pixels, both ways) that is still split further.
const int PACKS_MIN_SPLIT_SIZE = 11;
// A level is only split while the band scale shifted by the level stays below this.
const int PACKS_SPLIT_SCALE_LIMIT = 0x5a828;
// Units per quality step: the quantizer gets quality * PACKS_QUALITY_UNIT, the level test twice that.
const int PACKS_QUALITY_UNIT = 0x40;
// Value g_encodeExtraBits is set to before encoding.
const int PACKS_ENCODE_EXTRA_BITS = 0x10;
// Planes of samples the encoder keeps: a scratch plane and three image planes.
const int PACKS_ENCODE_PLANES = 4;
// Bytes of a packed pixel (three samples in 32 bits).
const int PACKS_PACKED_PIXEL_BYTES = 4;
// Entries of the band descriptor table (nine per level and eight levels).
const int PACKS_DESCRIPTOR_COUNT = 0x48;
// Encoding passes (the encoder runs once).
const int PACKS_ENCODE_PASSES = 1;

// Converts the bottom-up RGB bytes `pixels` (`width` x `height`) to three sample planes `first`, `second`, `third` (the
// fourth buffer `scratch` is working space). Returns non-zero on failure. EN 0x7bc3a0, not ported yet.
extern "C" int Packs_ComponentsToPlanes(const unsigned char *pixels, short *scratch, short *first, short *second,
                                        short *third, int width, int height);
// Quantizes the `levelCount` levels of the three planes into bands with the quality `bandScale` and fills `descriptors`
// (nine entries per level). EN 0x7bbfe0, not ported yet.
extern "C" void Packs_QuantizeBands(short *first, short *second, short *third, int *widths, int *heights, int bandScale,
                                    int levelCount, PacksBandEntry *descriptors);
// Moves the bands of one plane to the layout the band encoder reads: `plane` from `source` over `levelCount` levels.
// EN 0x7b9210, not ported yet.
extern "C" void Packs_ArrangeBands(short *plane, short *source, int *widths, int *heights, int levelCount);
// Encodes the bands of the three planes into the two code blocks; `secondLength` gets the size of the second block.
// EN 0x7bb280, not ported yet.
extern "C" void Packs_EncodePlanes(short *scratch, short *first, short *second, short *third, int *widths, int *heights,
                                   int levelCount, int *secondLength, PacksBandEntry *descriptors);
// Packs the three planes into 32-bit pixels (see packs_image.cpp). EN 0x7b95a0.
extern "C" void Packs_PackPlanes(short *planeFirst, short *planeSecond, short *planeThird, unsigned char *output,
                                 int width, int height, int stride);
// Writes the whole stream of an encoded image (see packs_write_header.cpp). EN 0x7baf40.
extern "C" int Packs_WriteStream(const unsigned char *firstBlock, const unsigned char *secondBlock, int *widths,
                                 int *heights, int width, int levelCount, int *secondLength,
                                 const unsigned char *descriptors, unsigned char *output, int *position);

// Encodes the bottom-up RGB image `pixels` (`width` x `height`) with quality `quality` into `output`; `*outputLength` gets
// the stream length. Returns 1, or 0 when memory or the plane conversion fails. EN 0x7b8af0.
extern "C" int Packs_EncodeImage(const unsigned char *pixels, int width, int height, int quality, unsigned char *output,
                                 int *outputLength);

#endif
