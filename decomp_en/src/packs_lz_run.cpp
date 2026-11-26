// Run-length probe of the Packs LZ compressor (EN build, EN-only: no JP twin found).
#include "packs_codec.h"

// Packs_LzCountRun: counts how many bytes from PacksLzState::data on equal the first one, stopping at the end of the data
// and at PACKS_LZ_RUN_MAX (a longer run is cut to it). Returns 0 when nothing is left. Called by the compressor at
// EN 0x7bd240.
extern "C" unsigned int Packs_LzCountRun(void)
{
    unsigned int run = 0;
    const char *cursor = (const char *)g_packsLzState->data;
    unsigned int available = g_packsLzState->length - g_packsLzPosition;
    char first = *cursor;
    if (available >= PACKS_LZ_RUN_MAX + 1)
        available = PACKS_LZ_RUN_MAX;
    for (; run < available; run++)
    {
        if (*cursor++ != first)
            break;
    }
    return run;
}

// Packs_LzMatchLength: the length of the match between the bytes at `candidate` of the input and the bytes at
// PacksLzState::data. Counting starts at PACKS_LZ_HASHED_BYTES (the hash guarantees the first two) and stops at the first
// differing byte, at `maxLength` or when the candidate's position passes the end of the input. Called by the compressor.
extern "C" unsigned int Packs_LzMatchLength(unsigned short candidate, unsigned short maxLength)
{
    unsigned int length = PACKS_LZ_HASHED_BYTES;
    const unsigned char *earlier = g_packsLzBlockStart + candidate;
    if (length < maxLength)
    {
        do
        {
            if (g_packsLzPosition + length >= g_packsLzState->length)
                break;
            if (earlier[length] != g_packsLzState->data[length])
                break;
            length++;
        } while (length < maxLength);
    }
    return length;
}

// ---- Token loop of the compressor (EN 0x7bd240; JP twin Packs.dll 10006a40, medium confidence) ----
// Writes the compressed form that Packs_DecodeBlock reads: a flag bit per token, literal bytes, and matches as a distance,
// a length code and the byte after the match.

// Mask of the first flag bit of the first flag word of a block (the loop shifts it before each use).
const unsigned short PACKS_LZ_FIRST_FLAG_MASK = 0x80;
// Runs of equal bytes shorter than this are not encoded as runs; the search looks for ordinary matches instead.
const unsigned int PACKS_LZ_RUN_MIN = 0xe;
// A match search is only made while the best match so far is shorter than this.
const unsigned int PACKS_LZ_SEARCH_LIMIT = 0x40;
// The search needs at least this many bytes left in the input.
const unsigned int PACKS_LZ_SEARCH_TAIL = 4;
// Longest ordinary match (the comparison loop stops at this length).
const unsigned int PACKS_LZ_MATCH_MAX = 0x10d;
// Shortest ordinary match worth encoding.
const unsigned int PACKS_LZ_MATCH_MIN = 2;
// A distance below this fits one byte (flag zero); a larger one takes the high bits first (flag one).
const unsigned short PACKS_LZ_NEAR_DISTANCE_LIMIT = 0x100;
// Match lengths 2 to 5 are coded as a unary run of flags; longer ones with a four-bit prefix.
const unsigned int PACKS_LZ_UNARY_MAX = 5;
// Matches shorter than this have their length coded in three bits after the prefix, longer ones in a byte.
const unsigned int PACKS_LZ_SHORT_LENGTH_LIMIT = 0xe;
// Offset subtracted from a match length before it is stored in the length code.
const unsigned int PACKS_LZ_LENGTH_BIAS_SHORT = 6;
const unsigned int PACKS_LZ_LENGTH_BIAS_LONG = 0xe;
// Flag prefixes (bits written with Packs_LzWriteLowBits) of the run token and its sizes.
const unsigned char PACKS_LZ_RUN_PREFIX = 0x20;
const int PACKS_LZ_RUN_PREFIX_BITS = 6;
const unsigned char PACKS_LZ_RUN_TOKEN = 1;
const unsigned int PACKS_LZ_RUN_SMALL_LIMIT = 0x10;
const int PACKS_LZ_RUN_LENGTH_BITS = 4;
// The token that ends the stream: prefix 0x60 in seven flag bits, then a zero byte.
const unsigned char PACKS_LZ_END_PREFIX = 0x60;
const int PACKS_LZ_END_PREFIX_BITS = 7;
// Bits of the high part of a far distance.
const int PACKS_LZ_FAR_DISTANCE_BITS = 5;
// Bits of the prefix of a long match.
const int PACKS_LZ_LONG_PREFIX_BITS = 4;
const int PACKS_LZ_SHORT_LENGTH_BITS = 3;

// Starts the next flag bit of the flag word (reserving a new word at the output when the 16 bits are used) and clears it.
// The compressor expands this inline where the original did; it is the body of Packs_LzWriteZeroFlag.
static inline void PutZeroFlag()
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

// The same for a set flag bit (the body of Packs_LzWriteOneFlag).
static inline void PutOneFlag()
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

// Stores one byte at the output and counts it.
static inline void PutByte(unsigned char value)
{
    *g_packsLzState->outputByte = value;
    g_packsLzState->outputByte++;
    g_packsLzState->outputBytes++;
}

// Compresses the whole input of g_packsLzState token by token. At each position the longest run of equal bytes (at least
// PACKS_LZ_RUN_MIN) or, failing that, the longest earlier match found through the hash chain is coded; a position with
// neither is a literal byte. The bytes a token covers are then indexed into the hash chains and the progress callback is
// called. The last token written is the end marker. Called by Packs_LzEncode.
extern "C" void Packs_LzEncodeTokens(void)
{
    g_packsFlagWriter.word = g_packsLzState->outputWord;
    *g_packsFlagWriter.word = 0;
    g_packsFlagWriter.mask = PACKS_LZ_FIRST_FLAG_MASK;
    g_packsLzState->outputWord++;
    g_packsLzState->outputBytes += PACKS_WORD_BYTES;
    do
    {
        PacksLzState *state = g_packsLzState;
        unsigned int bestLength = Packs_LzCountRun();
        g_packsLzMatchPosition = PACKS_LZ_NO_ENTRY;
        if (bestLength < PACKS_LZ_RUN_MIN)
            bestLength = 0;
        g_packsLzPending = bestLength;
        g_packsLzNextByte = *state->data;
        if (bestLength < PACKS_LZ_SEARCH_LIMIT && g_packsLzPosition <= state->length - PACKS_LZ_SEARCH_TAIL)
        {
            unsigned short candidate = state->chainTail[*(unsigned short *)state->data];
            while (candidate != PACKS_LZ_NO_ENTRY)
            {
                const unsigned char *earlier = g_packsLzBlockStart + candidate;
                unsigned int end = state->length;
                const unsigned char *current = state->data;
                unsigned int position = g_packsLzPosition;
                unsigned int matched = PACKS_LZ_MATCH_MIN;
                do
                {
                    if (position + matched >= end)
                        break;
                    if (earlier[matched] != current[matched])
                        break;
                    matched++;
                } while (matched < PACKS_LZ_MATCH_MAX);
                unsigned short length = matched;
                state = g_packsLzState;
                if (length >= PACKS_LZ_MATCH_MIN && length >= bestLength)
                {
                    bestLength = length;
                    g_packsLzMatchPosition = candidate;
                    g_packsLzPending = bestLength;
                }
                candidate = state->links[candidate & PACKS_LZ_WINDOW_MASK];
            }
        }
        if (bestLength == 0)
        {
            // literal: flag zero, then the byte itself
            PutZeroFlag();
            *g_packsLzState->outputByte = *g_packsLzState->data;
            g_packsLzState->outputByte++;
            g_packsLzState->outputBytes++;
            g_packsLzPending = 1;
        }
        else
        {
            PutOneFlag();
            if (g_packsLzMatchPosition == PACKS_LZ_NO_ENTRY)
            {
                // run of equal bytes: prefix, length, then the repeated byte
                Packs_LzWriteLowBits(PACKS_LZ_RUN_PREFIX, PACKS_LZ_RUN_PREFIX_BITS);
                *g_packsLzState->outputByte = PACKS_LZ_RUN_TOKEN;
                g_packsLzState->outputByte++;
                unsigned short runLength = g_packsLzPending - PACKS_LZ_LENGTH_BIAS_LONG;
                if (runLength < PACKS_LZ_RUN_SMALL_LIMIT)
                {
                    PutZeroFlag();
                    Packs_LzWriteLowBits(runLength, PACKS_LZ_RUN_LENGTH_BITS);
                }
                else
                {
                    Packs_LzWriteOneFlag();
                    Packs_LzWriteLowBits(runLength >> 8, PACKS_LZ_RUN_LENGTH_BITS);
                    PutByte((unsigned char)runLength);
                }
                *g_packsLzState->outputByte = (unsigned char)g_packsLzNextByte;
                g_packsLzState->outputByte++;
                g_packsLzState->outputBytes += 2;
            }
            else
            {
                // match: distance (one byte when near, else its high bits first), then the length code
                unsigned short distance = g_packsLzPosition - g_packsLzMatchPosition;
                if (distance < PACKS_LZ_NEAR_DISTANCE_LIMIT)
                    PutZeroFlag();
                else
                {
                    Packs_LzWriteOneFlag();
                    Packs_LzWriteLowBits(distance >> 8, PACKS_LZ_FAR_DISTANCE_BITS);
                }
                PutByte((unsigned char)distance);
                switch (g_packsLzPending)
                {
                case 5:
                    Packs_LzWriteZeroFlag();
                case 4:
                    PutZeroFlag();
                case 3:
                    PutZeroFlag();
                case 2:
                    Packs_LzWriteOneFlag();
                    break;
                default:
                    Packs_LzWriteLowBits(0, PACKS_LZ_LONG_PREFIX_BITS);
                    if (g_packsLzPending < (int)PACKS_LZ_SHORT_LENGTH_LIMIT)
                    {
                        Packs_LzWriteOneFlag();
                        Packs_LzWriteLowBits((unsigned char)(g_packsLzPending - PACKS_LZ_LENGTH_BIAS_SHORT),
                                             PACKS_LZ_SHORT_LENGTH_BITS);
                    }
                    else
                    {
                        PutZeroFlag();
                        PutByte((unsigned char)(g_packsLzPending - PACKS_LZ_LENGTH_BIAS_LONG));
                    }
                }
            }
        }
        Packs_LzIndexBytes();
        if (g_packsLzState->progress != 0)
            g_packsLzState->progress(g_packsLzState);
    } while ((unsigned int)g_packsLzPosition < (unsigned int)g_packsLzState->length);
    Packs_LzWriteLowBits(PACKS_LZ_END_PREFIX, PACKS_LZ_END_PREFIX_BITS);
    PutByte(0);
}

// Packs_LzEncode: compresses one block. Makes `state` the current compressor state, resets the match search (position,
// pending count, best match), reserves the first flag word in the output and runs the token loop. Returns 0. Called by
// Packs_Compress for every block.
extern "C" int Packs_LzEncode(PacksLzState *state)
{
    g_packsLzState = state;
    g_packsLzBlockStart = state->data;
    g_packsLzSpareWord = g_packsLzPending = g_packsLzNextByte = 0;
    g_packsLzPosition = g_packsLzSpare = 0;
    g_packsLzMatchPosition = PACKS_LZ_NO_ENTRY;
    state->outputWord++;
    g_packsLzState->outputBytes += PACKS_WORD_BYTES;
    Packs_LzEncodeTokens();
    return 0;
}
