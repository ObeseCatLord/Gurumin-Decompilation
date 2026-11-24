// The per-kind character update (char_update.cpp, 0x100f9670): runs once a frame for every character slot.
#ifndef GURUMIN_CHAR_UPDATE_H
#define GURUMIN_CHAR_UPDATE_H

extern "C" int DAT_10c635c8;   // 1 once the enemy-slot table DAT_10254090 has been filled by the first update

// 0x100f9670 (Char_UpdateByKind; C++ linkage: the original has an EH frame)
int Char_UpdateByKind(int index);

// 0x100f5a80: the scene frame (Scene_Frame).
#include "scene_frame.h"

#endif
