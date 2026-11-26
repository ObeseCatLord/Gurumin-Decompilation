// Coefficient band encoder of the Packs image codec (EN build). Ported from the Ghidra output and the EN bytes; the
// counterpart of Packs_DecodeBand (src/packs_band.cpp).
#include "packs_codec.h"

// Returns the probability (16 bits, at most PACKS_BAND_PROBABILITY_MAX) that goes with an activity value.
static inline unsigned int ProbabilityOfActivity(unsigned int activity)
{
    unsigned int probability =
        (((activity >> 1) - activity) - PACKS_ARITH_ROUND) / (activity + PACKS_BAND_PROBABILITY_BIAS) + 1;
    if (probability > PACKS_BAND_PROBABILITY_MAX)
        probability = PACKS_BAND_PROBABILITY_MAX;
    return probability;
}

// Encodes `count` packed coefficients `data` into `output` and fills the band header `band` first. Each packed word holds
// a run length minus one in its low four bits (zeros before the value) and the value above it: an 11-bit magnitude and a
// sign. Every zero is coded as a 0 decision, a value as `magnitude` 1 decisions, a 0 decision and the sign; the
// probability of each decision follows the activity of the band as in Packs_DecodeBand. Returns the number of packed words
// used (0 when the band is empty). Called by the plane encoders.
extern "C" int Packs_EncodeBand(const unsigned short *data, unsigned char *output, int count, int reserved,
                                PacksBandHeader *band)
{
    Packs_MeasureBand(band, data, count);
    unsigned int scale = band->scale;
    if (scale == 0)
        return 0;
    unsigned int startActivity = scale * scale * scale * scale;
    int otherGain = PACKS_BAND_WEIGHT_ONE - band->gain;
    int keepWeight = (PACKS_BAND_WEIGHT_ONE - band->keep) * otherGain;
    int mixWeight = otherGain * band->keep;
    unsigned int gainStep = band->gain * PACKS_BAND_WEIGHT_ONE;
    int cells = 0;
    g_packsWriteByte = 0;
    g_packsWriteMask = PACKS_FIRST_BIT;
    unsigned int startShare = (((startActivity & 0xffff) * keepWeight + PACKS_ARITH_ROUND) >> PACKS_ARITH_SHIFT) +
                              (startActivity >> PACKS_ARITH_SHIFT) * keepWeight;
    g_encodeLow = 0;
    g_encodeHigh = g_encodeMax;
    g_encodePending = 0;
    int used = 0;
    int zeros = 0;
    unsigned int activity = startActivity;
    unsigned int probability = ProbabilityOfActivity(activity);
    while (cells < count)
    {
        unsigned short word = data[used++];
        unsigned short value = word >> PACKS_RUN_BITS;
        unsigned int run = (unsigned short)((word & PACKS_RUN_MASK) + 1);
        cells += run;
        zeros += run;
        if (value != 0 || cells == count)
        {
            zeros--;
            while (zeros != 0)
            {
                Packs_EncodeBit(0, probability, output);
                activity = (activity >> PACKS_ARITH_SHIFT) * mixWeight +
                           (((activity & 0xffff) * mixWeight + PACKS_ARITH_ROUND) >> PACKS_ARITH_SHIFT) + startShare;
                if (activity == 0)
                    activity = 1;
                probability = ProbabilityOfActivity(activity);
                zeros--;
            }
            if (value == 0)
            {
                Packs_EncodeBit(0, probability, output);
            }
            else
            {
                unsigned int magnitude = value & PACKS_MAGNITUDE_MASK;
                for (unsigned int step = magnitude; step != 0; step--)
                    Packs_EncodeBit(1, probability, output);
                Packs_EncodeBit(0, probability, output);
                Packs_EncodeBit(word >> PACKS_SIGN_SHIFT, PACKS_ARITH_ROUND, output);
                activity = magnitude * gainStep + (activity >> PACKS_ARITH_SHIFT) * mixWeight +
                           startShare + (((activity & 0xffff) * mixWeight + PACKS_ARITH_ROUND) >> PACKS_ARITH_SHIFT);
                if (activity == 0)
                    activity = 1;
                probability = ProbabilityOfActivity(activity);
            }
        }
    }
    g_encodePending += g_encodeExtraBits;
    Packs_EncodeFlush(g_encodeLow >= g_encodeHalf, output);
    if (g_packsWriteMask > PACKS_FIRST_BIT)
    {
        output[g_packsWritePosition] = g_packsWriteByte;
        g_packsWritePosition++;
    }
    return used;
}
