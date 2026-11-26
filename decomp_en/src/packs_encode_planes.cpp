// Plane band encoding of the Packs image codec (EN build, EN-only: no JP twin found).
#include "packs_codec.h"

// Encodes the band of the first plane (from the cell it has reached), then that of the second and of the third, each
// with its own band header, and adds the cells each one used to its running count. Called by the image encoder.
extern "C" int Packs_EncodePlaneBands(const unsigned short *first, const unsigned short *second,
                                      const unsigned short *third, unsigned char *output, int count, int reserved,
                                      PacksBandHeader *bands)
{
    g_packsEncodedCells[0] += Packs_EncodeBand(first + g_packsEncodedCells[0], output, count, reserved, &bands[0]);
    g_packsEncodedCells[1] += Packs_EncodeBand(second + g_packsEncodedCells[1], output, count, reserved, &bands[1]);
    g_packsEncodedCells[2] += Packs_EncodeBand(third + g_packsEncodedCells[2], output, count, reserved, &bands[2]);
    return 0;
}
