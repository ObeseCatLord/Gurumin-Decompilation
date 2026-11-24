// Message window title (the banner text DAT_10210ae8 and its open/close state).

#ifndef MESSAGE_WINDOW_H
#define MESSAGE_WINDOW_H

#include <string.h>

#include "game_vars.h"
#include "ui_vars.h"

#define MESSAGE_WINDOW_OPEN    1   // DAT_104bf8d4: shown, fading in
#define MESSAGE_WINDOW_CLOSING 2   // DAT_104bf8d4: fading out

// Out-of-line copy of MessageWindow_SetTitle (0x1000cb30).
extern "C" int FUN_1000cb30(const char *text);

// Sets the message window title; 0 closes the window. Opening it the first time resets its fade-in
// counter. The menus inline it (Menu_SaveLoad closes the window with it twice).
__forceinline int MessageWindow_SetTitle(const char *text)
{
    if (DAT_104bf8d4 != MESSAGE_WINDOW_OPEN)
    {
        DAT_104bf8d4 = MESSAGE_WINDOW_OPEN;
        DAT_104bf8d8 = 0;
    }
    if (text != 0)
    {
        if (*text == 0)
        {
            DAT_10210ae8[0] = 0;
            return 0;
        }
        strcpy(DAT_10210ae8, text);
        return 0;
    }
    DAT_104bf8d4 = MESSAGE_WINDOW_CLOSING;
    return 0;
}

#endif
