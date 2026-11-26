// Freeing an index or sphere list without a result (EN build, EN-only: no JP twin found).
#include "index_list.h"
#include "engine_memory.h"

// Frees the buffer and empties the list. A void twin of FreeIndices. No direct caller.
void IndexList::FreeEntries()
{
    if (entries != 0)
        g_engineFree(entries);
    entries = 0;
    count = 0;
}
