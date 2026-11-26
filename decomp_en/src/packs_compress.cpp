// Compressor driver of Packs.dll (EN build). Ported from the Ghidra output and the EN bytes (no JP source).
#include <string.h>

#include "packs_codec.h"

// Byte the hash tables are cleared to (no entry).
const int PACKS_LZ_CLEAR_BYTE = 0xff;

// Compresses `length` bytes at `input` into `output`: the header holds the length and the number of blocks, then every
// block of up to PACKS_BLOCK_INPUT bytes is compressed with fresh hash tables and written as its 16-bit compressed size
// (counted in the data), the data and a flag that is 1 when more blocks follow. After each block the compressor's
// progress callback is called; when it returns non-zero compression stops. Returns the bytes written. Called by the
// packing tools of the game's resource builder.
extern "C" int Packs_Compress(const unsigned char *input, int *output, int length)
{
    int remaining = length;
    g_packsAbort = 0;
    *output = length;
    PacksLzState state;
    state.m_0014 = PACKS_LZ_STATE_START;
    state.length = 0;
    state.links = (unsigned short *)g_packsLzLinks;
    state.chainTail = (unsigned short *)g_packsLzChainTail;
    unsigned int blocksLeft = (length + PACKS_BLOCK_COUNT_BIAS) / PACKS_BLOCK_INPUT;
    output[1] = blocksLeft;
    int *out = output + 2;
    int total = PACKS_COMPRESSED_HEADER_BYTES;
    state.chainHead = (unsigned short *)g_packsLzChainHead;
    if (blocksLeft == 0)
        return PACKS_COMPRESSED_HEADER_BYTES;
    unsigned int blockCount = blocksLeft;
    while (true)
    {
        blocksLeft--;
        int rest;
        if (remaining >= (int)PACKS_BLOCK_INPUT)
        {
            memcpy(g_packsLzInput, input, PACKS_BLOCK_INPUT);
            state.length = PACKS_BLOCK_INPUT;
            input += PACKS_BLOCK_INPUT;
            rest = remaining - PACKS_BLOCK_INPUT;
        }
        else
        {
            memcpy(g_packsLzInput, input, remaining);
            input += remaining;
            rest = 0;
            state.length = remaining;
        }
        memset(g_packsLzLinks, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        memset(g_packsLzLinks + PACKS_LZ_CLEAR_PIECE, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        memset(g_packsLzChainTail, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        memset(g_packsLzChainTail + 1 * PACKS_LZ_CLEAR_PIECE, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        memset(g_packsLzChainTail + 2 * PACKS_LZ_CLEAR_PIECE, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        memset(g_packsLzChainTail + 3 * PACKS_LZ_CLEAR_PIECE, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        memset(g_packsLzChainHead, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        memset(g_packsLzChainHead + 1 * PACKS_LZ_CLEAR_PIECE, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        memset(g_packsLzChainHead + 2 * PACKS_LZ_CLEAR_PIECE, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        memset(g_packsLzChainHead + 3 * PACKS_LZ_CLEAR_PIECE, PACKS_LZ_CLEAR_BYTE, PACKS_LZ_CLEAR_PIECE);
        state.outputBytes = 0;
        state.outputWord = (unsigned short *)g_packsLzOutput;
        state.data = g_packsLzInput;
        Packs_LzEncode(&state);
        *(unsigned short *)g_packsLzOutput = (unsigned short)state.outputBytes;
        memcpy(out, g_packsLzOutput, state.outputBytes);
        total += state.outputBytes;
        *(bool *)((int)out + state.outputBytes) = blocksLeft != 0;
        out = (int *)((int)out + state.outputBytes + 1);
        state.outputWord = 0;
        PacksProgressCallback progress = g_packsCompressProgress;
        if (progress != 0)
            g_packsAbort = progress(g_packsProgressContext, state.outputBytes,
                                                   ((blockCount - blocksLeft) * PACKS_PROGRESS_BLOCK +
                                                    g_packsProgressStep * PACKS_PROGRESS_STEP) /
                                                       (g_packsProgressWeight * PACKS_PROGRESS_WEIGHT +
                                                        blockCount * PACKS_PROGRESS_PER_BLOCK));
        if (g_packsAbort != 0)
            break;
        total++;
        remaining = rest;
        if (blocksLeft == 0)
            return total;
    }
    return total;
}
