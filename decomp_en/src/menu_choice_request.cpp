// Requesting a choice menu (EN build, EN-only: no JP twin found).
#include "small_b11.h"

// MenuChoice_Request: stores the request (value, third value, caption pointer and a copy of the caption text) and sets
// g_menuChoiceMode to MENU_CHOICE_REQUESTED; the main loop (EN 0x609f70) opens the menu on its next pass. A null
// caption leaves the copy untouched. Returns 0.
extern "C" int MenuChoice_Request(int value, const char *caption, int extra)
{
    g_menuChoice = value;
    g_menuChoiceExtra = extra;
    g_menuChoiceMode = MENU_CHOICE_REQUESTED;
    g_menuChoiceCaptionGiven = caption;
    if (caption != 0)
    {
        char *destination = g_menuChoiceCaption;
        while ((*destination++ = *caption++) != '\0')
            ;
    }
    return 0;
}
