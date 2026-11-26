// Save files of the EN build (JP: decomp/src/save_slot.cpp).
#ifndef GURUMIN_EN_SAVE_SLOT_H
#define GURUMIN_EN_SAVE_SLOT_H

// Loads the save file `path` (`name` is the slot name, not used); returns 1, or 0 when it is missing or does not descramble. EN 0x640040 (JP 0x1003b2c0).
extern "C" int Save_Load(const char *path, const char *name);
// Loads save file `name` from the SaveData folder next to the program. EN 0x6403e0 (JP 0x1005cdc0).
extern "C" int Save_LoadSlot(const char *name);

// Slots per save page: the file of slot n on page p is "gdata%02d.sav" with p * SAVE_SLOTS_PER_PAGE + n.
const int SAVE_SLOTS_PER_PAGE = 7;
// Size in bytes of the pixel buffer the preview loader fills.
const int SAVE_PREVIEW_BYTES = 0x30000;
// Edge in pixels of the square preview texture.
const int SAVE_PREVIEW_SIZE = 0x100;
extern "C" int g_savePage;           // EN 0x00b493e4: the page of save slots the load menu shows
extern "C" int g_savePreviewSlot;    // EN 0x0092f274: the slot whose preview picture g_fadePanelTexture holds

// Reads the preview picture of save file `name` (a file of the SaveData folder or of the Steam cloud) into `pixels`
// (SAVE_PREVIEW_BYTES). Returns non-zero on success. EN 0x63ee60 (not ported yet).
extern "C" int Save_ReadPreview(const char *name, unsigned char *pixels);
// Makes g_fadePanelTexture the preview of slot `slot` of the current page (an empty texture when the save has none);
// does nothing when it already is. Returns 0. EN 0x62e3c0 (see save_preview.cpp; no direct caller).
extern "C" int SavePreview_Show(int slot);

// Reads the summary of save file `name` that the load menu shows (from the Steam cloud, else the SaveData folder): *map
// gets the map number, *playTime the play time, `playerName` the player's name ("Popon" in costume 6) and, when `mode`
// is not 0, *mode the chapter (3 and 5 for the two extra modes, 4 once the map is 1000). Returns 1, or 0 when the file
// is missing or does not descramble. C++ linkage (the GameFile local has an unwind table). EN 0x63e9f0 (JP 0x10010d10).
int Save_ReadSlotInfo(const char *name, int *map, int *playTime, char *playerName, int *mode);

#endif
