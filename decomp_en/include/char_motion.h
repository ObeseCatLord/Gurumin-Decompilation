// Character motion and model reload helpers of the EN build (JP: char_motion.cpp, char_turn.cpp). This is the one
// declaration of these functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_CHAR_MOTION_H
#define GURUMIN_EN_CHAR_MOTION_H

#include "char.h"

// Slots reloaded by Chars_ReloadModels (the object slots start at OBJECT_SLOT_FIRST).
const int RELOAD_SLOT_COUNT = OBJECT_SLOT_FIRST;
// Argument of Model_ApplyCommands meaning "reset".
const int MODEL_COMMANDS_RESET = -1;

// Applies the pending commands of character `index` to its model. EN 0x643440 (JP 0x10012220).
extern "C" int Model_ApplyCommands(int index, int value);

// Loads motion file `path` on character `index` and plays frames start..end; once = 0 loops. Returns 0. EN 0x606f50.
extern "C" int Char_PlayMotionFile(int index, const char *path, int start, int end, int once);
// Reloads the model of every character slot still in the world. EN 0x6432f0.
extern "C" void Chars_ReloadModels(void);
// Pushes character `index` away from `point` with `strength`. Returns 0. EN 0x602b00.
extern "C" int Char_PushAwayFrom(int strength, char flag, int index, const Vec4 *point);

#endif
