// Colour palette table of the EN build's image converter: the buckets that pixels are matched against (EN-only code
// at EN 0x748a60-0x749290). Declared here once.
#ifndef GURUMIN_EN_PALETTE_TABLE_H
#define GURUMIN_EN_PALETTE_TABLE_H

// One palette bucket (0x1c bytes).
struct PaletteBucket
{
    unsigned char channel[4];   // 0x00: the bucket's colour channels
    int sumChannel0;            // 0x04: sum of the first channel of the pixels that matched this bucket
    int sumChannel1;            // 0x08: sum of the second channel of those pixels
    int sumChannel2;            // 0x0c: sum of the third channel of those pixels
    int sumChannel3;            // 0x10: sum of the fourth channel of those pixels
    int pixelCount;             // 0x14: number of pixels that matched this bucket
    int sumDistance;            // 0x18: sum of the match distances of those pixels
};

// Number of buckets in use. EN 0x0166d40c.
extern "C" int g_paletteBucketCount;
// The buckets (the table is larger than any one function shows). EN 0x0166d410.
extern "C" PaletteBucket g_paletteBuckets[];

// Number of palette entries (and buckets loaded from them).
const int PALETTE_SIZE = 256;
// Palette entry as the image converter reads it (4 bytes).
struct PaletteColour
{
    unsigned char channel[4];   // 0x00: colour channels
};
// Squares of 0..255, used by the colour distance. EN 0x0166d008.
extern "C" int g_paletteSquares[PALETTE_SIZE];
// -1 after a palette was loaded (set by Palette_Load, not read by the code recovered so far). EN 0x0166d408.
extern "C" int g_paletteLastMatch;

// Index of the bucket whose colour is nearest to the four channel values (the fourth counts three times); the squared
// distance goes to `*distance` when given. EN 0x748bd0 (palette_nearest.cpp).
extern "C" int Palette_FindNearest(int channel0, int channel1, int channel2, int channel3, int *distance);
// The same search over the first three channels only; `unused` is not read. EN 0x748b20 (palette_nearest.cpp, EN-only).
extern "C" int Palette_FindNearest3(int channel0, int channel1, int channel2, int unused, int *distance);
// Fills the PALETTE_SIZE buckets from `palette` with their fourth channel 0, zeroes their counters and fills the square
// table. EN 0x748ca0 (palette_buckets.cpp).
extern "C" void Palette_LoadOpaque(const PaletteColour *palette);
// The same, with the fourth channel of each bucket taken from the palette. EN 0x748d30 (palette_buckets.cpp).
extern "C" void Palette_LoadWithAlpha(const PaletteColour *palette);
// Adds each of the `count` pixels (four channels each) to the bucket nearest to it: the channel sums, the pixel count and
// the match distance. EN 0x7491e0 (palette_buckets.cpp).
extern "C" void Palette_Accumulate(const PaletteColour *pixels, int count);

// qsort comparators of the buckets: largest sumDistance first. EN 0x749270 and its identical twin EN 0x749290
// (palette_compare.cpp, EN-only); each is passed to qsort by a different caller.
extern "C" int PaletteBucket_CompareByDistanceDescending(const PaletteBucket *a, const PaletteBucket *b);
extern "C" int PaletteBucket_CompareByDistanceDescendingTwin(const PaletteBucket *a, const PaletteBucket *b);

// Zeroes the six counters of bucket `index` (everything but the colour channels). EN 0x748a60.
extern "C" void Palette_ClearBucket(int index);
// Identical twin of Palette_ClearBucket. EN 0x748ac0.
extern "C" void Palette_ClearBucketTwin(int index);

#endif
