// Outlined text: the text drawn four times, offset one pixel diagonally, as a darker outline, then on top.

#include "font_renderer.h"

#include "ui_vars.h"

// 0x101508d0. Draws `text` with a brown (0xa66821) outline when outlines are on. The outline's opacity is
// alpha * alpha * 0.8, so it fades out faster than the text. Returns the result of the final draw.
int FontRenderer::DrawOutlined(int x, int y, const char *text, int colour, int a, float alpha, float scale)
{
    if (DAT_10202b20 != 0)
    {
        float outlineAlpha = alpha * alpha * 0.8f;
        DrawText(x - 1, y - 1, text, 0xa66821, a, outlineAlpha, scale);
        DrawText(x - 1, y + 1, text, 0xa66821, a, outlineAlpha, scale);
        DrawText(x + 1, y - 1, text, 0xa66821, a, outlineAlpha, scale);
        DrawText(x + 1, y + 1, text, 0xa66821, a, outlineAlpha, scale);
    }
    return DrawText(x, y, text, colour, a, alpha, scale);
}

// 0x101509b0. Same as DrawOutlined with the outline in `outlineColour`.
int FontRenderer::DrawOutlinedIn(int x, int y, const char *text, int colour, int a, float alpha, float scale,
                                 int outlineColour)
{
    if (DAT_10202b20 != 0)
    {
        float outlineAlpha = alpha * alpha * 0.8f;
        DrawText(x - 1, y - 1, text, outlineColour, a, outlineAlpha, scale);
        DrawText(x - 1, y + 1, text, outlineColour, a, outlineAlpha, scale);
        DrawText(x + 1, y - 1, text, outlineColour, a, outlineAlpha, scale);
        DrawText(x + 1, y + 1, text, outlineColour, a, outlineAlpha, scale);
    }
    return DrawText(x, y, text, colour, a, alpha, scale);
}
