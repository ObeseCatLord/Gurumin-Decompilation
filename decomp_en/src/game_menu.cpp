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

// Game::AddFlagEntry: adds a flag entry (kind MENU_ENTRY_FLAG) to menu window `id`: `flag` (cleared now) is the
// variable it sets. Returns MENU_ENTRY_COUNT, or 0 when the window is not open or has no free entry. Called by the
// menu scripts.
int Game::AddFlagEntry(int id, const char *label, int *flag)
{
    int slot = FindSlot(id);
    if (slot == MENU_SLOT_NONE)
        return 0;
    int i;
    for (i = 0; i < MENU_ENTRY_SLOTS; i++)
    {
        if (labels[slot][i][0] == 0)
            break;
    }
    if (i == MENU_ENTRY_SLOTS)
        return 0;
    if (flag != 0)
        *flag = 0;
    strcpy(labels[slot][i], label);
    menuItems[slot].entries[i] = flag;
    entryKind[slot][i] = MENU_ENTRY_FLAG;
    entryMax[slot][i] = -1;
    entryMin[slot][i] = 0;
    lastEntry[slot] = i;
    for (i = 0; i < MENU_ENTRY_COUNT; i++)
    {
        if (menuItems[slot].entries[i] != 0 && entryKind[slot][i] == MENU_ENTRY_FLAG)
            *menuItems[slot].entries[i] = 0;
    }
    return i;
}

// Game::AddKind5Entry: AddFlagEntry with entry kind MENU_ENTRY_KIND5.
int Game::AddKind5Entry(int id, const char *label, int *flag)
{
    int slot = FindSlot(id);
    if (slot == MENU_SLOT_NONE)
        return 0;
    int i;
    for (i = 0; i < MENU_ENTRY_SLOTS; i++)
    {
        if (labels[slot][i][0] == 0)
            break;
    }
    if (i == MENU_ENTRY_SLOTS)
        return 0;
    if (flag != 0)
        *flag = 0;
    strcpy(labels[slot][i], label);
    menuItems[slot].entries[i] = flag;
    entryKind[slot][i] = MENU_ENTRY_KIND5;
    entryMax[slot][i] = -1;
    entryMin[slot][i] = 0;
    lastEntry[slot] = i;
    for (i = 0; i < MENU_ENTRY_COUNT; i++)
    {
        if (menuItems[slot].entries[i] != 0 && entryKind[slot][i] == MENU_ENTRY_FLAG)
            *menuItems[slot].entries[i] = 0;
    }
    return i;
}

// Game::AddRangeEntry: adds a range entry (kind MENU_ENTRY_RANGE) to menu window `id`: `value` (cleared now) is the
// variable it sets between `low` and `high`. Returns MENU_ENTRY_COUNT, or 0 when the window is not open or has no free
// entry.
int Game::AddRangeEntry(int id, const char *label, int low, int high, int *value)
{
    int slot = FindSlot(id);
    if (slot == MENU_SLOT_NONE)
        return 0;
    int i;
    for (i = 0; i < MENU_ENTRY_SLOTS; i++)
    {
        if (labels[slot][i][0] == 0)
            break;
    }
    if (i == MENU_ENTRY_SLOTS)
        return 0;
    if (value != 0)
        *value = 0;
    strcpy(labels[slot][i], label);
    menuItems[slot].entries[i] = value;
    entryMax[slot][i] = high;
    entryKind[slot][i] = MENU_ENTRY_RANGE;
    entryMin[slot][i] = low;
    lastEntry[slot] = i;
    for (i = 0; i < MENU_ENTRY_COUNT; i++)
    {
        if (menuItems[slot].entries[i] != 0 && entryKind[slot][i] == MENU_ENTRY_FLAG)
            *menuItems[slot].entries[i] = 0;
    }
    return i;
}
