// EN IME candidate list (JP twin: menu_list.cpp).

#include "menu_list.h"

// Number of candidates shown on the current page (at most CANDIDATES_PER_PAGE).
inline int VisibleLines()
{
    int remaining = g_candidateTotal - g_candidatePageSize * g_candidatePage - g_candidateFirstOffset;
    if (remaining >= CANDIDATES_PER_PAGE)
        remaining = CANDIDATES_PER_PAGE;
    return remaining;
}

// Returns the width in pixels of the widest visible candidate. Called when the candidate window is laid out.
extern "C" int ImeCandidates_WidestWidth(FontRenderer *renderer)
{
    int widest = 0;
    for (int i = 0; i < VisibleLines(); i++)
    {
        int width = renderer->TextWidth(g_candidateLines[i].text, CANDIDATE_TEXT_SCALE);
        if (widest < width)
            widest = width;
    }
    return widest;
}
