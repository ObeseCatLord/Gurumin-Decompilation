// Status panel (EN build). Ported from the matched JP source decomp/src/status_panel.cpp.
#include "dialogs.h"
#include "font_renderer.h"
#include "render.h"
#include "status_panel.h"
#include "texture_mgr.h"

// Render_Command code and value the panel is drawn with (the blend mode), and the panel's size and alpha.
const int PANEL_RENDER_COMMAND = 2;
const int PANEL_Y = 295;
const int PANEL_HEIGHT = 140;
const int PANEL_RIGHT_EDGE = 644;
const float PANEL_ALPHA_SCALE = 0.85f;
// Left edge of the panel on a screen that is at most this much wider than 4:3, and on a wider one.
const float PANEL_NARROW_LIMIT = 1.2f;
const int PANEL_X_NARROW = 425;
const int PANEL_X_WIDE = 470;
// Panel states in which the text shows, and the lowest fade at which it does.
const int PANEL_TEXT_FIRST_STATE = 8;
const int PANEL_TEXT_LAST_STATE = 22;
const float PANEL_TEXT_MIN_ALPHA = 0.1f;
// The text lines: x offset from the panel's left edge and y of each line (lines 0, 1, 3, 4 and 5 of the table show).
const int PANEL_TEXT_INDENT = 20;
const int PANEL_LINE_Y[] = { 315, 335, 355, 375, 400 };
// The last line turns yellow from this level on.
const char PANEL_LAST_LINE_YELLOW_LEVEL = 4;
const int COLOUR_WHITE = 0xffffff;
const int COLOUR_YELLOW = 0xffff00;
// Text outline colour (none), alpha and scale.
const float PANEL_TEXT_SCALE = 0.7f;

// StatusPanel_Draw: draws the panel at its place at 85 % of `alpha`, and in states 8..22 (once `alpha` is at least
// 0.1) its five lines of white text, the last one yellow when the level is 4 or more. Returns 0. Called every frame
// by the world renderer.
extern "C" int StatusPanel_Draw(float alpha)
{
    if (g_statusPanelState == 0)
        return 0;
    Render_Command(PANEL_RENDER_COMMAND, 0, 0);
    int x = PANEL_X_WIDE;
    if (PANEL_NARROW_LIMIT >= g_screenWidthDivisor)
        x = PANEL_X_NARROW;
    g_dialogGame.DrawPanelAlt(x, PANEL_Y, PANEL_RIGHT_EDGE - x, PANEL_HEIGHT, 0, alpha * PANEL_ALPHA_SCALE);
    int lastColour = g_statusPanelLevel < PANEL_LAST_LINE_YELLOW_LEVEL ? COLOUR_WHITE : COLOUR_YELLOW;
    if (g_statusPanelState < PANEL_TEXT_FIRST_STATE || g_statusPanelState > PANEL_TEXT_LAST_STATE ||
        alpha < PANEL_TEXT_MIN_ALPHA)
        return 0;
    x += PANEL_TEXT_INDENT;
    g_textRenderer.DrawOutlined(x, PANEL_LINE_Y[0], g_statusPanelLines[0], COLOUR_WHITE, 0, 1.0f, PANEL_TEXT_SCALE);
    g_textRenderer.DrawOutlined(x, PANEL_LINE_Y[1], g_statusPanelLines[1], COLOUR_WHITE, 0, 1.0f, PANEL_TEXT_SCALE);
    g_textRenderer.DrawOutlined(x, PANEL_LINE_Y[2], g_statusPanelLines[3], COLOUR_WHITE, 0, 1.0f, PANEL_TEXT_SCALE);
    g_textRenderer.DrawOutlined(x, PANEL_LINE_Y[3], g_statusPanelLines[4], COLOUR_WHITE, 0, 1.0f, PANEL_TEXT_SCALE);
    g_textRenderer.DrawOutlined(x, PANEL_LINE_Y[4], g_statusPanelLines[5], lastColour, 0, 1.0f, PANEL_TEXT_SCALE);
    return 0;
}
