// Outlined text: the text drawn four times, offset one pixel diagonally, as a darker outline, then on top.

struct TextRenderer
{
    // 0x101501f0: draws `text` at (x, y) in `colour` with opacity `alpha` (see menu_list.cpp).
    int M_101501f0(int x, int y, const char *text, int colour, int a, float alpha, int b);
    int DrawOutlined(int x, int y, const char *text, int colour, int a, float alpha, int b);
    int DrawOutlinedIn(int x, int y, const char *text, int colour, int a, float alpha, int b, int outlineColour);
};

extern "C" int DAT_10202b20;  // 1 = draw text outlines

// 0x101508d0. Draws `text` with a brown (0xa66821) outline when outlines are on. The outline's opacity is
// alpha * alpha * 0.8, so it fades out faster than the text. Returns the result of the final draw.
int TextRenderer::DrawOutlined(int x, int y, const char *text, int colour, int a, float alpha, int b)
{
    if (DAT_10202b20 != 0)
    {
        float outlineAlpha = alpha * alpha * 0.8f;
        M_101501f0(x - 1, y - 1, text, 0xa66821, a, outlineAlpha, b);
        M_101501f0(x - 1, y + 1, text, 0xa66821, a, outlineAlpha, b);
        M_101501f0(x + 1, y - 1, text, 0xa66821, a, outlineAlpha, b);
        M_101501f0(x + 1, y + 1, text, 0xa66821, a, outlineAlpha, b);
    }
    return M_101501f0(x, y, text, colour, a, alpha, b);
}

// 0x101509b0. Same as DrawOutlined with the outline in `outlineColour`.
int TextRenderer::DrawOutlinedIn(int x, int y, const char *text, int colour, int a, float alpha, int b,
                                 int outlineColour)
{
    if (DAT_10202b20 != 0)
    {
        float outlineAlpha = alpha * alpha * 0.8f;
        M_101501f0(x - 1, y - 1, text, outlineColour, a, outlineAlpha, b);
        M_101501f0(x - 1, y + 1, text, outlineColour, a, outlineAlpha, b);
        M_101501f0(x + 1, y - 1, text, outlineColour, a, outlineAlpha, b);
        M_101501f0(x + 1, y + 1, text, outlineColour, a, outlineAlpha, b);
    }
    return M_101501f0(x, y, text, colour, a, alpha, b);
}
