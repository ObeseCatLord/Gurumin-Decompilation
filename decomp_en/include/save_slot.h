// Save files of the EN build (JP: decomp/src/save_slot.cpp).
#ifndef GURUMIN_EN_SAVE_SLOT_H
#define GURUMIN_EN_SAVE_SLOT_H

// Loads the save file `path` (`name` is the slot name, not used); returns 1, or 0 when it is missing or does not descramble. EN 0x640040 (JP 0x1003b2c0).
extern "C" int Save_Load(const char *path, const char *name);
// Loads save file `name` from the SaveData folder next to the program. EN 0x6403e0 (JP 0x1005cdc0).
extern "C" int Save_LoadSlot(const char *name);

#endif
