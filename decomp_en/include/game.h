// The Game object of the EN build (JP: decomp/include/game.h). Only the members EN code has used so far are named; the
// offsets are proven by matched EN functions (the layout equals the JP one up to the script flags). This is the one
// declaration of Game, GameMenuItems and GameNamedEntry in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_GAME_H
#define GURUMIN_EN_GAME_H

// Number of menu window slots a Game keeps open at once.
const int MENU_SLOT_COUNT = 3;
// Returned by Game::FindSlot when the window id is not open.
const int MENU_SLOT_NONE = MENU_SLOT_COUNT;
// Entries per window table: 0x80 menu entries, then the result pointer and one more.
const int MENU_RESULT = 0x80;
const int MENU_ENTRY_COUNT = 0x82;
// Bytes of the per-window flag arrays (cursorRemembered, savedCursor).
const int MENU_FLAG_BYTES = 0x258;
// Number of script flags (index 0 is always set).
const int SCRIPT_FLAG_COUNT = 4000;
// Highest valid script flag index.
const int SCRIPT_FLAG_LAST = SCRIPT_FLAG_COUNT - 1;
// Number of named map table entries.
const int NAMED_ENTRY_COUNT = 0x100;
// Value returned by Game::FindNamedConstant for an unknown name.
const int CONSTANT_NOT_FOUND = -1;
// Menu close sound id passed to Game::menuSoundCallback.
const int MENU_SOUND_CLOSE = 2;

// Per-window entry table: a pointer to the variable an entry sets, then (entries[MENU_RESULT]) the pointer that
// receives the window's chosen result.
struct GameMenuItems
{
    int *entries[MENU_ENTRY_COUNT];
};

// Map table entry (0xe8 bytes): map number, four script-flag conditions and the map's names.
struct GameNamedEntry
{
    int map;                    // 0x00: map number (9999 = any)
    int flags[4];               // 0x04: script flags that must pass
    char name[0x20];            // 0x14: map name used by scripts
    char m_0034[0xe8 - 0x34];   // 0x34: file, title and values (not used by EN code yet)
};

// The game object: menu windows, the script variable tables and the script flags.
struct Game
{
    char m_0000[0xa4];                      // 0x000: textures and HUD state (not used by EN code yet)
    int windowId[MENU_SLOT_COUNT];          // 0x0a4: id of the menu window in each slot
    char m_00b0[0x110 - 0xb0];              // 0x0b0: window geometry (not used by EN code yet)
    int windowOpen[MENU_SLOT_COUNT];        // 0x110: 1 while the window slot is open
    char m_011c[0x140 - 0x11c];             // 0x11c: not used by EN code yet
    int windowCursor[MENU_SLOT_COUNT];      // 0x140: cursor position in the window
    char m_014c[0x1dc - 0x14c];             // 0x14c: not used by EN code yet
    char cursorRemembered[MENU_FLAG_BYTES]; // 0x1dc: per window id: non-zero when its cursor is remembered
    char savedCursor[MENU_FLAG_BYTES];      // 0x434: per window id: the remembered cursor position
    char m_068c[0x579c - 0x68c];            // 0x68c: window labels (not used by EN code yet)
    GameMenuItems menuItems[MENU_SLOT_COUNT];  // 0x579c: per-window entries and result pointer
    char m_5db4[0x7654 - 0x5db4];           // 0x5db4: entry kinds and values (not used by EN code yet)
    void (*captionCallback[MENU_SLOT_COUNT])(int cursor, char *caption);  // 0x7654: writes the caption of the entry under the cursor
    void (*menuSoundCallback)(int sound);   // 0x7660: plays a menu sound (MENU_SOUND_*)
    char m_7664[0x95e14 - 0x7664];          // 0x7664: menu models and map tables (not used by EN code yet)
    GameNamedEntry namedEntries[NAMED_ENTRY_COUNT];  // 0x95e14: named map table
    int namedEntryCount;                    // 0xa4614: entries in use
    char m_a4618[0x3ee694 - 0xa4618];       // 0xa4618: map data and scripts (not used by EN code yet)
    int scriptFlags[SCRIPT_FLAG_COUNT];     // 0x3ee694: script flags, saved with the game

    int FindSlot(int id);                           // inline: slot of open window `id`, MENU_SLOT_NONE if not open
    int TestScriptFlag(int index);                  // EN 0x5ec240 (JP 0x100022b0)
    int SetScriptFlag(int index, int value);        // EN 0x644940 (JP 0x10008420)
    int FindNamedConstant(const char *name);        // EN 0x5e7ba0 (JP FindConstant)
    int SetWindowCallbackValue(int id, void (*callback)(int cursor, char *caption));  // EN 0x643390
    int SetWindowResult(int id, int *result);       // EN 0x6433e0
    int CloseWindow(int id);                        // EN 0x5ca0e0
    int SetWindowCursor(int id, int value);         // EN 0x6465e0
};

// Slot of the open window `id`, or MENU_SLOT_NONE (inline in the original, expanded in every window function).
inline int Game::FindSlot(int id)
{
    int slot = 0;
    do
    {
        if (windowOpen[slot] != 0 && windowId[slot] == id)
            break;
        slot++;
    } while (slot < MENU_SLOT_COUNT);
    return slot;
}

#endif
