// Opening a menu window slot of the Game object (EN build). Ported from the Ghidra output and the EN bytes; the JP twin
// (APP.dll 0x100084b0) is not matched.
#include "game.h"

// Opens the menu window `id` at (x, y) with the given size. A window that is already open is reset in its own slot,
// otherwise the first free slot is taken (and its cursor and fade restarted). The new window gets the cursor, empty
// labels and no result pointer, and the menu sound callback plays the open sound. Every OpenWindow call also advances the
// grow value of all three slots. Returns `id`, or 0 when all slots are in use. Called by the menu code (0x62e480,
// 0x6355e0, 0x6e0790, 0x6e0850, 0x6e1700).
int Game::OpenWindow(int id, int x, int y, int width, int height)
{
    int slot = FindSlot(id);
    if (slot == MENU_SLOT_NONE)
    {
        slot = 0;
        do
        {
            if (windowVisible[slot] == 0)
                break;
            slot++;
        } while (slot < MENU_SLOT_COUNT);
        if (slot == MENU_SLOT_NONE)
            return 0;
        windowCursor[slot] = 0;
        windowFade[slot] = 1;
    }
    m_068c[slot] = 0;
    windowX[slot] = x;
    windowY[slot] = y;
    windowWidth[slot] = width;
    windowOpen[slot] = 1;
    shownValue[slot] = 0;
    m_0134[slot] = 0;
    captionCallback[slot] = 0;
    windowId[slot] = id;
    windowHeight[slot] = height;
    windowRefresh[slot] = 0;
    m_0164[slot] = 0;
    m_0158[slot] = 0;
    activeWindow = slot;
    pageNumber[slot] = 0;
    m_00f8[slot] = 0;
    windowLastRow[slot] = WINDOW_ROWS_DEFAULT;
    windowFlags[slot] = WINDOW_FLAGS_DEFAULT;
    windowVisible[slot] = 1;
    windowCursor[slot] = 0;
    if (cursorRemembered[id] != 0)
        windowCursor[slot] = (unsigned char)savedCursor[id];
    menuItems[slot].entries[MENU_RESULT] = 0;
    m_0104 = WINDOW_OPEN_STATE;
    for (int entry = 0; entry < MENU_ENTRY_SLOTS; entry++)
        labels[slot][entry][0] = 0;
    menuSoundCallback(MENU_SOUND_OPEN);
    windowGrow[slot] = WINDOW_GROW_START;
    for (int each = 0; each < MENU_SLOT_COUNT; each++)
    {
        windowGrow[each] += WINDOW_GROW_STEP;
        for (int entry = 0; entry < MENU_ENTRY_COUNT; entry++)
        {
            if (menuItems[each].entries[entry] != 0)
                menuItems[each].entries[entry] = 0;
        }
    }
    return id;
}
