// The text window stack of the EN build (the in-game text entry and list windows): the globals every window method reads.
// This is the one declaration of them in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_TEXT_WINDOW_H
#define GURUMIN_EN_TEXT_WINDOW_H

struct TextWindow;

extern "C" TextWindow *g_textWindowTop;   // EN 0x0222b140: the window on top of the stack (0 = none); each window links to the one below it
extern "C" int g_textWindowSetting;       // EN 0x0222b144: value stored by TextWindows_Reset, read by every window method (not recovered)
extern "C" int g_textWindowDepth;         // EN 0x0222b148: number of windows stacked below the top one

// Empties the window stack, stores `setting` and starts text input into `buffer` (TextInput_Begin). Returns 0. EN 0x7b8130.
extern "C" int TextWindows_Reset(char *buffer, int setting);

#endif
