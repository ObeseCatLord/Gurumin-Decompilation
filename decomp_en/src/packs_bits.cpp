// Bit coders of the Packs codec (EN build, EN-only: no JP twin found).
#include "packs_stream_bit.h"

// Returns the next bit of `data`. The reader keeps the byte being read, a mask that walks from bit 0 to bit 7 and the
// position of that byte; after bit 7 the next byte of `data` is loaded. Called by the decoders of the image codec.
extern "C" int Packs_ReadBit(const unsigned char *data)
{
    int bit = (g_packsReadByte & g_packsReadMask) != 0;
    if (g_packsReadMask != PACKS_LAST_BIT)
    {
        g_packsReadMask *= 2;
        return bit;
    }
    g_packsReadMask = PACKS_FIRST_BIT;
    g_packsReadPosition++;
    g_packsReadByte = data[g_packsReadPosition];
    return bit;
}

// Adds `bit` to the byte being built; when the mask passes bit 7 the byte is stored at the next position of `output` and
// a new byte is started. Called by the encoders of the image codec.
extern "C" void Packs_WriteBit(int bit, unsigned char *output)
{
    if (bit != 0)
        g_packsWriteByte |= g_packsWriteMask;
    if (g_packsWriteMask != PACKS_LAST_BIT)
    {
        g_packsWriteMask *= 2;
        return;
    }
    output[g_packsWritePosition] = g_packsWriteByte;
    g_packsWritePosition++;
    g_packsWriteByte = 0;
    g_packsWriteMask = PACKS_FIRST_BIT;
}

// Moves the compressor to the next flag bit and clears it. When all 16 bits of the flag word are used, a new flag word
// is reserved at the output (both the output pointer and the byte count advance by 2). Called by the compressor.
extern "C" void Packs_LzWriteZeroFlag(void)
{
    g_packsFlagWriter.mask <<= 1;
    if (g_packsFlagWriter.mask == 0)
    {
        g_packsFlagWriter.word = g_packsLzState->outputWord;
        g_packsFlagWriter.mask = 1;
        *g_packsFlagWriter.word = 0;
        g_packsLzState->outputWord++;
        g_packsLzState->outputBytes += PACKS_WORD_BYTES;
    }
    *g_packsFlagWriter.word &= ~g_packsFlagWriter.mask;
}

// Returns the next bit of the packed stream (the out-of-line copy of Packs_NextStreamBit). Called by Packs_DecodeBlock.
extern "C" unsigned short Packs_ReadStreamBit(void)
{
    return Packs_NextStreamBit();
}

// Moves the compressor to the next flag bit and sets it. When all 16 bits of the flag word are used, a new flag word is
// reserved at the output first (both the output pointer and the byte count advance by 2). Called by the compressor.
extern "C" void Packs_LzWriteOneFlag(void)
{
    g_packsFlagWriter.mask <<= 1;
    if (g_packsFlagWriter.mask == 0)
    {
        g_packsFlagWriter.word = g_packsLzState->outputWord;
        g_packsFlagWriter.mask = 1;
        *g_packsFlagWriter.word = 0;
        g_packsLzState->outputWord++;
        g_packsLzState->outputBytes += PACKS_WORD_BYTES;
    }
    *g_packsFlagWriter.word |= g_packsFlagWriter.mask;
}

// Reads `count` bits of the packed stream, the first one ending up highest, and returns them as a number. Called by
// Packs_DecodeBlock.
extern "C" unsigned short Packs_ReadStreamBits(int count)
{
    int value = 0;
    while (count != 0)
    {
        value = value * 2 + Packs_ReadStreamBit();
        count--;
    }
    return value;
}

// Writes the lowest `count` bits of `value` as compressor flags, highest of those bits first (bits at or above `count`
// are skipped, a set bit becomes a one flag and a clear bit a zero flag). Called by the compressor at EN 0x7bd240.
extern "C" void Packs_LzWriteLowBits(unsigned char value, int count)
{
    unsigned char mask = PACKS_LAST_BIT;
    for (int bitIndex = 7; bitIndex >= 0; bitIndex--, mask >>= 1)
    {
        if (bitIndex >= count)
            continue;
        if (value & mask)
            Packs_LzWriteOneFlag();
        else
            Packs_LzWriteZeroFlag();
    }
}
