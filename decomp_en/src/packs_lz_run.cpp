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
