// The framed message with an optional subtext line (drawn by 0x1000eed0, called by 0x1008c680).

#include "font_renderer.h"
#include "game.h"

#include "game_vars.h"
#include "ui_vars.h"

#define PANEL_MARGIN 40         // the panel is this much wider than its widest text
#define SCREEN_CENTRE_X 320.0f
#define PANEL_LEFT_OFFSET 20.0f  // the text starts this far inside the panel
#define PANEL_HEIGHT_ONE_LINE 0x44
#define PANEL_HEIGHT_TWO_LINES 0x58
#define MESSAGE_FADE_FRAMES 10.0f
#define LARGE_TEXT_SCALE 2.0f
#define NORMAL_TEXT_SCALE 1.2f

// 0x1000eed0 (Message_DrawPanel). While a message is up (DAT_104bf8c8), draws it with the counter DAT_104bf8cc fading it in
// (full at 10). With the flag DAT_104c01fc clear the text sits at y = 256 (subtext at 276), otherwise at y = 192 (subtext at
// 212). A subtext of exactly 1 is not a text: the message alone is drawn at x = 180 at double size, without a panel. With no
// subtext, the message is centred on x = 320 in a panel 40 wider than the text (68 high); with one, both lines are centred
// on the wider of the two in a panel 88 high. Returns 1.
extern "C" int FUN_1000eed0(void)
{
    if (DAT_104bf8c8 != 0)
    {
        if (DAT_104c01fc == 0)
        {
            if (DAT_10467ea0[0] == 1)
            {
                DAT_1020e538.DrawOutlined(180, 256, DAT_10219bc0, 0xffffff, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES, LARGE_TEXT_SCALE);
                return 1;
            }
            if (DAT_10467ea0[0] == 0)
            {
                int width = DAT_1020e538.TextWidth(DAT_10219bc0, NORMAL_TEXT_SCALE);
                float left = SCREEN_CENTRE_X - (float)width * 0.5f;
                DAT_104b7200.DrawPanelAlt((int)(left - PANEL_LEFT_OFFSET), 234, width + PANEL_MARGIN, PANEL_HEIGHT_ONE_LINE, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES);
                DAT_1020e538.DrawOutlined((int)left, 256, DAT_10219bc0, 0xffffff, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
                return 1;
            }
            int width = DAT_1020e538.TextWidth(DAT_10219bc0, NORMAL_TEXT_SCALE);
            int subWidth = DAT_1020e538.TextWidth(DAT_10467ea0, NORMAL_TEXT_SCALE);
            if (subWidth > width)
            {
                width = subWidth;
            }
            float left = SCREEN_CENTRE_X - (float)width * 0.5f;
            DAT_104b7200.DrawPanelAlt((int)(left - PANEL_LEFT_OFFSET), 234, width + PANEL_MARGIN, PANEL_HEIGHT_TWO_LINES, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES);
            DAT_1020e538.DrawOutlined((int)left, 256, DAT_10219bc0, 0xffffff, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
            DAT_1020e538.DrawOutlined((int)left, 276, DAT_10467ea0, 0xffffff, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
        }
        else
        {
            if (DAT_10467ea0[0] == 1)
            {
                DAT_1020e538.DrawOutlined(180, 192, DAT_10219bc0, 0xffffff, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES, LARGE_TEXT_SCALE);
                return 1;
            }
            if (DAT_10467ea0[0] == 0)
            {
                int width = DAT_1020e538.TextWidth(DAT_10219bc0, NORMAL_TEXT_SCALE);
                float left = SCREEN_CENTRE_X - (float)width * 0.5f;
                DAT_104b7200.DrawPanelAlt((int)(left - PANEL_LEFT_OFFSET), 170, width + PANEL_MARGIN, PANEL_HEIGHT_ONE_LINE, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES);
                DAT_1020e538.DrawOutlined((int)left, 192, DAT_10219bc0, 0xffffff, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
                return 1;
            }
            int width = DAT_1020e538.TextWidth(DAT_10219bc0, NORMAL_TEXT_SCALE);
            int subWidth = DAT_1020e538.TextWidth(DAT_10467ea0, NORMAL_TEXT_SCALE);
            if (subWidth > width)
            {
                width = subWidth;
            }
            float left = SCREEN_CENTRE_X - (float)width * 0.5f;
            DAT_104b7200.DrawPanelAlt((int)(left - PANEL_LEFT_OFFSET), 170, width + PANEL_MARGIN, PANEL_HEIGHT_TWO_LINES, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES);
            DAT_1020e538.DrawOutlined((int)left, 192, DAT_10219bc0, 0xffffff, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
            DAT_1020e538.DrawOutlined((int)left, 212, DAT_10467ea0, 0xffffff, 0, (float)DAT_104bf8cc / MESSAGE_FADE_FRAMES, NORMAL_TEXT_SCALE);
        }
    }
    return 1;
}
