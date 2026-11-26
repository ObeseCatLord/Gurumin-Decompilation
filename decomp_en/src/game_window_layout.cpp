// Menu window layout setters and slot resets of the Game object (EN build). The first two have JP twins
// (SetWindowLayout, SetWindowLayout2 in decomp/src/app_0006.cpp); the others are EN-only.
#include <string.h>
#include "game.h"

// Number of ints of a window slot's entry table cleared by ResetWindowSlots.
const int WINDOW_ENTRY_TABLE_INTS = MENU_ENTRY_COUNT;

// Sets two layout values and the page number pointers of open window `id`. Returns 0. Called by the map and
// system menus when they open a paged window.
int Game::SetWindowLayout(int id, int a, int b, int *pageNumberPtr, int *pageChangedPtr)
{
    int slot = FindSlot(id);
    if (slot != MENU_SLOT_NONE)
    {
        layoutA[slot] = a;
        layoutB[slot] = b;
        pageNumber[slot] = pageNumberPtr;
        pageChanged[slot] = pageChangedPtr;
    }
    return 0;
}

// Sets the value open window `id` draws, with the position it is drawn at. Returns 0. No direct caller.
int Game::SetWindowShownValue(int id, int x, int y, int *value)
{
    int slot = FindSlot(id);
    if (slot != MENU_SLOT_NONE)
    {
        shownValue[slot] = value;
        shownValueX[slot] = x;
        shownValueY[slot] = y;
    }
    return 0;
}

// Moves and resizes the window slot that has the cursor (nothing while no slot has it). Returns 0. No direct caller.
int Game::SetActiveWindowRect(int x, int y, int width, int height)
{
    if (activeWindow > -1)
    {
        windowX[activeWindow] = x;
        windowY[activeWindow] = y;
        windowWidth[activeWindow] = width;
        windowHeight[activeWindow] = height;
    }
    return 0;
}

// Closes every window slot (id, open and shown flags) and clears the entry kinds of all three. Called by the
// scene start (EN 0x5a2dd0).
int Game::ResetWindowSlots()
{
    for (int slot = 0; slot < MENU_SLOT_COUNT; slot++)
    {
        windowVisible[slot] = 0;
        windowOpen[slot] = 0;
        windowId[slot] = 0;
        for (int entry = 0; entry < WINDOW_ENTRY_TABLE_INTS; entry++)
            entryKind[slot][entry] = 0;
    }
    return 0;
}

// Clears the open, shown and fade values of every window slot that has any of them set. Returns 1. Called by the
// window update (EN 0x603a90, 0x605380).
int Game::ClearFadedWindows()
{
    for (int slot = 0; slot < MENU_SLOT_COUNT; slot++)
    {
        if (windowOpen[slot] != 0 || windowVisible[slot] != 0 || windowFade[slot] != 0)
        {
            windowOpen[slot] = 0;
            windowVisible[slot] = 0;
            windowFade[slot] = 0;
        }
    }
    return 1;
}

// Finds the named map entry for `map` whose script flags pass, copies its name, file and title into the buffers and
// its value into `value`. Returns 1, or 0 when none passes. Called by the map loader (EN 0x605380).
int Game::FindNamedEntry(int map, char *name, char *file, char *title, int unused1, int unused2, int unused3, int *value)
{
    int i = 0;
    int count = namedEntryCount;
    if (count > 0)
    {
        GameNamedEntry *entry = namedEntries;
        do
        {
            if (entry->map == map)
            {
                if (TriggerApplies((GameTrigger *)entry, 1))
                {
                    strcpy(name, namedEntries[i].name);
                    strcpy(file, namedEntries[i].file);
                    strcpy(title, namedEntries[i].title);
                    *value = namedEntries[i].value;
                    return 1;
                }
                count = namedEntryCount;
            }
            i++;
            entry++;
        } while (i < count);
    }
    return 0;
}
