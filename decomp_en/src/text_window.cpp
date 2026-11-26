// Text window stack start-up (EN build, EN-only: no JP twin found).
#include "ime.h"
#include "text_window.h"

// TextWindows_Reset: forgets every stacked text window, remembers `font` for the window methods and starts the text
// input into `buffer`. Returns 0. Called through a pointer (no direct caller).
extern "C" int TextWindows_Reset(char *buffer, FontRenderer *font)
{
    g_textWindowDepth = 0;
    g_textWindowTop = 0;
    g_textWindowFont = font;
    TextInput_Begin(buffer);
    return 0;
}
