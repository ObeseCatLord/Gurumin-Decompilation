// Inline form of the packed-stream bit reader (EN 0x7bd190 is its out-of-line copy), shared by the files that decode
// packed streams. Declared once here.
#ifndef GURUMIN_EN_PACKS_STREAM_BIT_H
#define GURUMIN_EN_PACKS_STREAM_BIT_H

#include "packs_codec.h"

// Returns the next bit of the packed stream. The stream is read 16 bits at a time (low bit first): when none are left,
// the next word is taken from the stream's input.
inline unsigned short Packs_NextStreamBit()
{
    if (g_packsWordReader.count == 0)
    {
        g_packsWordReader.buffer = *(unsigned short *)g_packsStream->input;
        g_packsWordReader.count = PACKS_WORD_BITS;
        g_packsStream->input += PACKS_WORD_BYTES;
        g_packsWordReader.inputBytes += PACKS_WORD_BYTES;
    }
    unsigned short bit = g_packsWordReader.buffer & 1;
    g_packsWordReader.buffer >>= 1;
    g_packsWordReader.count--;
    return bit;
}

#endif
