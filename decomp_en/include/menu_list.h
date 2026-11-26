// The IME candidate list of the EN build (the lines shown while typing a name). Declared once here
// (JP: decomp/include/ui_vars.h).
#ifndef GURUMIN_EN_MENU_LIST_H
#define GURUMIN_EN_MENU_LIST_H

#include "font_renderer.h"
#include "ime.h"

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

// Vertical distance between two candidate lines when they are drawn.
const int CANDIDATE_LINE_HEIGHT = 16;
extern "C" int g_candidateSelected;           // EN 0x02228094: index of the selected candidate in the whole list

// Width in pixels of the widest visible candidate. EN 0x7ad3b0 (JP 0x1015fac0).
extern "C" int ImeCandidates_WidestWidth(FontRenderer *renderer);

// Width in pixels of the widest candidate of the page measured with the Windows font at 0.8 scale (ime_candidate_width.cpp).
// EN 0x7ad340.
extern "C" int ImeCandidates_WidestWindowsWidth(FontRenderer *renderer);

// Number of candidates of the current page that are visible: the rest of the list after the pages before it, at most
// CANDIDATES_PER_PAGE. EN 0x7ad310 (ime_candidate_count.cpp). Called by the candidate list drawing and layout.
extern "C" int ImeCandidates_VisibleCount(void);

// Draws the visible candidates at (x, y), the selected one in `selectedColour`. EN 0x7ad1a0 (JP 0x1015fb20).
extern "C" int ImeCandidates_Draw(FontRenderer *renderer, int x, int y, int colour, int selectedColour);

// Bytes of one of the three text segments of the text being entered.
const int INPUT_SEGMENT_SIZE = 0x100;
// Texts of the second and third segment of the text being entered (the first is g_asciiInputInitial, EN 0x02226b50).
extern "C" char g_inputSegmentSecond[INPUT_SEGMENT_SIZE];   // EN 0x02226c50: second segment
extern "C" char g_inputSegmentThird[INPUT_SEGMENT_SIZE];    // EN 0x02226d50: third segment
extern "C" int g_textInputConverting;   // EN 0x02228070: non-zero while the composition is being converted (draw selects the colours of the segments)
extern "C" int g_textInputSelecting;    // EN 0x02226940: non-zero while a clause is picked (draw selects the colour of the third segment)

// Draws the three segments of the text being entered one after the other from (x, y): the first in `firstColour`, the
// second in `secondColour` while converting (else `thirdColour`) and the third in `secondColour` when a clause is
// picked (else `thirdColour`; while converting the third segment is always `thirdColour`). Returns 0 while text input is
// inactive, else 1. EN 0x7ace50 (EN-only, ime_text_draw.cpp).
extern "C" int TextInput_DrawSegments(FontRenderer *renderer, int x, int y, int firstColour, int secondColour,
                                      int thirdColour);

#endif
