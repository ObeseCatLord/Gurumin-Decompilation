// Image conversion helpers of the Packs image codec (EN build: Packs.dll is linked into the exe).
// JP twins: Packs.dll 10001950, 10003490, 10002bc0.
#include "packs_codec.h"

// Packs a sample into an output buffer slot: stores `base` at slot `g_packsOutputIndex`, then adds `delta` as a
// signed 12-bit value in the top bits (magnitude in bits 4-14, sign in bit 15, clamped to +-2047) and advances the
// slot index. Called by the image encoder loops.
extern "C" void Packs_StoreSignedSample(unsigned short *output, unsigned short base, short delta)
{
    output[g_packsOutputIndex] = base;
    if (delta > PACKS_SAMPLE_MAX)
        delta = PACKS_SAMPLE_MAX;
    if (delta < -PACKS_SAMPLE_MAX)
        delta = -PACKS_SAMPLE_MAX;
    if (delta >= 0)
        output[g_packsOutputIndex++] += delta << PACKS_SAMPLE_SHIFT;
    else
        output[g_packsOutputIndex++] += PACKS_SAMPLE_SIGN_BIT - (delta << PACKS_SAMPLE_SHIFT);
}

// Value that centres a converted component inside the unsigned clamp range, and the clamp limits (components are
// kept in 7 fractional bits above 8 integer bits).
const unsigned int PACKS_RGB_BIAS = 0x10040;
const unsigned int PACKS_RGB_MIN = 0x10000;
const unsigned int PACKS_RGB_MAX = 0x17fff;
const int PACKS_RGB_SHIFT = 7;

// Limits a biased component to PACKS_RGB_MIN..PACKS_RGB_MAX and scales it to a byte.
static unsigned char Packs_ComponentByte(unsigned int value)
{
    unsigned int atLeastMin = value > PACKS_RGB_MIN ? value : PACKS_RGB_MIN;
    unsigned int clamped = atLeastMin < PACKS_RGB_MAX ? atLeastMin : PACKS_RGB_MAX;
    return (unsigned char)(clamped >> PACKS_RGB_SHIFT);
}

// Converts three planes of width x height signed samples to interleaved 8-bit components: component 0 = 2a - 3(b + 2c),
// component 1 = 2(a + b), component 2 = 2(a + c) - 3b (each biased, clamped and scaled to a byte). Bytes are appended
// to `output` from `*outputLength` (reset to 0 first), which ends as the number of bytes written. Called by the image
// decoder after the inverse transform.
extern "C" void Packs_PlanesToComponents(short *planeA, short *planeB, short *planeC, int *width, int *height,
                                         unsigned char *output, int *outputLength)
{
    *outputLength = 0;
    for (int y = 0; y < *height; y++)
    {
        int x = 0;
        int row = *width * y;
        for (; x < *width; x++)
        {
            short a = planeA[row + x];
            short b = planeB[row + x];
            short c = planeC[row + x];
            unsigned int first = a * 2 + PACKS_RGB_BIAS + (b + c * 2) * -3;
            unsigned int second = (a + b) * 2 + PACKS_RGB_BIAS;
            unsigned int third = (a + c) * 2 + PACKS_RGB_BIAS + b * -3;
            output[*outputLength] = Packs_ComponentByte(first);
            (*outputLength)++;
            output[*outputLength] = Packs_ComponentByte(second);
            (*outputLength)++;
            output[*outputLength] = Packs_ComponentByte(third);
            (*outputLength)++;
        }
    }
}

// Biases added to the three sample planes before packing and the largest values that fit the packed fields.
const int PACKS_PACK_BIAS_FIRST = 0x200;
const int PACKS_PACK_BIAS_SECOND = 0x400;
const int PACKS_PACK_BIAS_THIRD = 0x200;
const int PACKS_FIELD_11_MAX = 0x7ff;
const int PACKS_FIELD_10_MAX = 0x3ff;
const int PACKS_BYTES_PER_PIXEL = 4;

// Packs three sample planes into 32-bit pixels: every sample is biased and clamped (11 bits for the first and second
// plane, 10 bits for the third) and the three fields are stored little-endian in four bytes (first: bits 0-10, second:
// bits 11-21, third: bits 22-31). `width` pixels of each of `height` rows are packed into `output`; planes advance by
// `stride` samples per row and every sample read is cleared to 0 in its plane. Called by the image encoder.
extern "C" void Packs_PackPlanes(short *planeFirst, short *planeSecond, short *planeThird, unsigned char *output,
                                 int width, int height, int stride)
{
    int written = 0;
    int row = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            unsigned short first = planeFirst[row + x] + PACKS_PACK_BIAS_FIRST;
            if ((short)first < 0)
                first = 0;
            else if ((short)first > PACKS_FIELD_11_MAX)
                first = PACKS_FIELD_11_MAX;
            output[written] = first % 256;
            unsigned short firstCarry = first >> 8;
            unsigned short mid = planeSecond[row + x] + PACKS_PACK_BIAS_SECOND;
            if ((short)mid < 0)
                mid = 0;
            else if ((short)mid > PACKS_FIELD_11_MAX)
                mid = PACKS_FIELD_11_MAX;
            output[written + 1] = (mid % 32) * 8 + firstCarry;
            unsigned short midCarry = mid >> 5;
            unsigned short last = planeThird[row + x] + PACKS_PACK_BIAS_THIRD;
            if ((short)last < 0)
                last = 0;
            else if ((short)last > PACKS_FIELD_10_MAX)
                last = PACKS_FIELD_10_MAX;
            output[written + 2] = (last % 4) * 64 + midCarry;
            output[written + 3] = last >> 2;
            written += PACKS_BYTES_PER_PIXEL;
            planeFirst[row + x] = 0;
            planeSecond[row + x] = 0;
            planeThird[row + x] = 0;
        }
        row += stride;
    }
}
