// A five-line text panel at the lower right of the screen (state DAT_1020c330).

#include "font_renderer.h"
#include "game.h"

#include "ui_vars.h"

extern "C" void FUN_1013f070(int mode, int a, int b);  // sets the 2D blend mode

// 0x1001e340. Draws the panel at (460, 295), 184x140, at 85 % of `alpha`, and in states 8..22 (once `alpha` is at
// least 0.1) its five lines of white text, the last one yellow when DAT_1020dc80 is 4 or more. Returns 0.
extern "C" int FUN_1001e340(float alpha)
{
    if (DAT_1020c330 == 0)
        return 0;
    FUN_1013f070(2, 0, 0);
    DAT_104b7200.DrawPanelAlt(460, 295, 184, 140, 0, alpha * 0.85f);
    int lastColour = DAT_1020dc80 < 4 ? 0xffffff : 0xffff00;
    if (DAT_1020c330 < 8 || DAT_1020c330 > 22 || alpha < 0.1f)
        return 0;
    DAT_1020e538.DrawOutlined(480, 315, DAT_1020da50[0], 0xffffff, 0, 1.0f, 0.7f);
    DAT_1020e538.DrawOutlined(480, 335, DAT_1020daa0, 0xffffff, 0, 1.0f, 0.7f);
    DAT_1020e538.DrawOutlined(480, 355, DAT_1020db40, 0xffffff, 0, 1.0f, 0.7f);
    DAT_1020e538.DrawOutlined(480, 375, DAT_1020db90, 0xffffff, 0, 1.0f, 0.7f);
    DAT_1020e538.DrawOutlined(480, 400, DAT_1020dbe0, lastColour, 0, 1.0f, 0.7f);
    return 0;
}
