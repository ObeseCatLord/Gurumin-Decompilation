// Plane band decoding of the Packs image codec (EN build, EN-only: no JP twin found).
#include "packs_codec.h"

// Decodes the three bands of one block row: the band of `bands[0]` into the first plane, `bands[1]` into the second and
// `bands[2]` into the third, each at `offset` samples into its plane. Called by the image decoder.
extern "C" void Packs_DecodePlaneBands(const unsigned char *data, short *planeFirst, short *planeSecond, short *planeThird,
                                       int offset, int width, int height, int stride, const PacksBandEntry *bands)
{
    // The band table has a 4-byte entry per band; its first three bytes are the PacksBandHeader the decoder reads.
    Packs_DecodeBand(data, planeFirst + offset, width, height, stride, (const PacksBandHeader *)&bands[0]);
    Packs_DecodeBand(data, planeSecond + offset, width, height, stride, (const PacksBandHeader *)&bands[1]);
    Packs_DecodeBand(data, planeThird + offset, width, height, stride, (const PacksBandHeader *)&bands[2]);
}
