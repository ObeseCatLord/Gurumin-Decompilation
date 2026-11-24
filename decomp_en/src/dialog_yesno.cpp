// Yes/no dialogs (EN build). Ported from the matched JP source decomp/src/dialog_yesno.cpp.
#include "dialogs.h"
#include "game.h"

// Window id, position and size of the yes/no dialog.
const int YESNO_WINDOW_ID = 0x216;
const int YESNO_WINDOW_X = 0x100;
const int YESNO_WINDOW_Y = 0x90;
const int YESNO_WINDOW_WIDTH = 0x7c;
const int YESNO_WINDOW_HEIGHT = 0x60;
// Layout flags of the dialog's window slot.
const int YESNO_WINDOW_FLAGS = 0x20;

// Dialog_YesNoNoFirst: shows `message` with "No" as the first choice (for dangerous questions such as overwriting a
// save). The original passes the no value as the result pointer too. Returns 1. Called by the save and quit menus.
extern "C" int Dialog_YesNoNoFirst(const char *message, int yesValue, int noValue)
{
    g_game.OpenWindow(YESNO_WINDOW_ID, YESNO_WINDOW_X, YESNO_WINDOW_Y, YESNO_WINDOW_WIDTH, YESNO_WINDOW_HEIGHT);
    g_game.SetWindowResult(YESNO_WINDOW_ID, (int *)noValue);
    MessageWindow_SetTitle("");
    MessageWindow_SetBody(message);
    int slot = g_game.FindSlot(YESNO_WINDOW_ID);
    if (slot != MENU_SLOT_COUNT)
        g_game.windowFlags[slot] = YESNO_WINDOW_FLAGS;
    g_game.AddFlagEntry(YESNO_WINDOW_ID, g_textNo, (int *)noValue);
    g_game.AddFlagEntry(YESNO_WINDOW_ID, g_textYes, (int *)yesValue);
    return 1;
}

// Dialog_YesNoYesFirst: shows `message` with "Yes" as the first choice. Returns 1.
extern "C" int Dialog_YesNoYesFirst(const char *message, int yesValue, int noValue)
{
    g_game.OpenWindow(YESNO_WINDOW_ID, YESNO_WINDOW_X, YESNO_WINDOW_Y, YESNO_WINDOW_WIDTH, YESNO_WINDOW_HEIGHT);
    g_game.SetWindowResult(YESNO_WINDOW_ID, (int *)noValue);
    MessageWindow_SetTitle("");
    MessageWindow_SetBody(message);
    int slot = g_game.FindSlot(YESNO_WINDOW_ID);
    if (slot != MENU_SLOT_COUNT)
        g_game.windowFlags[slot] = YESNO_WINDOW_FLAGS;
    g_game.AddFlagEntry(YESNO_WINDOW_ID, g_textYes, (int *)yesValue);
    g_game.AddFlagEntry(YESNO_WINDOW_ID, g_textNo, (int *)noValue);
    return 1;
}
