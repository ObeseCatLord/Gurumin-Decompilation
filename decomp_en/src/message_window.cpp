// Per-frame step of the script message window (EN build, EN-only: no JP twin found). The state lives in
// game_vars.h (g_messageOpen, g_messageProgress, g_messageScroll, g_messageFrames); Message_Show starts it.
#include "game_vars.h"

// g_messageOpen values: the window opens (progress counts up), or closes (progress counts down).
const int MESSAGE_WINDOW_OPENING = 1;
const int MESSAGE_WINDOW_CLOSING = 2;
// Progress at which the window counts as fully open.
const int MESSAGE_WINDOW_FULL = 10;

// MessageWindow_Tick: advances the message window by one frame. The window opens until its progress reaches
// MESSAGE_WINDOW_FULL, stays open for g_messageFrames frames, then closes; once the progress is back at 0 the
// window is gone (g_messageOpen = 0). Returns 1. Called once per frame by the world update and the scene loops.
extern "C" int MessageWindow_Tick(void)
{
    int mode;
    g_messageScroll++;
    if (g_messageScroll > g_messageFrames)
        mode = MESSAGE_WINDOW_CLOSING;
    else
        mode = g_messageOpen;
    if (mode == MESSAGE_WINDOW_OPENING)
    {
        if (g_messageProgress < MESSAGE_WINDOW_FULL)
            g_messageProgress++;
    }
    else if (mode == MESSAGE_WINDOW_CLOSING)
    {
        if (g_messageProgress > 0)
            g_messageProgress--;
    }
    if (g_messageProgress == 0)
        mode = 0;
    g_messageOpen = mode;
    return 1;
}
