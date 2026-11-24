// Outlined text: the text drawn four times, offset one pixel diagonally, as a darker outline, then on top (EN build).
// Ported from the matched JP source decomp/src/text_outline.cpp.
#include "font_renderer.h"

// Colour of the default outline (brown).
const int OUTLINE_COLOUR_BROWN = 0xa66821;
// The outline fades with alpha squared times this factor, so it fades out faster than the text.
const float OUTLINE_ALPHA_FACTOR = 0.8f;
// Pixels the outline copies are offset from the text.
const int OUTLINE_OFFSET = 1;

// Draws `text` with a brown outline when outlines are on. The outline's opacity is alpha * alpha * 0.8. Returns the
// result of the final draw. Called by the menus and the HUD.
int FontRenderer::DrawOutlined(int x, int y, const char *text, int colour, float depth, float alpha, float scale)
{
    if (g_textOutlineEnabled != 0)
    {
        float outlineAlpha = alpha * alpha * OUTLINE_ALPHA_FACTOR;
        DrawText(x - OUTLINE_OFFSET, y - OUTLINE_OFFSET, text, OUTLINE_COLOUR_BROWN, depth, outlineAlpha, scale);
        DrawText(x - OUTLINE_OFFSET, y + OUTLINE_OFFSET, text, OUTLINE_COLOUR_BROWN, depth, outlineAlpha, scale);
        DrawText(x + OUTLINE_OFFSET, y - OUTLINE_OFFSET, text, OUTLINE_COLOUR_BROWN, depth, outlineAlpha, scale);
        DrawText(x + OUTLINE_OFFSET, y + OUTLINE_OFFSET, text, OUTLINE_COLOUR_BROWN, depth, outlineAlpha, scale);
    }
    return DrawText(x, y, text, colour, depth, alpha, scale);
}

// Same as DrawOutlined with the outline in `outlineColour`.
int FontRenderer::DrawOutlinedIn(int x, int y, const char *text, int colour, float depth, float alpha, float scale,
                                 int outlineColour)
{
    if (g_textOutlineEnabled != 0)
    {
        float outlineAlpha = alpha * alpha * OUTLINE_ALPHA_FACTOR;
        DrawText(x - OUTLINE_OFFSET, y - OUTLINE_OFFSET, text, outlineColour, depth, outlineAlpha, scale);
        DrawText(x - OUTLINE_OFFSET, y + OUTLINE_OFFSET, text, outlineColour, depth, outlineAlpha, scale);
        DrawText(x + OUTLINE_OFFSET, y - OUTLINE_OFFSET, text, outlineColour, depth, outlineAlpha, scale);
        DrawText(x + OUTLINE_OFFSET, y + OUTLINE_OFFSET, text, outlineColour, depth, outlineAlpha, scale);
    }
    return DrawText(x, y, text, colour, depth, alpha, scale);
}
