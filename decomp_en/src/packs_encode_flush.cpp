// End of an arithmetic-coded stream in the Packs codec (EN build, EN-only: no JP twin found).
#include "packs_codec.h"

// Adds `bit` to the byte being built (the inline form of Packs_WriteBit); when the mask passes bit 7 the byte is stored
// at the next position of `output` and a new byte is started.
static inline void AppendBit(int bit, unsigned char *output)
{
    if (bit != 0)
        g_packsWriteByte |= g_packsWriteMask;
    if (g_packsWriteMask != PACKS_LAST_BIT)
    {
        g_packsWriteMask *= 2;
        return;
    }
    output[g_packsWritePosition] = g_packsWriteByte;
    g_packsWriteMask = PACKS_FIRST_BIT;
    g_packsWritePosition++;
    g_packsWriteByte = 0;
}

// Packs_EncodeFlush: ends the code stream. Writes `bit`, then every bit still pending from the interval shifts as the
// opposite of `bit` (which empties the pending count). Called by Packs_EncodeBand (EN 0x7bb630) and the bit encoder (EN
// 0x7b9400).
extern "C" void Packs_EncodeFlush(int bit, unsigned char *output)
{
    AppendBit(bit, output);
    while (g_encodePending != 0)
    {
        AppendBit(1 - bit, output);
        g_encodePending--;
    }
}
