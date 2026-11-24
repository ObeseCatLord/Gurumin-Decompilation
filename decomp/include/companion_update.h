// The player's companions' updates (companion_update.cpp) and the status clamp (char_status.cpp), run by the characters
// that own companions (the player, the BS_006 boss).
#ifndef GURUMIN_COMPANION_UPDATE_H
#define GURUMIN_COMPANION_UPDATE_H

// 0x100b0250, 0x100b0880, 0x100b0f00 (C++ linkage: the originals have an EH frame for their matrices): keep
// companion 0, 1 or 2 of character `index` (slot index + 2 + k) in range and run its revival.
int Companion0_Update(int index);
int Companion1_Update(int index);
int Companion2_Update(int index);

// 0x1001a840 (Char_ClampStatus, char_status.cpp): clamps the counter and level of character `index`.
extern "C" int FUN_1001a840(int index);

#endif
