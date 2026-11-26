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
