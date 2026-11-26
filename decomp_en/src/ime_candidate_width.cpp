// Width of the IME candidate list measured with the Windows font (EN build, EN-only: no JP twin found).
#include "menu_list.h"

// Scale at which this measure takes the candidate strings (smaller than the drawn size).
const float CANDIDATE_MEASURE_SCALE = 0.8f;

// Returns the width in pixels of the widest candidate of the current page, measuring each with
// FontRenderer::MeasureWindowsText at CANDIDATE_MEASURE_SCALE. Called by the text window layout (EN 0x7b18f0).
extern "C" int ImeCandidates_WidestWindowsWidth(FontRenderer *renderer)
{
    int widest = 0;
    int i = 0;
    while (true)
    {
        int remaining = g_candidateTotal - g_candidatePage * g_candidatePageSize - g_candidateFirstOffset;
        int visible = CANDIDATES_PER_PAGE;
        if (remaining < CANDIDATES_PER_PAGE)
            visible = remaining;
        if (i >= visible)
            break;
        int width;
        int height;
        renderer->MeasureWindowsText(g_candidateLines[i].text, CANDIDATE_MEASURE_SCALE, &width, &height);
        if (widest < width)
            widest = width;
        i++;
    }
    return widest;
}
