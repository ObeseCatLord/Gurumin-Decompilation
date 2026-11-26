// The Game object of the EN build (JP: decomp/include/game.h). Only the members EN code has used so far are named; the
// offsets are proven by matched EN functions (the layout equals the JP one up to the script flags). This is the one
// declaration of Game, GameMenuItems and GameNamedEntry in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_GAME_H
#define GURUMIN_EN_GAME_H

struct FontRenderer;   // the text renderer the menus draw with

#include "node.h"

// Base of the game objects: empty, with an out-of-line destructor (EN 0x76f0e0, folded with ~Matrix4 by the linker).
struct Object
{
    ~Object();
};

// Number of menu model nodes a Game owns.
const int MENU_NODE_COUNT = 0x20;

// Number of menu window slots a Game keeps open at once.
const int MENU_SLOT_COUNT = 3;
// Returned by Game::FindSlot when the window id is not open.
const int MENU_SLOT_NONE = MENU_SLOT_COUNT;
// Entries per window table: 0x80 menu entries, then the result pointer and one more.
const int MENU_RESULT = 0x80;
const int MENU_ENTRY_COUNT = 0x82;
// Bytes of the per-window flag arrays (cursorRemembered, savedCursor).
const int MENU_FLAG_BYTES = 0x258;
// The two per-window-id arrays as ints, and how many of them the menu reset clears.
const int MENU_CURSOR_MEMORY_INTS = MENU_FLAG_BYTES * 2 / 4;
const int MENU_CURSOR_CLEAR_INTS = 200;
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
// Menu sound id passed to Game::menuSoundCallback when a window opens.
const int MENU_SOUND_OPEN = 0;
// Initial windowGrow value of a newly opened window slot, and the step every OpenWindow adds to all slots.
const float WINDOW_GROW_START = 0.03f;
const float WINDOW_GROW_STEP = 0.05f;
// Layout flags (windowFlags) of a newly opened window.
const int WINDOW_FLAGS_DEFAULT = 0x18;
// Row count a newly opened window starts with (windowLastRow).
const int WINDOW_ROWS_DEFAULT = 0x80;
// Value of Game::m_0104 after a window opens.
const int WINDOW_OPEN_STATE = 10;

// Number of map event slots, event trigger slots, queued trigger slots and queued event ids.
const int MAP_EVENT_COUNT = 2000;
const int QUEUED_TRIGGER_COUNT = 2000;
const int TRIGGER_COUNT = 3400;
const int QUEUED_EVENT_COUNT = 0x100;
// GameTrigger::map value meaning "any map".
const int TRIGGER_ANY_MAP = 9999;
// Bytes of the script token buffer.
const int SCRIPT_TOKEN_SIZE = 0x80;

// Ids a DEAD event waits for besides its argument.
const int TRIGGER_ID_COUNT = 5;
// Highest killed-list index (ids above it count as the last entry).
const int KILLED_LAST = 0x5f;
// Event trigger (0xc8 bytes): runs when its map matches and all four script flags pass.
struct GameTrigger
{
    int map;                    // 0x00: map number, TRIGGER_ANY_MAP = any map
    int flags[4];               // 0x04: script flags that must be set (negative = must be clear, 0 = no condition)
    int value;                  // 0x14: event id (for map events)
    char name[0x10];            // 0x18: event name, e.g. "GAMEOVER", "KSKON", "TALK", "OPEN"
    union
    {
        struct
        {
            int argument;               // 0x28: what the event reacts to (for example the character or object id)
            int ids[TRIGGER_ID_COUNT];  // 0x2c: more ids a "DEAD" event waits for (all of them and `argument` must be killed)
        };
        int watched[TRIGGER_ID_COUNT + 1];   // 0x28: argument and ids as one list (the DEAD event checks all six)
    };
    char m_0040[0xc8 - 0x40];   // 0x40: more event numbers and their labels (not used by EN code yet)
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
    char file[0x20];            // 0x34: file name of the map
    char title[0x20];           // 0x54: title of the map
    char m_0074[0xa4 - 0x74];   // 0x74: not used by EN code yet
    int value;                  // 0xa4: number FindNamedEntry reports
    char m_00a8[0xe8 - 0xa8];   // 0xa8: not used by EN code yet
};

// The game object: menu windows, the script variable tables and the script flags.
struct Game : Object
{
    int m_0000;                             // 0x000: reset to 0 with the menus (not yet named)
    int m_0004;                             // 0x004: reset to 0 with the menus (not yet named)
    int texMenu1;                           // 0x008: texture slot of the menu sheet 1 (digits, cursor, debug font)
    int texMenu4;                           // 0x00c: texture slot of the alternative panel frame
    int m_0010;                             // 0x010: texture slot, -1 (unused)
    int texMenu2;                           // 0x014: texture slot of menu sheet 2
    int texMenu5;                           // 0x018: texture slot of menu sheet 5
    int texBalloon5;                        // 0x01c: texture slot of speech balloon 5
    int texBalloon6;                        // 0x020: texture slot of speech balloon 6
    int texFrame;                           // 0x024: texture slot of the frame sheet
    char m_0028[0x2c - 0x28];               // 0x028: not used by EN code yet
    int texMap01;                           // 0x02c: texture slot of map sheet 1
    int texMap02;                           // 0x030: texture slot of map sheet 2
    int texUdChip;                          // 0x034: texture slot of the up/down chip sheet
    int texWaku01;                          // 0x038: texture slot of the HUD frame sheet (digit strip rows 0, 16 and 64-80)
    int texWaku02;                          // 0x03c: texture slot of the second HUD sheet (digit strip rows 16-48)
    int texWaku03;                          // 0x040: texture slot of the third HUD sheet
    int texWaku04;                          // 0x044: texture slot of the fourth HUD sheet
    int texIcon01;                          // 0x048: texture slot of the item icon sheet
    int texWaku05;                          // 0x04c: texture slot of the fifth HUD sheet
    char m_0050[0x7c - 0x50];               // 0x050: not used by EN code yet
    int texBalloon7;                        // 0x07c: texture slot of speech balloon 7
    int texMenu15;                          // 0x080: texture slot of the panel fill
    int texMenu14;                          // 0x084: texture slot of the panel frame
    int activeWindow;                       // 0x088: window slot that has the cursor (reset to 0 with the menus)
    int windowVisible[MENU_SLOT_COUNT];     // 0x08c: non-zero while the window slot is shown
    int windowFade[MENU_SLOT_COUNT];        // 0x098: fade-in counter of the window slot (0 to 32)
    int windowId[MENU_SLOT_COUNT];          // 0x0a4: id of the menu window in each slot
    int windowX[MENU_SLOT_COUNT];           // 0x0b0: left edge of the window slot
    int windowY[MENU_SLOT_COUNT];           // 0x0bc: top edge of the window slot
    int windowWidth[MENU_SLOT_COUNT];       // 0x0c8: width of the window slot
    int windowHeight[MENU_SLOT_COUNT];      // 0x0d4: height of the window slot
    int lastEntry[MENU_SLOT_COUNT];         // 0x0e0: index of the last entry added to the window
    int windowRefresh[MENU_SLOT_COUNT];     // 0x0ec: set to 1 by RequestWindowRefresh for an open window slot (reader not recovered)
    int m_00f8[MENU_SLOT_COUNT];            // 0x0f8: per window slot: cleared when the window opens (reader not recovered)
    int m_0104;                             // 0x104: set to 10 when a window opens (reader not recovered)
    char m_0108[0x110 - 0x108];             // 0x108: not used by EN code yet
    int windowOpen[MENU_SLOT_COUNT];        // 0x110: 1 while the window slot is open
    int windowLastRow[MENU_SLOT_COUNT];     // 0x11c: per window slot: number of rows minus one (SetWindowRows)
    int windowFlags[MENU_SLOT_COUNT];       // 0x128: per window slot: layout flags (0x18 when opened, the yes/no dialogs set 0x20)
    int m_0134[MENU_SLOT_COUNT];            // 0x134: per window slot: cleared when the window opens (reader not recovered)
    int windowCursor[MENU_SLOT_COUNT];      // 0x140: cursor position in the window
    char m_014c[0x158 - 0x14c];             // 0x14c: not used by EN code yet
    int m_0158[MENU_SLOT_COUNT];            // 0x158: per window slot: cleared when the window opens (reader not recovered)
    int m_0164[MENU_SLOT_COUNT];            // 0x164: per window slot: cleared when the window opens (reader not recovered)
    int m_0170[MENU_SLOT_COUNT];            // 0x170: per window slot: column of the cursor in a grid window (times m_0164 = x offset)
    int m_017c[MENU_SLOT_COUNT];            // 0x17c: per window slot: row height factor of a grid window (times windowFlags = y offset)
    int *pageChanged[MENU_SLOT_COUNT];      // 0x188: per window slot: flag the page-number owner sets to 1 when the page changes (SetWindowLayout)
    int *pageNumber[MENU_SLOT_COUNT];       // 0x194: per window slot: the zero-based page number the window shows (SetWindowLayout)
    int layoutB[MENU_SLOT_COUNT];           // 0x1a0: per window slot: second layout value of SetWindowLayout
    int layoutA[MENU_SLOT_COUNT];           // 0x1ac: per window slot: first layout value of SetWindowLayout
    int *shownValue[MENU_SLOT_COUNT];       // 0x1b8: per window slot: the value drawn at (shownValueX, shownValueY) (SetWindowShownValue)
    int shownValueX[MENU_SLOT_COUNT];       // 0x1c4: x the value is drawn at
    int shownValueY[MENU_SLOT_COUNT];       // 0x1d0: y the value is drawn at
    union                                   // 0x1dc: the per-window-id cursor memory, cleared together when the menus reset
    {
        struct
        {
            char cursorRemembered[MENU_FLAG_BYTES]; // 0x1dc: per window id: non-zero when its cursor is remembered
            char savedCursor[MENU_FLAG_BYTES];      // 0x434: per window id: the remembered cursor position
        };
        int cursorMemory[MENU_CURSOR_MEMORY_INTS];   // the same bytes as ints (the reset clears the first MENU_CURSOR_CLEAR_INTS)
    };
    char m_068c[MENU_SLOT_COUNT];           // 0x68c: per window slot: cleared when the window opens (reader not recovered)
    char m_068f[0x690 - 0x68f];             // 0x68f: not used by EN code yet
    int m_0690;                             // 0x690: reset to 0 when the menu textures load (not yet named)
    char m_0694[0x69c - 0x694];             // 0x694: not used by EN code yet
    char labels[MENU_SLOT_COUNT][MENU_ENTRY_SLOTS][MENU_LABEL_SIZE];  // 0x69c: item label of each window entry (empty = free)
    GameMenuItems menuItems[MENU_SLOT_COUNT];  // 0x579c: per-window entries and result pointer
    int entryKind[MENU_SLOT_COUNT][MENU_ENTRY_COUNT];       // 0x5db4: entry kind (MenuEntryKind)
    int entryMax[MENU_SLOT_COUNT][MENU_ENTRY_COUNT];        // 0x63cc: range entries' upper value (-1 for the others)
    int entryMin[MENU_SLOT_COUNT][MENU_ENTRY_COUNT];        // 0x69e4: range entries' lower value
    int menuTexturesLoaded;                 // 0x6ffc: set to 1 when the menu textures are loaded
    char m_7000[0x7630 - 0x7000];           // 0x7000: entry highlights (not used by EN code yet)
    int cursorTargetX;                      // 0x7630: x the menu cursor moves towards
    int cursorTargetY;                      // 0x7634: y the menu cursor moves towards
    int cursorX;                            // 0x7638: x the menu cursor is drawn at
    int cursorY;                            // 0x763c: y the menu cursor is drawn at
    int m_7640;                             // 0x7640: reset to 0 with the menus (not yet named)
    int m_7644;                             // 0x7644: reset to 0 with the menus (not yet named)
    int m_7648;                             // 0x7648: reset to 0 with the menus (not yet named)
    int m_764c;                             // 0x764c: reset to 0 with the menus (not yet named)
    int m_7650;                             // 0x7650: reset to 0 with the menus (not yet named)
    void (*captionCallback[MENU_SLOT_COUNT])(int cursor, char *caption);  // 0x7654: writes the caption of the entry under the cursor
    void (*menuSoundCallback)(int sound);   // 0x7660: plays a menu sound (MENU_SOUND_*)
    FontRenderer *textRenderer;             // 0x7664: the text renderer the menus draw with
    float windowGrow[MENU_SLOT_COUNT];      // 0x7668: per window slot: starts at WINDOW_GROW_START when the window opens, +0.05 per OpenWindow of any slot
    char m_7674[0x8868 - 0x7674];           // 0x7674: not used by EN code yet
    Node menuNodes[MENU_NODE_COUNT];        // 0x8868: nodes of the menu models
    char m_39c68[0x95e14 - 0x39c68];        // 0x39c68: menu tables and map tables (not used by EN code yet)
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
    int SetCursorRemembered(int id, char value);    // EN 0x6458c0: sets cursorRemembered[id]
    int SetValue7650(int value);                   // EN 0x643370: stores m_7650 (not named yet)
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
    int MarkKilled(int id);                         // EN 0x63c170 (JP 0x10054770): marks `id` killed and raises the DEAD events whose ids are all killed
    int RaiseDoorEvents(int argument, int on);      // EN 0x63c2c0
    int RaiseLeverEvents(int argument, int on);     // EN 0x63c4f0
    int RaiseFireEvents(int argument, int on);      // EN 0x63c610
    int IsAnyWindowShown();                         // EN 0x5f4a90: 1 when a menu window is open or still fading out
    int SetWindowFlags(int id, int flags);          // EN 0x644c50: sets the layout flags of open window `id`
    int RequestWindowRefresh(int id);               // EN 0x646190 (EN-only, game_menu_refresh.cpp): flags open window `id` for refresh
    int SetWindowRows(int id, int rows);            // EN 0x645a10: stores the row count of open window `id` (as rows - 1)
    int ClearEventQueues();                         // EN 0x63f660: empties the running and the queued event lists
    int SetWindowLayout(int id, int a, int b, int *pageNumber, int *pageChanged);   // EN 0x646310 (JP SetWindowLayout, game_window_layout.cpp)
    int SetWindowShownValue(int id, int x, int y, int *value);   // EN 0x6461e0 (JP SetWindowLayout2, game_window_layout.cpp): the value a window draws
    int SetActiveWindowRect(int x, int y, int width, int height);   // EN 0x63f6e0 (EN-only, game_window_layout.cpp): moves and resizes the window with the cursor
    int ResetWindowSlots();                         // EN 0x5c9f70 (EN-only, game_window_layout.cpp): closes all window slots and clears their entry kinds
    int ClearFadedWindows();                        // EN 0x5c2b80 (EN-only, game_window_layout.cpp): zeroes open/shown/fade of a slot once any of the three is set
    int FindNamedEntry(int map, char *name, char *file, char *title, int unused1, int unused2, int unused3, int *value);   // EN 0x5edac0 (game_window_layout.cpp): copies the named entry of `map` the flags allow
    int ReadWordInLine();                           // EN 0x63f0d0
    int ReadWord();                                 // EN 0x63f1f0
    int ForEachActiveTrigger(int (*callback)(GameTrigger *trigger));   // EN 0x5cb610
    int IsQueuedEventReady();                       // EN 0x5e2f00
    int RunQueuedEvents(int (*callback)(GameTrigger *trigger));   // EN 0x5ca890
    ~Game();                                                               // EN 0x5a1150
    void ResetMenus();                                                     // EN 0x63f480 (JP 0x10007f20)
    int LoadMenuTextures(FontRenderer *renderer);                          // EN 0x5f2970 (JP LoadMenuTextures)
    int DrawPanel(int x, int y, int w, int h, float z, float alpha);       // EN 0x5e27f0 (JP DrawPanel)
    int DrawPanelAlt(int x, int y, int w, int h, float z, float alpha);    // EN 0x5e2730
    int DrawCursor(int x, int y, float fade, int frame);                   // EN 0x5cd420
    int DrawWindowMarks(int frame);                                        // EN 0x5d9430 (JP 0x100092c0): draws the scroll marks and the cursor of the three menu windows
    int DrawBalloon(int x, int y, int lit, int rows, int a, int b, float fade);   // EN 0x5e2570 (JP 0x10008910): draws a speech balloon of `rows` middle rows
    int MoveCursor();                                                      // EN 0x5a9e40 (JP 0x10009050): moves the menu cursor towards its target
    int DrawDebugText(const char *text, int x, int y);   // EN 0x5e05b0 (JP 0x100091d0): draws `text` with the 8x16 debug font of menu sheet 1
    int DrawLabelledNumber(int value, int x, int y, float z, float fade, float scale, float alpha);  // EN 0x5dec10
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
