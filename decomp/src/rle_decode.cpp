// The run-length packing of the game's compressed bitmaps (Texture_LoadBmp). Each run starts with a code byte whose
// high nibble is the kind of run; short runs keep their length - 1 in the low nibble, long runs a 12-bit length
// (low nibble, then the next byte) less 17 or 33.

#include <string.h>

// The 12-bit length of a long run: the low nibble of the code byte, then the byte after it.
inline unsigned int LongRunLength(const unsigned char *code)
{
    unsigned short word = *(const unsigned short *)code;
    return (word & 0xf) << 8 | word >> 8;
}

// 0x101612b0 (Rle_Decode). Unpacks `packedSize` bytes at `packed` into the `size` bytes at `out`. Runs:
//   0: 1-16 literal bytes             1: 17-4112 literal bytes
//   2: 1-16 copies of the next byte   3: 17-4112 copies of the byte after the length
//   4: 1-16 zero bytes                5: 33-4128 zero bytes
//   6: 1-16 0xff bytes                7: 33-4128 0xff bytes
// Returns 1 when both buffers were used up exactly, 0 otherwise (or for an unknown run).
extern "C" int FUN_101612b0(unsigned char *out, int size, const unsigned char *packed, int packedSize)
{
    unsigned char *dst = out;
    const unsigned char *src = packed;
    while (size > 0)
    {
        unsigned int code = src[0];
        unsigned int count;
        switch (code >> 4)
        {
        case 0:
            count = (code & 0xf) + 1;
            packedSize -= count + 1;
            memcpy(dst, src + 1, count);
            src += count + 1;
            break;
        case 1:
            count = LongRunLength(src) + 0x11;
            packedSize -= count + 2;
            memcpy(dst, src + 2, count);
            src += count + 2;
            break;
        case 2:
            count = (code & 0xf) + 1;
            packedSize -= 2;
            memset(dst, src[1], count);
            src += 2;
            break;
        case 3:
            count = LongRunLength(src) + 0x11;
            packedSize -= 3;
            memset(dst, src[2], count);
            src += 3;
            break;
        case 4:
            count = (code & 0xf) + 1;
            packedSize -= 1;
            memset(dst, 0, count);
            src += 1;
            break;
        case 5:
            count = LongRunLength(src) + 0x21;
            packedSize -= 2;
            memset(dst, 0, count);
            src += 2;
            break;
        case 6:
            count = (code & 0xf) + 1;
            packedSize -= 1;
            memset(dst, 0xff, count);
            src += 1;
            break;
        case 7:
            count = LongRunLength(src) + 0x21;
            packedSize -= 2;
            memset(dst, 0xff, count);
            src += 2;
            break;
        default:
            return 0;
        }
        size -= count;
        dst += count;
    }
    if (size == 0 && packedSize == 0)
        return 1;
    return 0;
}
