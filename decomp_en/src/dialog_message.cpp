// Setting the dialog message (EN build, EN-only: no JP twin found).
#include <string.h>
#include "dialogs.h"

// DialogMessage_Set: copies `text` into g_dialogMessage, or clears the message when `text` is null. A non-empty text also
// restarts the banner: fade mode BANNER_START_FADE_IN and fade counter 0. Returns 0.
extern "C" int DialogMessage_Set(const char *text)
{
    if (text != 0)
    {
        strcpy(g_dialogMessage, text);
        if (text[0] != 0)
        {
            g_overlayClosed = BANNER_START_FADE_IN;
            g_bannerFade = 0;
        }
    }
    else
        g_dialogMessage[0] = 0;
    return 0;
}
