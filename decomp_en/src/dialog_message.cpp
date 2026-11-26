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

// BannerText_Set: copies `text` into g_bannerText. Unless the banner is already fading in it restarts fading in from 0;
// an empty text clears the banner text and a null `text` makes the banner fade out. Returns 0. Called by the script
// commands that show or hide a banner.
extern "C" int BannerText_Set(const char *text)
{
    if (g_overlayClosed != BANNER_START_FADE_IN)
    {
        g_overlayClosed = BANNER_START_FADE_IN;
        g_bannerFade = 0;
    }
    if (text != 0)
    {
        if (text[0] == 0)
        {
            g_bannerText[0] = 0;
            return 0;
        }
        strcpy(g_bannerText, text);
        return 0;
    }
    g_overlayClosed = BANNER_START_FADE_OUT;
    return 0;
}
