// Bit coders of the Packs codec (EN build, EN-only: no JP twin found).
#include "packs_codec.h"

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

// Returns the next bit of the packed stream. The stream is read 16 bits at a time (low bit first): when none are left,
// the next word is taken from the stream's input. Called by Packs_DecodeBlock.
extern "C" int Packs_ReadStreamBit(void)
{
    if (g_packsWordReader.count == 0)
    {
        g_packsWordReader.buffer = *(unsigned short *)g_packsStream->input;
        g_packsWordReader.count = PACKS_WORD_BITS;
        g_packsStream->input += PACKS_WORD_BYTES;
        g_packsWordReader.inputBytes += PACKS_WORD_BYTES;
    }
    int bit = g_packsWordReader.buffer & 1;
    g_packsWordReader.count--;
    g_packsWordReader.buffer >>= 1;
    return bit;
}
