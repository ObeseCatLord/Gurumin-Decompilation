// The IME candidate list of the EN build (the lines shown while typing a name). Declared once here
// (JP: decomp/include/ui_vars.h).
#ifndef GURUMIN_EN_MENU_LIST_H
#define GURUMIN_EN_MENU_LIST_H

#include "font_renderer.h"

// Candidates shown on one page at most.
const int CANDIDATES_PER_PAGE = 9;
// Bytes of one candidate line.
const int CANDIDATE_TEXT_SIZE = 0x80;
// Scale at which candidate text is measured and drawn.
const float CANDIDATE_TEXT_SCALE = 1.0f;

// One candidate line.
struct CandidateLine
{
    char text[CANDIDATE_TEXT_SIZE];   // 0x00: the candidate string
};

extern "C" int g_candidateFirstOffset;        // EN 0x02228084: index of the first candidate of the list
extern "C" int g_candidateTotal;              // EN 0x02228088 (JP DAT_10f66aac): candidates in the list
extern "C" int g_candidatePageSize;           // EN 0x0222808c (JP DAT_10f66ab0): candidates per page step
extern "C" int g_candidatePage;               // EN 0x02228090 (JP DAT_10f66de8): current page
extern "C" CandidateLine g_candidateLines[];  // EN 0x02227050 (JP DAT_10f67008): the candidate strings of the page

// Width in pixels of the widest visible candidate. EN 0x7ad3b0 (JP 0x1015fac0).
extern "C" int ImeCandidates_WidestWidth(FontRenderer *renderer);

#endif
