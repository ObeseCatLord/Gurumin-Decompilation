// Globals and routines of the main menu (Menu_Main, src/menu_main.cpp): the title menu, the difficulty and sound
// pages, the name entry, the shop buttons and the first save list. This is the one declaration of each of them:
// include this header, never redeclare one.
#ifndef GURUMIN_MENU_MAIN_H
#define GURUMIN_MENU_MAIN_H

#include "menu_save_load.h"

// Menu window ids used by Menu_Main besides the ones of menu_save_load.h.
enum MenuMainWindow
{
    WINDOW_MAIN_MENU = 0x22b,     // the list of the title menu (new game, continue, sound, item dictionary, quit)
    WINDOW_LIST_PANEL = 0x218,    // the page of the difficulty, sound and name entry lists
    WINDOW_VOLUME = 0x219,        // the volume page of the sound configuration
    WINDOW_MAP_LIST = 0x230,      // the debug list of maps
    WINDOW_LOAD_FRAME = 0x22f     // closed when the load list opens
};

extern "C" int DAT_1030c514;      // set when the title question "back to the title" was answered "yes"
extern "C" int DAT_1030c518;      // set when a title question was answered "no" or cancelled
extern "C" int DAT_1030c51c;      // set when a map was picked in the debug list
extern "C" int DAT_1030c520;      // set when the debug list picked "start at the beginning"
extern "C" int DAT_1030c524;      // set when the difficulty list was cancelled
extern "C" int DAT_1030c528;      // set when "Hard" was picked
extern "C" int DAT_1030c530;      // set when "Beginner" was picked
extern "C" int DAT_1030c534;      // set when "new game" was picked
extern "C" int DAT_1030c538;      // set to leave the difficulty page after a pick
extern "C" int DAT_1030c53c;      // set to open the difficulty page
extern "C" int DAT_1030c554;      // set when the item list was cancelled
extern "C" int DAT_1030c560;      // set when the question to leave the shop was answered "yes"
extern "C" int DAT_1030c564;      // set when the leave command of the shop was picked
extern "C" int DAT_1030c58c;      // set when the title menu list was cancelled
extern "C" int DAT_1030c5a0;      // set when "Pleasure" was picked
extern "C" int DAT_1030c5a4;      // set when "Crazy" was picked
extern "C" int DAT_1030c5a8;      // set to open the list of maps
extern "C" int DAT_1030c5ac;
extern "C" int DAT_1030c660;      // set when "new game" is started from the main menu
extern "C" int DAT_1030c6bc;      // set when the item dictionary was picked
extern "C" int DAT_1030c7e0;      // set when "continue" was picked
extern "C" int DAT_1030c7e4;      // set to rebuild the continue list
extern "C" int DAT_1030c7f8;      // set to open the sound configuration
extern "C" int DAT_1030c800;      // set to open the volume page
extern "C" int DAT_1030c804;      // the sound output setting being edited
extern "C" int DAT_1030c808;      // the voice setting being edited (0 = voices on)
extern "C" int DAT_1030c80c;      // set when the volume page was cancelled
extern "C" int DAT_1030c810;      // set when the volume was lowered one step
extern "C" int DAT_1030c814;      // set when the volume was raised one step
extern "C" int DAT_1030c818;      // set when the sound configuration was confirmed
extern "C" int DAT_104c02d0;      // the helmet picked to be upgraded

extern "C" long FUN_10001b70(long volume);          // 0x10001b70 (SoundArc_Message227, app_0002.cpp): the music attenuation
extern "C" int FUN_1000ed70(const char *text, const char *subtext, int frames);   // 0x1000ed70: opens the message window
extern "C" int FUN_10012130(int *item, int *count);  // 0x10012130 (Inventory_Add)
#include "value_command.h"
extern "C" int FUN_1015d500(int value);              // 0x1015d500 (ime.cpp): starts the text input
extern "C" int FUN_1015f310(char *text);             // 0x1015f310 (ime.cpp): starts the name entry with `text`

extern "C" int FUN_10112140(void);                   // 0x10112140 (Menu_Main)

#endif
