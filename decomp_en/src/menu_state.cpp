// Menu input state reset (EN build, EN-only: no JP twin found).
#include "menu_state.h"
#include "messages.h"
#include "small_b11.h"

// MenuInput_Reset: puts the menu input globals back to their start values. Returns 0. Called by the game script
// builtins (RunGameBuiltin, 0x5e87d0) after a menu or message command.
extern "C" int MenuInput_Reset(void)
{
    g_menuStateA363a0 = MENU_STATE_RESET_VALUE;
    g_menuStateA363a4 = 0;
    g_menuCursorReported = 0;
    g_messageKeyWait = 0;
    return 0;
}

// MenuFade_Tick: advances the menu opening or closing counter by one frame. Returns 1. Called by the menu loop
// (EN 0x60cb10) once per frame.
extern "C" int MenuFade_Tick(void)
{
    if (g_menuStateA363a0 == MENU_STATE_OPENING)
    {
        g_menuStateA363a4++;
        if (g_menuStateA363a4 > MENU_FADE_OPEN_AFTER)
        {
            g_menuStateA363a0 = MENU_STATE_OPEN;
            g_menuOpenedA1da50 = MENU_FADE_OPENED;
        }
    }
    else if (g_menuStateA363a0 == MENU_STATE_CLOSING)
    {
        g_menuStateA363a4--;
        if (g_menuStateA363a4 == 0)
            g_menuStateA363a0 = 0;
    }
    return MENU_FADE_OPENED;
}
