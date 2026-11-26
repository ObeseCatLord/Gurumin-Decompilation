// Text window stack start-up (EN build, EN-only: no JP twin found).
#include "ime.h"
#include "text_window.h"

// TextWindows_Reset: forgets every stacked text window, remembers `setting` for the window methods and starts the text
// input into `buffer`. Returns 0. Called through a pointer (no direct caller).
extern "C" int TextWindows_Reset(char *buffer, int setting)
{
    g_textWindowDepth = 0;
    g_textWindowTop = 0;
    g_textWindowSetting = setting;
    TextInput_Begin(buffer);
    return 0;
}
