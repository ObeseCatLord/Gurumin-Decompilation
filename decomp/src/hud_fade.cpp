// HUD overlay drawn during screen fades.

#include "game.h"

#include "texture_object.h"

#include "game_vars.h"
#include "ui_vars.h"

extern "C" void FUN_1013f070(int, int, int);

// Draws the dark panel (and the texture DAT_102024a0 when one is set) with an alpha of DAT_104bff0c / 15
// while the HUD is visible (DAT_104bfd78) or the game is in state 0 (DAT_104bf8c0).
extern "C" void FUN_1000f4d0(void)
{
    if (DAT_104bfd78 != 0 || DAT_104bf8c0 == 0)
    {
        FUN_1013f070(9, 0, 0);
        if (DAT_104bff00 != 0 || DAT_104bff0c != 0)
        {
            DAT_104b7200.DrawPanelAlt(0x19d, 0x74, 200, 0xcc, 0, DAT_104bff0c / 15.0f);
            if (DAT_102024a0 > 0)
                DAT_118d0ba8[DAT_102024a0].DrawFrameAltStretched(0x19f, 0x76, 0x263, 0x13e, 0, DAT_104bff0c / 15.0f, 1, 0);
        }
        FUN_1013f070(9, 1, 0);
    }
}
