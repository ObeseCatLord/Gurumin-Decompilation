// Coefficient band decoder of the Packs image codec (EN build). Ported from the Ghidra output and the EN bytes.
#include "packs_codec.h"

// Bits in a byte and the shift that divides by it.
const int PACKS_BITS_PER_BYTE = 8;
const int PACKS_BYTE_SHIFT = 3;

// Returns the probability (16 bits, at most PACKS_BAND_PROBABILITY_MAX) that goes with an activity value.
static inline unsigned int ProbabilityOfActivity(unsigned int activity)
{
    unsigned int probability =
        (((activity >> 1) - activity) - PACKS_ARITH_ROUND) / (activity + PACKS_BAND_PROBABILITY_BIAS) + 1;
    if (probability > PACKS_BAND_PROBABILITY_MAX)
        probability = PACKS_BAND_PROBABILITY_MAX;
    return probability;
}

// Decodes one band of width x height coefficients from `data` into `output` (rows `stride` apart). An empty band is
// filled with zeros. Otherwise the arithmetic decoder is started (the first g_arithValueBits bits of the data become
// its value) and the cells of each 16 x 16 block are decoded in scan order; the probability of each cell follows an
// activity that mixes the previous activity with the band's starting activity and, after a non-zero coefficient, grows
// with its magnitude. Called by the plane decoders.
extern "C" void Packs_DecodeBand(const unsigned char *data, short *output, int width, int height, int stride,
                                 const PacksBandHeader *band)
{
    unsigned int scale = band->scale;
    if (scale == 0)
    {
        for (int row = 0; row < height; row++)
        {
            for (int column = 0; column < width; column++)
                output[column] = 0;
            output += stride;
        }
        return;
    }
    unsigned char gain = band->gain;
    int otherGain = PACKS_BAND_WEIGHT_ONE - gain;
    int keepWeight = (PACKS_BAND_WEIGHT_ONE - band->keep) * otherGain;
    int mixWeight = otherGain * band->keep;
    g_arithBitsRead = 0;
    unsigned int startActivity = scale * scale * scale * scale;
    unsigned int startLow = startActivity & 0xffff;
    unsigned int startHigh = startActivity >> PACKS_ARITH_SHIFT;
    unsigned int startShare = ((startLow * keepWeight + PACKS_ARITH_ROUND) >> PACKS_ARITH_SHIFT) + startHigh * keepWeight;
    unsigned int gainStep = gain * PACKS_BAND_WEIGHT_ONE;
    unsigned int activity = startActivity;
    int startPosition = g_packsReadPosition;
    g_packsReadMask = PACKS_FIRST_BIT;
    g_packsReadByte = data[g_packsReadPosition];
    unsigned int value = 0;
    g_arithValue = value;
    for (int bit = 0; bit < g_arithValueBits; bit++)
    {
        unsigned int nextBit = (g_packsReadByte & g_packsReadMask) != 0;
        if (g_packsReadMask != PACKS_LAST_BIT)
            g_packsReadMask *= 2;
        else
        {
            g_packsReadPosition++;
            g_packsReadMask = PACKS_FIRST_BIT;
            g_packsReadByte = data[g_packsReadPosition];
        }
        value = nextBit + value * 2;
        g_arithValue = value;
    }
    g_arithHigh = g_arithMax;
    g_arithLow = 0;
    unsigned int probability = ProbabilityOfActivity(activity);
    for (int blockRow = 0; blockRow < height; blockRow += PACKS_BAND_BLOCK)
    {
        for (int blockColumn = 0; blockColumn < width; blockColumn += PACKS_BAND_BLOCK)
        {
            for (int cell = 0; cell < PACKS_BAND_CELLS; cell++)
            {
                int row;
                if (g_packsCellX[cell] + blockColumn < width && (row = blockRow + g_packsCellY[cell]) < height)
                {
                    short coefficient = (short)Packs_ArithDecode(data, probability);
                    output[row * stride + g_packsCellX[cell] + blockColumn] = coefficient;
                    if (coefficient < 0)
                        coefficient = -coefficient;
                    activity = (activity >> PACKS_ARITH_SHIFT) * mixWeight +
                               (((activity & 0xffff) * mixWeight + PACKS_ARITH_ROUND) >> PACKS_ARITH_SHIFT) + startShare;
                    if (coefficient != 0)
                        activity += coefficient * gainStep;
                    if (activity == 0)
                        activity = 1;
                    probability = ProbabilityOfActivity(activity);
                }
            }
        }
    }
    g_packsReadPosition = ((unsigned int)(g_arithValueBits + PACKS_BITS_PER_BYTE + g_arithBitsRead) >> PACKS_BYTE_SHIFT) + startPosition;
}
