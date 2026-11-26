// Menu window state queries and setters of the Game object (EN build): the window flags, the row count, the "any window
// shown" test and the clearing of the event queues. EN-only (no JP twin).
#include <string.h>

#include "game.h"

// Fade-in counter above which a window that is not open any more still counts as shown.
const int WINDOW_FADE_SHOWN_ABOVE = 16;

// Tells whether any menu window slot is open, or visible and faded in past WINDOW_FADE_SHOWN_ABOVE. Returns 1 or 0.
// Called by the per-frame update (0x60cb10) and by 0x5b1a60.
int Game::IsAnyWindowShown()
{
    for (int slot = 0; slot < MENU_SLOT_COUNT; slot++)
    {
        if (windowOpen[slot] != 0)
            return 1;
        if (windowVisible[slot] != 0 && windowFade[slot] > WINDOW_FADE_SHOWN_ABOVE)
            return 1;
    }
    return 0;
}

// Sets the layout flags of the open window `id` (0x18 or 0x16 where the callers use them). Does nothing when the window is
// not open. Returns 0. Called by the menu code at 0x62e480.
int Game::SetWindowFlags(int id, int flags)
{
    int slot = FindSlot(id);
    if (slot != MENU_SLOT_NONE)
        windowFlags[slot] = flags;
    return 0;
}

// Stores the number of rows of the open window `id` (kept as rows - 1). Does nothing when the window is not open.
// Returns 0. Called by the menu code at 0x62e480 and 0x6355e0.
int Game::SetWindowRows(int id, int rows)
{
    int slot = FindSlot(id);
    if (slot != MENU_SLOT_NONE)
        windowLastRow[slot] = rows - 1;
    return 0;
}

// Empties the list of running events and the list of queued events. Returns 0. No direct caller (reached through a pointer).
int Game::ClearEventQueues()
{
    memset(runningEvents, 0, sizeof(runningEvents));
    memset(queuedEvents, 0, sizeof(queuedEvents));
    return 0;
}
