// Menu window functions of the Game object (EN build). Ported from the matched JP sources decomp/src/app_0004.cpp
// and app_0006.cpp.
#include <string.h>

#include "game.h"

// Sets the caption callback of menu `id`. Returns 0 when the menu is not open.
int Game::SetWindowCallbackValue(int id, void (*callback)(int cursor, char *caption))
{
    int slot = FindSlot(id);
    if (slot == MENU_SLOT_NONE)
        return 0;
    captionCallback[slot] = callback;
    return 1;
}

// Sets the pointer that receives the chosen result of menu `id` and clears the result. Returns 0 when the menu is
// not open.
int Game::SetWindowResult(int id, int *result)
{
    int slot = FindSlot(id);
    if (slot == MENU_SLOT_NONE)
        return 0;
    menuItems[slot].entries[MENU_RESULT] = result;
    if (result != 0)
        *result = 0;
    return 1;
}

// Closes menu `id`: plays the close sound through the menu sound callback and frees the slot. Returns 0 when the menu
// is not open.
int Game::CloseWindow(int id)
{
    int slot = FindSlot(id);
    if (slot == MENU_SLOT_NONE)
        return 0;
    menuSoundCallback(MENU_SOUND_CLOSE);
    windowOpen[slot] = 0;
    return 1;
}

// Looks up a named script constant (case-insensitive prefix match over the named map table); -1 when unknown.
int Game::FindNamedConstant(const char *name)
{
    for (int i = 0; i < namedEntryCount; i++)
    {
        if (_strnicmp(name, namedEntries[i].name, strlen(namedEntries[i].name)) == 0)
            return namedEntries[i].map;
    }
    return CONSTANT_NOT_FOUND;
}

// Sets the cursor position of menu `id` (and remembers it when that menu keeps its cursor).
int Game::SetWindowCursor(int id, int value)
{
    int slot = FindSlot(id);
    if (slot != MENU_SLOT_NONE)
    {
        windowCursor[slot] = value;
        if (cursorRemembered[id] != 0)
            savedCursor[id] = (char)windowCursor[slot];
    }
    return 0;
}
