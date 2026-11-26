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

// Draws the visible candidates one under the other, starting at (x, y) with CANDIDATE_LINE_HEIGHT between lines; the
// selected candidate is drawn in `selectedColour`, the others in `colour`. Does nothing (returns 0) while text input is
// inactive. Called by the name entry screens. EN 0x7ad1a0 (JP 0x1015fb20).
extern "C" int ImeCandidates_Draw(FontRenderer *renderer, int x, int y, int colour, int selectedColour)
{
    if (g_textInputActive == 0)
        return 0;
    if (g_candidateLines[0].text[0] != 0)
    {
        const char *line = g_candidateLines[0].text;
        int i = 0;
        while (true)
        {
            int remaining = g_candidateTotal - g_candidatePage * g_candidatePageSize - g_candidateFirstOffset;
            int visible = CANDIDATES_PER_PAGE;
            if (remaining < CANDIDATES_PER_PAGE)
                visible = remaining;
            if (i >= visible)
                break;
            if (g_candidateFirstOffset + g_candidatePage * g_candidatePageSize + i != g_candidateSelected)
                renderer->DrawText(x, y, line, colour, 0, 1.0f, 1.0f);
            else
                renderer->DrawText(x, y, line, selectedColour, 0, 1.0f, 1.0f);
            i++;
            line += CANDIDATE_TEXT_SIZE;
            y += CANDIDATE_LINE_HEIGHT;
        }
    }
    return 1;
}
