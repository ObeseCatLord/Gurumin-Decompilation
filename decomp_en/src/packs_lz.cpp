// LZ unpacker and compressor hash chains of the Packs codec (EN build; JP twins Packs.dll 100070e0, 100066d0, 10006950).
#include <string.h>

#include "packs_codec.h"

// Unpacks a packed stream into `output`: checks the header (size not 0 and at most PACKS_MAX_UNPACKED_SIZE), sets up the
// block decoder and runs it. Returns the unpacked size, or 0 when the header is invalid or decoding failed. Called when
// an archive entry is loaded.
extern "C" unsigned int Packs_Unpack(const PacksPackedHeader *packed, unsigned char *output, int packedLength)
{
    g_packsAbort = 0;
    unsigned int size = packed->unpackedSize;
    PacksUnpackStream stream;
    if (output == 0)
        return 0;
    stream.inputLength = packedLength;
    if (size == 0 || size > PACKS_MAX_UNPACKED_SIZE)
        return 0;
    g_packsUnpackedSize = size;
    g_packsWindowStart = g_packsWindow;
    g_packsChecksumSeed = packed->checksumSeed;
    stream.input = (unsigned char *)packed->blocks;
    stream.window = g_packsWindow;
    g_packsOutput = output;
    if (Packs_UnpackBlocks(&stream) == PACKS_BLOCK_ERROR)
        size = 0;
    return size;
}

// Decodes the blocks of the packed stream one after the other: each block starts with a 16-bit length field, is decoded
// into the window by Packs_DecodeBlock, reported to the progress callback and copied to the output buffer; the byte after
// a block says whether more follow. Returns 1 when the packed data is used up, 0 when the callback aborted or the last
// block was reached, PACKS_BLOCK_ERROR when a block is bad. Called by Packs_Unpack.
extern "C" int Packs_UnpackBlocks(PacksUnpackStream *stream)
{
    unsigned int consumed = 0;
    g_packsStream = stream;
    stream->decodedLength = 0;
    if (g_packsStream->inputLength > 0)
    {
        unsigned int progressBase = 0;
        do
        {
            g_packsStream->blockLength = *(unsigned short *)g_packsStream->input;
            consumed += g_packsStream->blockLength;
            if (consumed >= g_packsStream->inputLength)
                return 1;
            g_packsStream->input += 2;
            if (Packs_DecodeBlock() == 0)
                return PACKS_BLOCK_ERROR;
            int decoded = g_packsStream->decodedLength;
            if (g_packsProgress != 0)
            {
                g_packsAbort = g_packsProgress(g_packsProgressContext, decoded,
                                               (g_packsProgressStep * PACKS_PROGRESS_STEP_UNIT +
                                                PACKS_PROGRESS_STEP_BYTES + progressBase) /
                                                   (g_packsProgressWeight * PACKS_PROGRESS_WEIGHT_UNIT +
                                                    g_packsChecksumSeed * PACKS_PROGRESS_SIZE_UNIT));
            }
            if (g_packsAbort != 0)
                return 0;
            memcpy(g_packsOutput, g_packsWindowStart, decoded);
            g_packsOutput += decoded;
            consumed++;
            progressBase += PACKS_PROGRESS_STEP_BYTES;
            g_packsStream->window = g_packsWindowStart;
            g_packsStream->decodedLength = 0;
            unsigned char more = *g_packsStream->input;
            g_packsStream->input++;
            if (more == 0)
                return 0;
        } while (consumed < g_packsStream->inputLength);
    }
    return 1;
}

// Adds the next g_packsLzPending bytes to the hash chains of the compressor. The chain of a pair of bytes is found by the
// 16-bit value of the pair; once a position is more than PACKS_LZ_REACH bytes behind, its entry is unlinked first.
// Called by the compressor's match search.
extern "C" void Packs_LzIndexBytes()
{
    do
    {
        if (g_packsLzPosition >= PACKS_LZ_REACH)
        {
            unsigned int oldHash = *(unsigned short *)(g_packsLzState->data - PACKS_LZ_REACH);
            unsigned short next = g_packsLzState->links[(g_packsLzPosition - PACKS_LZ_REACH) & PACKS_LZ_WINDOW_MASK];
            g_packsLzState->chainTail[oldHash] = next;
            if (next == PACKS_LZ_NO_ENTRY)
                g_packsLzState->chainHead[oldHash] = PACKS_LZ_NO_ENTRY;
        }
        unsigned short hash = *(unsigned short *)g_packsLzState->data;
        unsigned short previous = g_packsLzState->chainHead[hash];
        g_packsLzState->chainHead[hash] = g_packsLzPosition;
        if (previous != PACKS_LZ_NO_ENTRY)
            g_packsLzState->links[previous & PACKS_LZ_WINDOW_MASK] = g_packsLzPosition;
        else
            g_packsLzState->chainTail[hash] = g_packsLzPosition;
        g_packsLzState->links[g_packsLzPosition & PACKS_LZ_WINDOW_MASK] = PACKS_LZ_NO_ENTRY;
        g_packsLzState->data++;
        g_packsLzPosition++;
        g_packsLzPending--;
    } while (g_packsLzPending != 0);
}
