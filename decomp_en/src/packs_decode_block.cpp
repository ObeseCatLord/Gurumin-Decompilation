// Block decoder of the Packs LZ unpacker (EN build; JP twin: Packs.dll 100063c0, unmatched).
#include "packs_stream_bit.h"

// Bytes the bit reader skips at the start of a block, and the byte the first 16 bits are taken from (its second byte).
const int PACKS_BLOCK_HEADER_BYTES = 2;
const int PACKS_BLOCK_FIRST_COUNT = 8;
// The reader has used this many bytes of packed data before the first flag bit is read.
const int PACKS_BLOCK_START_BYTES = 4;
// Bits of a long distance's high part, and the distance value that introduces a run fill (0 ends the block).
const int PACKS_DISTANCE_HIGH_BITS = 5;
const unsigned int PACKS_DISTANCE_END = 0;
const unsigned int PACKS_DISTANCE_FILL = 1;
// Run fill: bits of the length (four more when a second length byte follows) and bytes added to it.
const int PACKS_FILL_LENGTH_BITS = 4;
const int PACKS_FILL_MIN_LENGTH = 0xe;
// Copy lengths: 2 to 5 are coded by flag bits, then PACKS_COPY_LONG_BASE plus three bits, otherwise a byte plus
// PACKS_COPY_BYTE_BASE.
const int PACKS_COPY_LONG_BITS = 3;
const int PACKS_COPY_LONG_BASE = 6;
const int PACKS_COPY_BYTE_BASE = 0xe;

// Decodes one block of the packed stream (g_packsStream) into the window. The block is a sequence of flag bits: 0 = copy
// one literal byte; 1 then 0 = copy from a short distance (one byte); 1 then 1 = a long distance (five bits and a byte):
// distance 0 ends the block (returns 1), distance 1 is a fill with one repeated byte, anything else is a copy. A copy's
// length comes from further flag bits. Returns 0 when a copy would run past the unpacked size. Called by
// Packs_UnpackBlocks.
extern "C" int Packs_DecodeBlock()
{
    g_packsWordReader.inputBytes = PACKS_BLOCK_START_BYTES;
    g_packsWordReader.buffer = g_packsStream->input[1];
    g_packsWordReader.count = PACKS_BLOCK_FIRST_COUNT;
    g_packsStream->input += PACKS_BLOCK_HEADER_BYTES;
    for (;;)
    {
        if (Packs_NextStreamBit() == 0)
        {
            *g_packsStream->window = *g_packsStream->input;
            g_packsStream->window++;
            g_packsStream->input++;
            g_packsStream->decodedLength++;
            g_packsWordReader.inputBytes++;
            continue;
        }
        unsigned int distance;
        if (Packs_NextStreamBit() == 0)
        {
            distance = *g_packsStream->input;
            g_packsStream->input++;
            g_packsWordReader.inputBytes++;
        }
        else
        {
            distance = Packs_ReadStreamBits(PACKS_DISTANCE_HIGH_BITS) * 0x100;
            distance += *g_packsStream->input;
            g_packsStream->input++;
            if (distance == PACKS_DISTANCE_END)
                return 1;
            if (distance == PACKS_DISTANCE_FILL)
            {
                unsigned int extra;
                int bytesUsed;
                if (Packs_ReadStreamBit() == 0)
                {
                    extra = Packs_ReadStreamBits(PACKS_FILL_LENGTH_BITS);
                    bytesUsed = g_packsWordReader.inputBytes + 0xf;
                }
                else
                {
                    extra = Packs_ReadStreamBits(PACKS_FILL_LENGTH_BITS) * 0x100;
                    extra += *g_packsStream->input;
                    g_packsStream->input++;
                    bytesUsed = g_packsWordReader.inputBytes + 0x10;
                }
                g_packsWordReader.inputBytes = bytesUsed + extra;
                unsigned char fill = *g_packsStream->input;
                g_packsStream->input++;
                for (int remaining = extra + PACKS_FILL_MIN_LENGTH; remaining != 0; remaining--)
                {
                    *g_packsStream->window = fill;
                    g_packsStream->window++;
                }
                g_packsStream->decodedLength += extra + PACKS_FILL_MIN_LENGTH;
                continue;
            }
        }

        int length;
        if (Packs_NextStreamBit() != 0)
            length = 2;
        else if (Packs_NextStreamBit() != 0)
            length = 3;
        else if (Packs_ReadStreamBit() != 0)
            length = 4;
        else if (Packs_ReadStreamBit() != 0)
            length = 5;
        else if (Packs_ReadStreamBit() == 0)
        {
            length = *g_packsStream->input;
            g_packsStream->input++;
            length += PACKS_COPY_BYTE_BASE;
            g_packsWordReader.inputBytes++;
        }
        else
            length = Packs_ReadStreamBits(PACKS_COPY_LONG_BITS) + PACKS_COPY_LONG_BASE;
        const unsigned char *source = g_packsStream->window - distance;
        if (g_packsUnpackedSize < (unsigned int)(g_packsStream->decodedLength + length))
            return 0;
        if (length != 0)
        {
            int remaining = length;
            do
            {
                *g_packsStream->window = *source++;
                g_packsStream->window++;
                remaining--;
            } while (remaining != 0);
        }
        g_packsStream->decodedLength += length;
    }
}
