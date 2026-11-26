// Key prompt countdown and banner fade step (EN build, EN-only: no JP twin found).
#include "prompt_fades.h"

// Number of characters (slots 0 up to this) whose key prompt time is counted down; the arrays hold more.
const int KEY_PROMPT_TICK_COUNT = 0x7c;
// Banner fade-in counter value when the banner is fully visible.
const int BANNER_FADE_FULL = 15;
// Values of g_overlayClosed (the banner fade mode): the banner fades in or out.
const int BANNER_MODE_FADE_IN = 1;
const int BANNER_MODE_FADE_OUT = 2;

// KeyPrompt_Tick: a shown prompt loses one frame of its display time per call and is hidden when none is left. Returns 0.
extern "C" int KeyPrompt_Tick(void)
{
    for (int i = 0; i < KEY_PROMPT_TICK_COUNT; i++)
    {
        if (g_keyPromptShown[i] != 0)
        {
            g_keyPromptFrames[i]--;
            if (g_keyPromptFrames[i] < 1)
                g_keyPromptShown[i] = 0;
        }
    }
    return 0;
}

// Banner_UpdateFade: steps the banner fade-in counter towards full (mode 1) or towards 0 (mode 2); once it is 0 the mode
// is cleared. Returns 0.
extern "C" int Banner_UpdateFade(void)
{
    int mode = g_overlayClosed;
    int fade = g_bannerFade;
    if (mode == BANNER_MODE_FADE_IN)
    {
        if (fade < BANNER_FADE_FULL)
        {
            fade++;
            g_bannerFade = fade;
        }
    }
    else if (mode == BANNER_MODE_FADE_OUT)
    {
        if (fade > 0)
        {
            fade--;
            g_bannerFade = fade;
        }
    }
    if (fade == 0)
        mode = 0;
    g_overlayClosed = mode;
    return 0;
}
