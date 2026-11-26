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

// Rescales the detail bands of every level of three sample planes, from the smallest level (`levelCount`) up to level 1.
// Level `n` has `widths[n]` x `heights[n]` samples in the top-left corner of the full plane (rows `widths[0]` samples
// apart); its three detail bands (right, below and diagonal of the next smaller level) are rescaled by the factor of
// level `n` for the first two and of level `n - 1` for the diagonal one. The factors start at PACKS_SCALE_ONE and halve
// per level, but never drop below `minScale`. `bands` holds PACKS_BANDS_PER_LEVEL entries per level, the biases being
// the top byte of each entry minus PACKS_BIAS_ORIGIN, times the factor. Called by the image decoder (EN 0x7b8f30).
void Packs_ScaleLevels(short *planeFirst, short *planeSecond, short *planeThird, int *widths, int *heights, int minScale,
                       int levelCount, const PacksBandEntry *bands)
{
    int scales[PACKS_LEVEL_COUNT];
    scales[0] = PACKS_SCALE_ONE;
    for (int level = 1; level < PACKS_LEVEL_COUNT; level++)
    {
        scales[level] = scales[level - 1] >> 1;
        if (scales[level] < minScale)
            scales[level] = minScale;
    }
    int fullWidth = widths[0];
    for (int level = levelCount; level > 0; level--)
    {
        const PacksBandEntry *levelBands = bands + (levelCount - level) * PACKS_BANDS_PER_LEVEL;
        int scale = scales[level];
        Packs_ScalePlanes(planeFirst + widths[level], planeSecond + widths[level], planeThird + widths[level],
                          widths[level - 1] - widths[level], heights[level], fullWidth, minScale, scale,
                          (levelBands[0].bias - PACKS_BIAS_ORIGIN) * scale, (levelBands[1].bias - PACKS_BIAS_ORIGIN) * scale,
                          (levelBands[2].bias - PACKS_BIAS_ORIGIN) * scale);
        int rowOffset = fullWidth * heights[level];
        Packs_ScalePlanes(planeFirst + rowOffset, planeSecond + rowOffset, planeThird + rowOffset, widths[level],
                          heights[level - 1] - heights[level], fullWidth, minScale, scale,
                          (levelBands[3].bias - PACKS_BIAS_ORIGIN) * scale, (levelBands[4].bias - PACKS_BIAS_ORIGIN) * scale,
                          (levelBands[5].bias - PACKS_BIAS_ORIGIN) * scale);
        int cornerOffset = (fullWidth * heights[level] + widths[level]);
        int cornerScale = scales[level - 1];
        Packs_ScalePlanes(planeFirst + cornerOffset, planeSecond + cornerOffset, planeThird + cornerOffset,
                          widths[level - 1] - widths[level], heights[level - 1] - heights[level], fullWidth, minScale,
                          cornerScale, (levelBands[6].bias - PACKS_BIAS_ORIGIN) * cornerScale,
                          (levelBands[7].bias - PACKS_BIAS_ORIGIN) * cornerScale,
                          (levelBands[8].bias - PACKS_BIAS_ORIGIN) * cornerScale);
    }
}
