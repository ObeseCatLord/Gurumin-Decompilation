// Fixed-point rescaling of the three sample planes of the Packs image codec (EN build: Packs.dll is linked into the
// exe). No JP twin source matched (JP: Packs.dll 10003600).
#include "packs_codec.h"

// Rescales one signed sample: value * numerator + (value * remainder + half + bias) / divisor, rounded away from zero for
// negative samples, and limited to +-PACKS_SCALE_LIMIT. Zero stays zero.
static int Packs_ScaleSample(int value, int wholeFactor, int fractionFactor, int half, int bias, int divisor)
{
    int scaled = 0;
    if (value != 0)
    {
        if (value > 0)
            scaled = value * wholeFactor + (value * fractionFactor + half + bias) / divisor;
        else
            scaled = value * wholeFactor - ((half - value * fractionFactor) + bias) / divisor;
    }
    if (scaled <= -PACKS_SCALE_LIMIT)
        scaled = -PACKS_SCALE_LIMIT;
    else if (scaled >= PACKS_SCALE_LIMIT)
        scaled = PACKS_SCALE_LIMIT;
    return scaled;
}

// Rescales `columns` x `rows` samples of three planes by ratio `numerator << 8 / (denominator * 16)`, adding a separate
// rounding bias per plane. Samples of a column are `stride` shorts apart; columns are one short apart. Called by the
// image decoder (EN 0x7bad40).
void Packs_ScalePlanes(short *planeFirst, short *planeSecond, short *planeThird, int columns, int rows, int stride,
                       int denominator, int numerator, int biasFirst, int biasSecond, int biasThird)
{
    int divisor = denominator * PACKS_SCALE_DIVISOR_UNIT;
    int wholeFactor = (numerator << PACKS_SCALE_FRACTION_BITS) / divisor - 1;
    int fractionFactor = (numerator << PACKS_SCALE_FRACTION_BITS) % divisor + divisor;
    int half = (divisor - 1) >> 1;
    for (int column = 0; column < columns; column++)
    {
        for (int row = 0; row < rows; row++)
        {
            int index = column + row * stride;
            planeFirst[index] = (short)Packs_ScaleSample(planeFirst[index], wholeFactor, fractionFactor, half, biasFirst, divisor);
            planeSecond[index] = (short)Packs_ScaleSample(planeSecond[index], wholeFactor, fractionFactor, half, biasSecond, divisor);
            planeThird[index] = (short)Packs_ScaleSample(planeThird[index], wholeFactor, fractionFactor, half, biasThird, divisor);
        }
    }
}
