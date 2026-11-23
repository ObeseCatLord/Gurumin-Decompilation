// The name banners shown when a map starts: one or two framed panels with a name in outlined text, fading in with
// the counter DAT_104bf8d8 (full at 15).

#include "font_renderer.h"
#include "game.h"

#include "game_vars.h"
#include "ui_vars.h"

// 0x1000cc30 (Hud_DrawNameBanners). With a first text, it is drawn at the upper left (panel at (32, 10)) and a second
// text next to it (panel at (310, 10)); with only the second text, that one is centred at the top (panel centred on
// x = 304 at y = 74). Each panel is 40 pixels wider than its text (32 when centred) and 64 high.
extern "C" void FUN_1000cc30(void)
{
    if (DAT_10210ae8[0] != 0)
    {
        int width = DAT_1020e538.TextWidth(DAT_10210ae8, 1.0f);
        DAT_104b7200.DrawPanelAlt(32, 10, width + 40, 64, 0, (float)DAT_104bf8d8 / 15.0f);
        DAT_1020e538.DrawOutlined(52, 32, DAT_10210ae8, 0xffffff, 0, (float)DAT_104bf8d8 / 15.0f, 1.0f);
        if (DAT_1020e0f8[0] != 0)
        {
            width = DAT_1020e538.TextWidth(DAT_1020e0f8, 1.0f);
            DAT_104b7200.DrawPanelAlt(310, 10, width + 40, 64, 0, (float)DAT_104bf8d8 / 15.0f);
            DAT_1020e538.DrawOutlined(330, 32, DAT_1020e0f8, 0xffffff, 0, (float)DAT_104bf8d8 / 15.0f, 1.0f);
        }
    }
    else if (DAT_1020e0f8[0] != 0)
    {
        int width = DAT_1020e538.TextWidth(DAT_1020e0f8, 1.0f);
        DAT_104b7200.DrawPanelAlt(304 - width / 2, 74, width + 32, 64, 0, (float)DAT_104bf8d8 / 15.0f);
        DAT_1020e538.DrawOutlined(320 - width / 2, 96, DAT_1020e0f8, 0xffffff, 0, (float)DAT_104bf8d8 / 15.0f, 1.0f);
    }
}
