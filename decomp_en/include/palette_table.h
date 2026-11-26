// Colour palette table of the EN build's image converter: the buckets that pixels are matched against (EN-only code
// at EN 0x748a60-0x749290). Declared here once.
#ifndef GURUMIN_EN_PALETTE_TABLE_H
#define GURUMIN_EN_PALETTE_TABLE_H

// One palette bucket (0x1c bytes).
struct PaletteBucket
{
    unsigned char channel[4];   // 0x00: the bucket's colour channels
    int m_0004;                 // 0x04: not yet named (reset together with the next five)
    int m_0008;                 // 0x08: not yet named
    int m_000c;                 // 0x0c: not yet named
    int m_0010;                 // 0x10: not yet named
    int m_0014;                 // 0x14: not yet named
    int m_0018;                 // 0x18: not yet named
};

// Number of buckets in use. EN 0x0166d40c.
extern "C" int g_paletteBucketCount;
// The buckets (the table is larger than any one function shows). EN 0x0166d410.
extern "C" PaletteBucket g_paletteBuckets[];

// Zeroes the six counters of bucket `index` (everything but the colour channels). EN 0x748a60.
extern "C" void Palette_ClearBucket(int index);
// Identical twin of Palette_ClearBucket. EN 0x748ac0.
extern "C" void Palette_ClearBucketTwin(int index);

#endif
