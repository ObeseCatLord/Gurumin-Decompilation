// Drawing of the text being entered (EN build, EN-only: no JP twin found).
#include "ascii_input.h"
#include "menu_list.h"

// Depth, alpha and scale the segments are drawn with.
const float INPUT_TEXT_DEPTH = 0.0f;
const float INPUT_TEXT_ALPHA = 1.0f;
const float INPUT_TEXT_SCALE = 1.0f;

// TextInput_DrawSegments: draws the three segments of the text being entered one after the other from (x, y). While
// converting the second segment uses `secondColour` and the third `thirdColour`; otherwise the second uses
// `thirdColour` and the third `secondColour` when a clause is picked (else `thirdColour`). Returns 0 while text input
// is inactive, else 1. No direct caller recovered.
extern "C" int TextInput_DrawSegments(FontRenderer *renderer, int x, int y, int firstColour, int secondColour,
                                      int thirdColour)
{
    if (g_textInputActive == 0)
        return 0;
    int firstWidth = renderer->DrawText(x, y, g_asciiInputInitial, firstColour, INPUT_TEXT_DEPTH, INPUT_TEXT_ALPHA,
                                        INPUT_TEXT_SCALE);
    if (g_textInputConverting != 0)
    {
        int secondWidth = renderer->DrawText(firstWidth + x, y, g_inputSegmentSecond, secondColour, INPUT_TEXT_DEPTH,
                                             INPUT_TEXT_ALPHA, INPUT_TEXT_SCALE);
        renderer->DrawText(secondWidth + firstWidth + x, y, g_inputSegmentThird, thirdColour, INPUT_TEXT_DEPTH,
                           INPUT_TEXT_ALPHA, INPUT_TEXT_SCALE);
        return 1;
    }
    firstWidth += renderer->DrawText(firstWidth + x, y, g_inputSegmentSecond, thirdColour, INPUT_TEXT_DEPTH,
                                     INPUT_TEXT_ALPHA, INPUT_TEXT_SCALE);
    if (g_textInputSelecting != 0)
        renderer->DrawText(firstWidth + x, y, g_inputSegmentThird, secondColour, INPUT_TEXT_DEPTH, INPUT_TEXT_ALPHA,
                           INPUT_TEXT_SCALE);
    else
        renderer->DrawText(firstWidth + x, y, g_inputSegmentThird, thirdColour, INPUT_TEXT_DEPTH, INPUT_TEXT_ALPHA,
                           INPUT_TEXT_SCALE);
    return 1;
}
