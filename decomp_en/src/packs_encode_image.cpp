// The Packs image encoder (EN build; JP twin: Packs.dll 0x10004fb0). Splits the image into resolution levels, quantizes the
// bands of the three sample planes, encodes them and writes the stream.
#include <stdlib.h>
#include <string.h>

#include "packs_image_encode.h"

// Packs_EncodeImage. Encodes the bottom-up RGB image `pixels` (`width` x `height`) with quality `quality` into `output`
// and stores the stream length in `*outputLength`. Works out how many levels the image is split into (a level is split
// while both sides stay above PACKS_MIN_SPLIT_SIZE and the band scale shifted by the level stays below
// PACKS_SPLIT_SCALE_LIMIT), converts the pixels to three sample planes, splits each plane into levels, quantizes and
// encodes the bands into two code blocks and writes the stream. Returns 1, or 0 when memory or the conversion fails.
// Called by the picture writer.
extern "C" int Packs_EncodeImage(const unsigned char *pixels, int width, int height, int quality, unsigned char *output,
                                 int *outputLength)
{
    int heights[PACKS_MAX_LEVELS + 1];
    int widths[PACKS_MAX_LEVELS + 1];
    int secondLength;
    int levelCount;
    PacksBandEntry descriptors[PACKS_DESCRIPTOR_COUNT];

    g_encodeExtraBits = PACKS_ENCODE_EXTRA_BITS;
    int pixelCount = width * height;
    int bandScale = quality * PACKS_QUALITY_UNIT;
    int pass = 0;
    do
    {
        levelCount = 0;
        widths[0] = width;
        heights[0] = height;
        for (int level = 1; level <= PACKS_MAX_LEVELS; level++)
        {
            widths[level] = (widths[level - 1] + 1) / 2;
            heights[level] = (heights[level - 1] + 1) / 2;
            if (widths[level - 1] > PACKS_MIN_SPLIT_SIZE && heights[level - 1] > PACKS_MIN_SPLIT_SIZE &&
                (bandScale * 2 << (level - 1)) < PACKS_SPLIT_SCALE_LIMIT)
                levelCount = level;
        }

        unsigned char *firstBlock = new unsigned char[widths[levelCount] * heights[levelCount] * PACKS_PACKED_PIXEL_BYTES];
        if (firstBlock == 0)
            return 0;
        int sampleCount = pixelCount * PACKS_ENCODE_PLANES;
        short *scratch = new short[sampleCount];
        if (scratch == 0)
            return 0;
        short *first = scratch + pixelCount;
        short *second = scratch + pixelCount * 2;
        short *third = scratch + pixelCount * 3;
        for (int i = 0; i < sampleCount; i++)
            scratch[i] = 0;
        memset(descriptors, 0, sizeof(descriptors));
        if (Packs_ComponentsToPlanes(pixels, scratch, first, second, third, width, height) != 0)
            return 0;
        for (int level = 0; level < levelCount; level++)
            Packs_DecomposeLevel(first, scratch, widths[level], heights[level], width);
        for (int level = 0; level < levelCount; level++)
            Packs_DecomposeLevel(second, scratch, widths[level], heights[level], width);
        for (int level = 0; level < levelCount; level++)
            Packs_DecomposeLevel(third, scratch, widths[level], heights[level], width);
        Packs_QuantizeBands(first, second, third, widths, heights, quality * PACKS_QUALITY_UNIT, levelCount, descriptors);
        Packs_PackPlanes(first, second, third, firstBlock, widths[levelCount], heights[levelCount], width);
        Packs_ArrangeBands(first, scratch, widths, heights, levelCount);
        Packs_ArrangeBands(second, first, widths, heights, levelCount);
        Packs_ArrangeBands(third, second, widths, heights, levelCount);
        Packs_EncodePlanes(scratch, first, second, third, widths, heights, levelCount, &secondLength, descriptors);
        Packs_WriteStream(firstBlock, (const unsigned char *)third, widths, heights, quality * PACKS_QUALITY_UNIT,
                          levelCount, &secondLength, (const unsigned char *)descriptors, output, outputLength);
        free(firstBlock);
        free(scratch);
        pass++;
    } while (pass < PACKS_ENCODE_PASSES);
    return 1;
}
