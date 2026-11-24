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

// Number of map event slots, event trigger slots, queued trigger slots and queued event ids.
const int MAP_EVENT_COUNT = 2000;
const int QUEUED_TRIGGER_COUNT = 2000;
const int TRIGGER_COUNT = 3400;
const int QUEUED_EVENT_COUNT = 0x100;
// GameTrigger::map value meaning "any map".
const int TRIGGER_ANY_MAP = 9999;
// Bytes of the script token buffer.
const int SCRIPT_TOKEN_SIZE = 0x80;

// Event trigger (0xc8 bytes): runs when its map matches and all four script flags pass.
struct GameTrigger
{
    int map;                    // 0x00: map number, TRIGGER_ANY_MAP = any map
    int flags[4];               // 0x04: script flags that must be set (negative = must be clear, 0 = no condition)
    int value;                  // 0x14: event id (for map events)
    char name[0x10];            // 0x18: event name, e.g. "GAMEOVER", "KSKON", "TALK", "OPEN"
    int argument;               // 0x28: what the event reacts to (for example the character or object id)
    char m_002c[0xc8 - 0x2c];   // 0x2c: more event numbers and their labels (not used by EN code yet)
};

// Entries a menu window can hold and the bytes of an entry's label.
const int MENU_ENTRY_SLOTS = 0x80;
const int MENU_LABEL_SIZE = 0x36;
// Values of Game::entryKind.
enum MenuEntryKind
{
    MENU_ENTRY_FLAG = 0,     // sets a flag variable
    MENU_ENTRY_VALUE = 1,    // has a value of its own
    MENU_ENTRY_RANGE = 2,    // sets a value between entryMin and entryMax
    MENU_ENTRY_KIND5 = 5     // like a flag entry, kept apart from the clearing pass
};

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
    char m_00b0[0xe0 - 0xb0];               // 0x0b0: window geometry (not used by EN code yet)
    int lastEntry[MENU_SLOT_COUNT];         // 0x0e0: index of the last entry added to the window
    char m_00ec[0x110 - 0xec];              // 0x0ec: not used by EN code yet
    int windowOpen[MENU_SLOT_COUNT];        // 0x110: 1 while the window slot is open
    char m_011c[0x128 - 0x11c];             // 0x11c: not used by EN code yet
    int windowFlags[MENU_SLOT_COUNT];       // 0x128: per window slot: layout flags (0x18 when opened, the yes/no dialogs set 0x20)
    char m_0134[0x140 - 0x134];             // 0x134: not used by EN code yet
    int windowCursor[MENU_SLOT_COUNT];      // 0x140: cursor position in the window
    char m_014c[0x1dc - 0x14c];             // 0x14c: not used by EN code yet
    char cursorRemembered[MENU_FLAG_BYTES]; // 0x1dc: per window id: non-zero when its cursor is remembered
    char savedCursor[MENU_FLAG_BYTES];      // 0x434: per window id: the remembered cursor position
    char m_068c[0x69c - 0x68c];             // 0x68c: not used by EN code yet
    char labels[MENU_SLOT_COUNT][MENU_ENTRY_SLOTS][MENU_LABEL_SIZE];  // 0x69c: item label of each window entry (empty = free)
    GameMenuItems menuItems[MENU_SLOT_COUNT];  // 0x579c: per-window entries and result pointer
    int entryKind[MENU_SLOT_COUNT][MENU_ENTRY_COUNT];       // 0x5db4: entry kind (MenuEntryKind)
    int entryMax[MENU_SLOT_COUNT][MENU_ENTRY_COUNT];        // 0x63cc: range entries' upper value (-1 for the others)
    int entryMin[MENU_SLOT_COUNT][MENU_ENTRY_COUNT];        // 0x69e4: range entries' lower value
    char m_6ffc[0x7654 - 0x6ffc];           // 0x6ffc: entry highlights (not used by EN code yet)
    void (*captionCallback[MENU_SLOT_COUNT])(int cursor, char *caption);  // 0x7654: writes the caption of the entry under the cursor
    void (*menuSoundCallback)(int sound);   // 0x7660: plays a menu sound (MENU_SOUND_*)
    char m_7664[0x95e14 - 0x7664];          // 0x7664: menu models and map tables (not used by EN code yet)
    GameNamedEntry namedEntries[NAMED_ENTRY_COUNT];  // 0x95e14: named map table
    int namedEntryCount;                    // 0xa4614: entries in use
    char m_a4618[0x15372c - 0xa4618];       // 0xa4618: map character tables (not used by EN code yet)
    GameTrigger triggers[TRIGGER_COUNT];    // 0x15372c: event triggers of the loaded map
    int triggerCount;                       // 0x1f976c: triggers in use
    GameTrigger queuedTriggers[QUEUED_TRIGGER_COUNT];  // 0x1f9770: triggers of queued events
    int queuedTriggerCount;                 // 0x25b1f0: queued triggers in use
    GameTrigger mapEvents[MAP_EVENT_COUNT]; // 0x25b1f4: named map events
    int mapEventCount;                      // 0x2bcc74: map events in use
    char m_2bcc78[0x3ed97c - 0x2bcc78];     // 0x2bcc78: character motions and script sections (not used by EN code yet)
    char *text;                             // 0x3ed97c: script text being tokenised
    int pos;                                // 0x3ed980: read position in `text`
    int end;                                // 0x3ed984: text length
    char token[SCRIPT_TOKEN_SIZE];          // 0x3ed988: the token read last
    int runningEvents[QUEUED_EVENT_COUNT];  // 0x3eda08: queued event ids being run
    int queuedEvents[QUEUED_EVENT_COUNT][2];  // 0x3ede08: queued event ids (> 0) and whether each was handled
    char killed[0x80];                      // 0x3ee608: 1 for each id killed
    int currentMap;                         // 0x3ee688: number of the loaded map
    int m_3ee68c[2];                        // 0x3ee68c: not used by EN code yet
    int scriptFlags[SCRIPT_FLAG_COUNT];     // 0x3ee694: script flags, saved with the game

    int FindSlot(int id);                           // inline: slot of open window `id`, MENU_SLOT_NONE if not open
    int TestScriptFlag(int index);                  // EN 0x5ec240 (JP 0x100022b0)
    int SetScriptFlag(int index, int value);        // EN 0x644940 (JP 0x10008420)
    int FindNamedConstant(const char *name);        // EN 0x5e7ba0 (JP FindConstant)
    int SetWindowCallbackValue(int id, void (*callback)(int cursor, char *caption));  // EN 0x643390
    int SetWindowResult(int id, int *result);       // EN 0x6433e0
    int OpenWindow(int id, int x, int y, int width, int height);   // EN 0x5c9560 (JP 0x100084b0)
    int CloseWindow(int id);                        // EN 0x5ca0e0
    int SetWindowCursor(int id, int value);         // EN 0x6465e0
    int AddFlagEntry(int id, const char *label, int *flag);     // EN 0x5a2840
    int AddKind5Entry(int id, const char *label, int *flag);    // EN 0x5a2970
    int AddRangeEntry(int id, const char *label, int low, int high, int *value);   // EN 0x5a2aa0
    int TriggerApplies(GameTrigger *trigger, int checkMap);   // EN 0x5c0860
    int RegisterEvent(int id);                      // EN 0x5a1f00 (JP FlagOwner::Register, other worker)
    int RaiseKskonEvents(int argument);             // EN 0x63c460
    int RaiseTalkEvents(int argument);              // EN 0x63c7c0
    int RaiseOpenEvents(int argument);              // EN 0x63c730
    int RaiseGameOverEvents();                      // EN 0x63c3e0
    int RaiseDoorEvents(int argument, int on);      // EN 0x63c2c0
    int RaiseLeverEvents(int argument, int on);     // EN 0x63c4f0
    int RaiseFireEvents(int argument, int on);      // EN 0x63c610
    int ReadWordInLine();                           // EN 0x63f0d0
    int ReadWord();                                 // EN 0x63f1f0
    int ForEachActiveTrigger(int (*callback)(GameTrigger *trigger));   // EN 0x5cb610
    int IsQueuedEventReady();                       // EN 0x5e2f00
    int RunQueuedEvents(int (*callback)(GameTrigger *trigger));   // EN 0x5ca890
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


// Tests script flag `index` (4000 flags saved with the game). A negative index tests that the flag is clear. Flag 0
// is always set; out-of-range indexes read as clear (or as set for the negated test). Called by every script condition; inline in the header (the compiler
// expands it in some callers and keeps the out-of-line copy at EN 0x5ec240 for the rest).
inline int Game::TestScriptFlag(int index)
{
    if (index < 0)
    {
        index = -index;
        if (index > SCRIPT_FLAG_LAST)
            return 1;
        return scriptFlags[index] == 0;
    }
    if (index > SCRIPT_FLAG_LAST)
        return 0;
    if (index == 0)
        return 1;
    return scriptFlags[index];
}

// The game object (EN 0x00f445f0; JP DAT_10874848).
extern "C" Game g_game;

#endif
