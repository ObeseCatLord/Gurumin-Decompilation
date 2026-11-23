// IME candidate list layout.

#include "font_renderer.h"
#include "menu_text.h"

#include "ui_vars.h"

// Number of candidates shown on the current page (at most 9).
inline int VisibleLines()
{
    int remaining = DAT_10f66aac - DAT_10f66ab0 * DAT_10f66de8 - DAT_10f66dd0;
    if (remaining >= 9)
        remaining = 9;
    return remaining;
}

// Returns the number of visible candidates.
extern "C" int FUN_1015fa90(void)
{
    return VisibleLines();
}

// Returns the width in pixels of the widest visible candidate.
extern "C" int FUN_1015fac0(FontRenderer *renderer)
{
    int widest = 0;
    for (int i = 0; i < VisibleLines(); i++)
    {
        int width = renderer->TextWidth(DAT_10f67008[i].text, 1.0f);
        if (widest < width)
            widest = width;
    }
    return widest;
}


// Draws the visible candidates one below the other (16 pixels apart) at (x, y), the selected one in `selColour`
// and the others in `colour`. Returns 0 when text input is not active, else 1.
// Not byte-identical yet: the original keeps the counter on the stack and x/y in registers (see open.tsv).
extern "C" int FUN_1015fb20(FontRenderer *renderer, int x, int y, int colour, int selColour)
{
    if (DAT_10f67000 == 0)
        return 0;
    if (DAT_10f67008[0].text[0] != 0)
    {
        for (int i = 0; i < VisibleLines(); i++, y += 16)
            renderer->DrawText(x, y, DAT_10f67008[i].text,
                                 DAT_10f66ab0 * DAT_10f66de8 + DAT_10f66dd0 + i == DAT_10f68114 ? selColour : colour, 0,
                                 1.0f, 1.0f);
    }
    return 1;
}
