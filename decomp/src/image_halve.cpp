// Halving an RGB image (used by the texture loader for the smaller texture quality setting).

#include "engine_memory.h"
#include "texture_load_iti.h"

#define RGB_BYTES 3

// 0x10161770 (Image_HalveRgb). Shrinks the `width` x `height` 24-bit image in `rgb` to half the size in place: every
// output pixel is the average of a 2x2 block (each colour byte averaged separately), written to a temporary buffer
// that is copied back over `rgb`. Returns 0 when `rgb` is null, otherwise 1.
extern "C" int FUN_10161770(void *rgb, int width, int height)
{
    if (rgb == 0)
        return 0;
    int halfWidth = width / 2;
    height = height / 2;
    unsigned char *result = (unsigned char *)DAT_11b0b938(height * halfWidth * RGB_BYTES, "a");
    if (height > 0)
    {
        int rowBytes = halfWidth * RGB_BYTES;
        unsigned char *out = result + 1;
        unsigned char *row0 = (unsigned char *)rgb + 1;
        unsigned char *row1 = (unsigned char *)rgb + width * RGB_BYTES;
        int rowsLeft = height;
        do
        {
            int x = 0;
            if (rowBytes > 0)
            {
                unsigned char *b = row1 + 4;
                unsigned char *a = row0;
                unsigned char *c = row1;
                do
                {
                    out[x - 1] = (unsigned char)((b[-1] + a[2] + a[-1] + *c) / 4);
                    out[x] = (unsigned char)((b[-3] + a[3] + *b + *a) / 4);
                    out[x + 1] = (unsigned char)((b[-2] + b[1] + a[4] + a[1]) / 4);
                    x += RGB_BYTES;
                    b += 6;
                    c += 6;
                    a += 6;
                } while (x < rowBytes);
            }
            row0 += width * 6;
            row1 += width * 6;
            out += rowBytes;
            rowsLeft--;
        } while (rowsLeft != 0);
    }
    DAT_11b07530(rgb, result, height * halfWidth * RGB_BYTES);
    DAT_11b1ba40(result);
    return 1;
}
