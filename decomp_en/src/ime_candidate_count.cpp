// How many candidates of the IME list are on the current page (EN build, EN-only: no JP twin found).
#include "menu_list.h"

// Returns the candidates left in the list after the pages before the current one, but at most CANDIDATES_PER_PAGE.
// Called by the candidate list drawing (EN 0x7b18f0, 0x5ccac0, 0x5cf280).
extern "C" int ImeCandidates_VisibleCount(void)
{
    int remaining = g_candidateTotal - g_candidatePage * g_candidatePageSize - g_candidateFirstOffset;
    int visible = CANDIDATES_PER_PAGE;
    if (remaining < CANDIDATES_PER_PAGE)
        visible = remaining;
    return visible;
}
