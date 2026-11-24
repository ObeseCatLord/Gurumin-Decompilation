// Screen overlays and message text.

#include <string.h>

#include "tri_batch.h"

#include "texture_object.h"
#include "render_state.h"

#include "game_vars.h"
#include "ui_vars.h"
#include "message_window.h"

// Draws the full-screen overlay quad (texture slot DAT_104c0140) with additive blending.
extern "C" int FUN_1000c340(void)
{
    FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);
    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
    DAT_1021cffc.DrawWith(&DAT_118d0ba8[DAT_104c0140]);
    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    return 1;
}

// 0x1000cb30. Sets the message window title text (DAT_10210ae8); passing 0 closes the window.
extern "C" int FUN_1000cb30(const char *text)
{
    return MessageWindow_SetTitle(text);
}
