// Header writer of the Packs image encoder (EN build). Ported from the Ghidra output and the EN bytes; the counterpart
// of the header reader inside the image decoder (EN 0x7b8f30).
#include "packs_codec.h"

// Bytes of the stream header written in front of the image data: the "HPC@" signature, the format version and a zero byte.
const unsigned char PACKS_SIGNATURE_0 = 'H';
const unsigned char PACKS_SIGNATURE_1 = 'P';
const unsigned char PACKS_SIGNATURE_2 = 'C';
const unsigned char PACKS_SIGNATURE_3 = '@';
const unsigned char PACKS_STREAM_VERSION = 3;
// A width of exactly this many pixels is stored as 0 (the field has 16 bits).
const int PACKS_WIDTH_WRAP = 0x10000;
// Band descriptors per level (the table holds this many four-byte entries for each level).
const int PACKS_DESCRIPTORS_PER_LEVEL = 9;
// Bytes of one descriptor: a flag byte, followed by three more bytes when the flag is non-zero.
const int PACKS_DESCRIPTOR_BYTES = 4;
// Bytes per pixel of the stored first plane.
const int PACKS_PLANE_PIXEL_BYTES = 4;

// Appends one byte to the output and moves the write position on. The value expression is evaluated at every use, so a
// value read through a pointer is read again for each byte (the output bytes may alias it).
#define PUT_BYTE(value) (output[*position] = (value) % 256, *position += 1)
// Appends a byte that is already in range.
#define PUT_RAW(value) (output[*position] = (value), *position += 1)
// Appends `value` as two little-endian bytes.
#define PUT_SHORT(value) PUT_BYTE(value), PUT_BYTE((value) / 0x100)
// Appends `value` as four little-endian bytes.
#define PUT_LONG(value) PUT_BYTE(value), PUT_BYTE((value) / 0x100), PUT_BYTE((value) / 0x10000), PUT_BYTE((value) / 0x10000 / 0x100)

// Writes the whole stream of an encoded image into `output`: the header (signature, width, the size of the first level,
// the level count, the length of the second block), the table of band descriptors, the first block and the second block.
// `*position` is the write position, which starts at 0 and ends at the stream length. Returns the number of descriptor
// bytes written for the table. Called by the image encoder at EN 0x7b8af0.
extern "C" int Packs_WriteStream(const unsigned char *firstBlock, const unsigned char *secondBlock, int *widths,
                                 int *heights, int width, int levelCount, int *secondLength,
                                 const unsigned char *descriptors, unsigned char *output, int *position)
{
    *position = 0;
    output[0] = PACKS_SIGNATURE_0;
    *position += 1;
    PUT_BYTE(PACKS_SIGNATURE_1);
    PUT_BYTE(PACKS_SIGNATURE_2);
    PUT_BYTE(PACKS_SIGNATURE_3);
    PUT_BYTE(PACKS_STREAM_VERSION);
    PUT_BYTE(0);
    if (width == PACKS_WIDTH_WRAP)
        width = 0;
    PUT_SHORT(width);
    PUT_LONG(*widths);
    PUT_LONG(*heights);
    PUT_SHORT(levelCount);
    PUT_BYTE(0);
    PUT_BYTE(0);
    int length = *secondLength;
    PUT_LONG(length);

    int used = 0;
    const unsigned char *descriptor = descriptors;
    for (int i = 0; i < levelCount * PACKS_DESCRIPTORS_PER_LEVEL; i++)
    {
        unsigned char flag = descriptor[0];
        PUT_RAW(flag);
        used++;
        if (flag != 0)
        {
            PUT_RAW(descriptor[1]);
            PUT_RAW(descriptor[2]);
            PUT_RAW(descriptor[3]);
            used += PACKS_DESCRIPTOR_BYTES - 1;
        }
        descriptor += PACKS_DESCRIPTOR_BYTES;
    }

    int firstLength = widths[levelCount] * heights[levelCount] * PACKS_PLANE_PIXEL_BYTES;
    for (int i = 0; i < firstLength; i++)
        PUT_RAW(firstBlock[i]);
    for (int i = 0; i < length; i++)
        PUT_RAW(secondBlock[i]);
    return used;
}
