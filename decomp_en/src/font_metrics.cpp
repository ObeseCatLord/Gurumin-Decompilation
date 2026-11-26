// Text measuring of the font renderer (EN build). Ported from the matched JP source decomp/src/font_metrics.cpp.
#include <string.h>

#include "font_renderer.h"
#include "render.h"

// FontRenderer::TextWidth: width in pixels of `text` drawn at `scale`: measured with the bitmap font when one is
// loaded (DrawString also returns the width, which is then divided by the font scale divisor), otherwise with the
// Windows font through GetTextExtentPoint32A. Called by the menus and the IME candidate list.
int FontRenderer::TextWidth(const char *text, float scale)
{
    if (glyphs == 0)
    {
        SIZE size;
        GetTextExtentPoint32A(dc, text, strlen(text), &size);
        return (int)((float)size.cx * scale);
    }
    return (int)((float)DrawString(0, 0, 0, text, 0, 0) * scale / g_screenWidthDivisor);
}

// FontRenderer::MeasureWindowsText: measures `text` with the Windows font through GetTextExtentPoint32A, cuts the width
// to FONT_MAX_MEASURED_WIDTH and stores width and height multiplied by `scale` in *width and *height. Returns 1. Called
// by the text window layout code.
int FontRenderer::MeasureWindowsText(const char *text, float scale, int *width, int *height)
{
    SIZE size;
    GetTextExtentPoint32A(dc, text, strlen(text), &size);
    if (size.cx > FONT_MAX_MEASURED_WIDTH)
        size.cx = FONT_MAX_MEASURED_WIDTH;
    *width = (int)((float)size.cx * scale);
    *height = (int)((float)size.cy * scale);
    return 1;
}
