// The framed message with an optional subtext line (EN build). Ported from the matched JP source decomp/src/message_panel.cpp.
#include "font_renderer.h"
#include "game.h"
#include "game_vars.h"
#include "dialogs.h"
#include "message_panel.h"

// The panel is this much wider than its widest text.
const int PANEL_MARGIN = 40;
// Screen centre x of the 640 wide layout.
const float SCREEN_CENTRE_X = 320.0f;
// The text starts this far inside the panel.
const float PANEL_LEFT_OFFSET = 20.0f;
// Panel height for one and for two text lines.
const int PANEL_HEIGHT_ONE_LINE = 0x44;
const int PANEL_HEIGHT_TWO_LINES = 0x58;
// Frames the fade-in counter takes to reach full opacity.
const float MESSAGE_FADE_FRAMES = 10.0f;
// Scale of the lone large message and of the normal message text.
const float LARGE_TEXT_SCALE = 2.0f;
const float NORMAL_TEXT_SCALE = 1.2f;

// Message_DrawPanel: while a message is up (g_messageOpen), draws it with the counter g_messageProgress fading it in
// (full at 10). With g_messageAlternateStyle clear the text sits at y = 256 (subtext at 276), otherwise at y = 192
// (subtext at 212). A subtext of exactly 1 is not a text: the message alone is drawn at x = 180 at double size, without
// a panel. With no subtext, the message is centred on x = 320 in a panel 40 wider than the text (68 high); with one,
// both lines are centred on the wider of the two in a panel 88 high. Returns 1. Called by the frame draw.
extern "C" int Message_DrawPanel(void)
{
    if (g_messageOpen == 0)
        return 1;
    if (g_messageAlternateStyle == 0)
    {
        if (g_messageSubtext[0] == 1)
        {
            g_textRenderer.DrawOutlined(180, 256, g_messageText, 0xffffff, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES, LARGE_TEXT_SCALE);
            return 1;
        }
        if (g_messageSubtext[0] == 0)
        {
            int width = g_textRenderer.TextWidth(g_messageText, NORMAL_TEXT_SCALE);
            float left = SCREEN_CENTRE_X - (float)width * 0.5f;
            g_dialogGame.DrawPanelAlt((int)(left - PANEL_LEFT_OFFSET), 234, width + PANEL_MARGIN, PANEL_HEIGHT_ONE_LINE, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES);
            g_textRenderer.DrawOutlined((int)left, 256, g_messageText, 0xffffff, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
            return 1;
        }
        int width = g_textRenderer.TextWidth(g_messageText, NORMAL_TEXT_SCALE);
        int subWidth = g_textRenderer.TextWidth(g_messageSubtext, NORMAL_TEXT_SCALE);
        if (subWidth > width)
        {
            width = subWidth;
        }
        float left = SCREEN_CENTRE_X - (float)width * 0.5f;
        g_dialogGame.DrawPanelAlt((int)(left - PANEL_LEFT_OFFSET), 234, width + PANEL_MARGIN, PANEL_HEIGHT_TWO_LINES, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES);
        g_textRenderer.DrawOutlined((int)left, 256, g_messageText, 0xffffff, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
        g_textRenderer.DrawOutlined((int)left, 276, g_messageSubtext, 0xffffff, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
    }
    else
    {
        if (g_messageSubtext[0] == 1)
        {
            g_textRenderer.DrawOutlined(180, 192, g_messageText, 0xffffff, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES, LARGE_TEXT_SCALE);
            return 1;
        }
        if (g_messageSubtext[0] == 0)
        {
            int width = g_textRenderer.TextWidth(g_messageText, NORMAL_TEXT_SCALE);
            float left = SCREEN_CENTRE_X - (float)width * 0.5f;
            g_dialogGame.DrawPanelAlt((int)(left - PANEL_LEFT_OFFSET), 170, width + PANEL_MARGIN, PANEL_HEIGHT_ONE_LINE, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES);
            g_textRenderer.DrawOutlined((int)left, 192, g_messageText, 0xffffff, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
            return 1;
        }
        int width = g_textRenderer.TextWidth(g_messageText, NORMAL_TEXT_SCALE);
        int subWidth = g_textRenderer.TextWidth(g_messageSubtext, NORMAL_TEXT_SCALE);
        if (subWidth > width)
        {
            width = subWidth;
        }
        float left = SCREEN_CENTRE_X - (float)width * 0.5f;
        g_dialogGame.DrawPanelAlt((int)(left - PANEL_LEFT_OFFSET), 170, width + PANEL_MARGIN, PANEL_HEIGHT_TWO_LINES, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES);
        g_textRenderer.DrawOutlined((int)left, 192, g_messageText, 0xffffff, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
        g_textRenderer.DrawOutlined((int)left, 212, g_messageSubtext, 0xffffff, 0, (float)g_messageProgress / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
    }
    return 1;
}
