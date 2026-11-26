// The swinging character (EN build; JP: decomp/include/swing_platform.h) and two per-slot tables it reads. Declared once
// here. Used by src/swing_platform.cpp.
#ifndef GURUMIN_EN_SWING_PLATFORM_H
#define GURUMIN_EN_SWING_PLATFORM_H

#include "char.h"

// Non-zero while the character takes part in the frame (it wakes up near the player); 0 = asleep. EN 0x00bc8ca0 (JP DAT_104483e8).
extern "C" int g_charAwake[CHAR_SLOT_COUNT];
// Per-slot value the behaviours clear every frame; its meaning is not recovered. EN 0x00920060 (JP DAT_10218fc0).
extern "C" int g_charClearedEachFrame[CHAR_SLOT_COUNT];

// One frame of the swinging character `index` (kind 0x0e): swings it on an arm round a pivot and turns its layer node with
// the swing. Returns 1 when the character is gone, else 0. EN 0x666bf0 (JP 0x10025ce0).
int Char_SwingPlatform(int index);

#endif
