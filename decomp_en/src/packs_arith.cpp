// Arithmetic decoder of Packs.dll (EN build). Ported from the Ghidra output and the EN bytes (no JP source).
#include "packs_codec.h"

// Shifts the decoder state until the interval is wider than a quarter again: whenever the interval straddles the half
// it is kept when it also straddles a quarter, else it is moved down by a quarter; then low, high and the value are
// doubled (wrapping at the top) and the next bit of `data` is shifted into the value. Returns when the interval no
// longer needs a shift, and 1 when no shift was needed at all.
static inline int RenormaliseInterval(const unsigned char *data)
{
    while (true)
    {
        if (g_arithHigh >= g_arithHalf && g_arithLow < g_arithHalf)
        {
            if (g_arithLow < g_arithQuarter || g_arithHigh >= g_arithThreeQuarters)
                return 0;
            g_arithValue -= g_arithQuarter;
            g_arithLow -= g_arithQuarter;
            g_arithHigh -= g_arithQuarter;
        }
        g_arithLow = g_arithLow * 2;
        if (g_arithLow >= g_arithTop)
            g_arithLow -= g_arithTop;
        g_arithHigh = g_arithHigh * 2;
        if (g_arithHigh >= g_arithTop)
            g_arithHigh -= g_arithTop;
        g_arithValue = g_arithValue * 2;
        g_arithHigh++;
        if (g_arithValue >= g_arithTop)
            g_arithValue -= g_arithTop;
        if ((g_packsReadByte & g_packsReadMask) != 0)
            g_arithValue++;
        if (g_packsReadMask == PACKS_LAST_BIT)
        {
            g_packsReadPosition++;
            g_packsReadMask = PACKS_FIRST_BIT;
            g_packsReadByte = data[g_packsReadPosition];
        }
        else
            g_packsReadMask *= 2;
        g_arithBitsRead++;
    }
}

// Decodes one binary decision of probability `probability` (16 bits) from `data`. The interval is split in proportion
// to the probability; when the value lies below the split the decision is a 1 and the decoder counts it (the first
// decision of a symbol), else it moves the interval's high end down. A 0 decision first returns the high end with the
// low half cleared; after a counted decision the interval is split in halves to decide the sign, which is returned in
// the low half (negated for the upper half). Called by the image decoder for every coefficient.
extern "C" unsigned int Packs_ArithDecode(const unsigned char *data, unsigned int probability)
{
    short decisions = 0;
    bool taken;
    while (true)
    {
        unsigned int split = (((g_arithHigh - g_arithLow) + 1) * (probability & 0xffff) + PACKS_ARITH_ROUND >> PACKS_ARITH_SHIFT) + g_arithLow;
        if (g_arithValue < split)
        {
            g_arithHigh = split - 1;
            taken = false;
        }
        else
        {
            decisions++;
            taken = true;
            g_arithLow = split;
        }
        RenormaliseInterval(data);
        if (taken)
            continue;
        break;
    }
    if (decisions == 0)
        return g_arithHigh & 0xffff0000;
    unsigned int split = ((g_arithHigh - g_arithLow) + 2 >> 1) + g_arithLow;
    if (g_arithValue < split)
        g_arithHigh = split - 1;
    else
    {
        decisions = -decisions;
        g_arithLow = split;
    }
    RenormaliseInterval(data);
    return (g_arithHigh & 0xffff0000) | (unsigned short)decisions;
}
