// Vertical flip of 24-bit bitmap pixels (EN build, EN-only: no JP twin found).
#include "texture_mgr.h"

// Bytes of one pixel of a 24-bit bitmap.
const int BITMAP_BYTES_PER_PIXEL = 3;

// Bitmap_FlipRows24: turns the 24-bit image `pixels` (width x height, rows packed) upside down by swapping row i with
// row height - 1 - i, three bytes at a time. Returns 1. Called by Texture_LoadBmp (bmp rows are stored bottom up).
extern "C" int Bitmap_FlipRows24(unsigned char *pixels, int width, int height)
{
    int rowBytes = width * BITMAP_BYTES_PER_PIXEL;
    unsigned char *top = pixels;
    unsigned char *bottom = pixels + (height - 1) * rowBytes;
    for (int row = height / 2; row > 0; row--)
    {
        for (int x = 0; x < rowBytes; x += BITMAP_BYTES_PER_PIXEL)
        {
            unsigned char first = top[x];
            unsigned char second = top[x + 1];
            unsigned char third = top[x + 2];
            top[x] = bottom[x];
            top[x + 1] = bottom[x + 1];
            top[x + 2] = bottom[x + 2];
            bottom[x] = first;
            bottom[x + 1] = second;
            bottom[x + 2] = third;
        }
        bottom -= rowBytes;
        top += rowBytes;
    }
    return 1;
}
