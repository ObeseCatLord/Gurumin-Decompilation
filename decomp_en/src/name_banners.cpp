// Name banners at the top of the screen (EN build). Ported from decomp/src/name_banners.cpp.
#include "dialogs.h"
#include "font_renderer.h"
#include "game.h"

// Fade-in steps until the banners are fully visible (alpha = counter / steps).
const float BANNER_FADE_STEPS = 15.0f;
// Banner panel: height, and width added to the text width (upper left banners / centred banner).
const int BANNER_PANEL_HEIGHT = 64;
const int BANNER_PANEL_PADDING = 40;
const int BANNER_PANEL_PADDING_CENTRED = 32;
// Upper left banner: panel position and text position.
const int BANNER_FIRST_PANEL_X = 32;
const int BANNER_FIRST_PANEL_Y = 10;
const int BANNER_FIRST_TEXT_X = 52;
const int BANNER_FIRST_TEXT_Y = 32;
// Second banner next to it: panel position and text position.
const int BANNER_SECOND_PANEL_X = 310;
const int BANNER_SECOND_TEXT_X = 330;
// Centred banner: x the panel and text centre on, and the panel and text y.
const int BANNER_CENTRE_PANEL_X = 304;
const int BANNER_CENTRE_PANEL_Y = 74;
const int BANNER_CENTRE_TEXT_X = 320;
const int BANNER_CENTRE_TEXT_Y = 96;
// Text colour, depth and scale of banner text.
const int BANNER_TEXT_COLOUR = 0xffffff;
const float BANNER_TEXT_DEPTH = 0.0f;
const float BANNER_TEXT_SCALE = 1.0f;
// Panel depth.
const float BANNER_PANEL_DEPTH = 0.0f;

// Hud_DrawNameBanners: with a first text, draws it at the upper left (panel at (32, 10)) and a second text next to it
// (panel at (310, 10)); with only the second text, draws that one centred at the top (panel centred on x = 304 at
// y = 74). Each panel is 40 pixels wider than its text (32 when centred) and 64 high. Called every frame by the HUD.
extern "C" void Hud_DrawNameBanners(void)
{
    if (g_bannerText[0] != 0)
    {
        int width = g_textRenderer.TextWidth(g_bannerText, BANNER_TEXT_SCALE);
        g_dialogGame.DrawPanelAlt(BANNER_FIRST_PANEL_X, BANNER_FIRST_PANEL_Y, width + BANNER_PANEL_PADDING,
                                  BANNER_PANEL_HEIGHT, BANNER_PANEL_DEPTH, (float)g_bannerFade / BANNER_FADE_STEPS);
        g_textRenderer.DrawOutlined(BANNER_FIRST_TEXT_X, BANNER_FIRST_TEXT_Y, g_bannerText, BANNER_TEXT_COLOUR,
                                    BANNER_TEXT_DEPTH, (float)g_bannerFade / BANNER_FADE_STEPS, BANNER_TEXT_SCALE);
        if (g_dialogMessage[0] != 0)
        {
            width = g_textRenderer.TextWidth(g_dialogMessage, BANNER_TEXT_SCALE);
            g_dialogGame.DrawPanelAlt(BANNER_SECOND_PANEL_X, BANNER_FIRST_PANEL_Y, width + BANNER_PANEL_PADDING,
                                      BANNER_PANEL_HEIGHT, BANNER_PANEL_DEPTH, (float)g_bannerFade / BANNER_FADE_STEPS);
            g_textRenderer.DrawOutlined(BANNER_SECOND_TEXT_X, BANNER_FIRST_TEXT_Y, g_dialogMessage, BANNER_TEXT_COLOUR,
                                        BANNER_TEXT_DEPTH, (float)g_bannerFade / BANNER_FADE_STEPS, BANNER_TEXT_SCALE);
        }
    }
    else if (g_dialogMessage[0] != 0)
    {
        int width = g_textRenderer.TextWidth(g_dialogMessage, BANNER_TEXT_SCALE);
        int halfWidth = width / 2;
        g_dialogGame.DrawPanelAlt(BANNER_CENTRE_PANEL_X - halfWidth, BANNER_CENTRE_PANEL_Y,
                                  width + BANNER_PANEL_PADDING_CENTRED, BANNER_PANEL_HEIGHT, BANNER_PANEL_DEPTH,
                                  (float)g_bannerFade / BANNER_FADE_STEPS);
        g_textRenderer.DrawOutlined(BANNER_CENTRE_TEXT_X - halfWidth, BANNER_CENTRE_TEXT_Y, g_dialogMessage,
                                    BANNER_TEXT_COLOUR, BANNER_TEXT_DEPTH, (float)g_bannerFade / BANNER_FADE_STEPS,
                                    BANNER_TEXT_SCALE);
    }
}
