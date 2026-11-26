// Menu window refresh request (EN build, EN-only: no JP twin found).
#include "game.h"

// Flags the open window `id` for refresh. Returns 1, or 0 when no open window has that id. No direct caller.
int Game::RequestWindowRefresh(int id)
{
    int slot = FindSlot(id);
    if (slot == MENU_SLOT_NONE)
        return 0;
    windowRefresh[slot] = 1;
    return 1;
}
