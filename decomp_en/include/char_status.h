// Per-character status tables of the EN build that Char_ClampStatus reads (declared once here; JP equivalents in
// decomp/include/char_vars.h and char_tables.h). The tables are indexed by the slot number plus CHAR_STATUS_SKEW as the
// original code does (the compiled code shares one index register between them).
#ifndef GURUMIN_EN_CHAR_STATUS_H
#define GURUMIN_EN_CHAR_STATUS_H

#include "char.h"
#include "vec4.h"

// Entries the status tables are shifted by: slot `i` lives at index i + CHAR_STATUS_SKEW.
const int CHAR_STATUS_SKEW = 3;

// Slots the character records are shifted by in Char_ClampStatus (the status index plus two more).
const int CHAR_RECORD_SKEW = 2;

// Highest counter and level a character's status keeps.
const int CHAR_COUNTER_MAX = 15;
const int CHAR_LEVEL_MAX = 5;

// Character mode that Char_ClampStatus handles, and the display states it chooses between.
const int CHAR_MODE_FOLLOW = 7;
const int CHAR_DISPLAY_NEAR = 4;
const int CHAR_DISPLAY_FAR = 6;

// A character's near counter above which the far display state is used.
const int CHAR_NEAR_LIMIT = 4;

extern "C" short g_charCounter[];                 // EN 0x009e227c (JP DAT_103153b4): a per-character counter, at most CHAR_COUNTER_MAX
extern "C" int g_charLevel[];                     // EN 0x009e2080 (JP DAT_10219778): a per-character level, at most CHAR_LEVEL_MAX
extern "C" unsigned char g_charFlag[];            // EN 0x009e1bd2 (JP DAT_10315352): a per-character flag, cleared every frame
extern "C" unsigned char g_charWakeValue[];       // EN 0x009e1a12 (JP DAT_102550e2): a per-character value; the larger of it and g_charHitLevel is kept
extern "C" char g_charMode[];                     // EN 0x00915a2a (JP DAT_104686b2): per-character mode (CHAR_MODE_FOLLOW = the mode Char_ClampStatus handles)
extern "C" unsigned char g_charNearCount[];       // EN 0x009e1a92 (JP DAT_1020d4c2): per-character count compared with CHAR_NEAR_LIMIT
extern "C" unsigned char g_charDisplayState[CHAR_SLOT_COUNT];   // EN 0x009e1a90 (JP DAT_1020d4c0): per-character display state (CHAR_DISPLAY_NEAR or CHAR_DISPLAY_FAR)
extern "C" Vec4 g_charOffset[CHAR_SLOT_COUNT];    // EN 0x01336c88 (JP DAT_10f08080): each character's offset from the reference position
extern "C" Vec4 g_referencePosition;              // EN 0x00d74228 (JP DAT_10836c74): the reference position the offsets are measured from

// Clamps character `index`'s counter and level, clears its flag, keeps the larger wake value, marks it defeated when its
// health ran out and otherwise (in mode CHAR_MODE_FOLLOW) picks its display state. Returns 0. EN 0x5a7080 (JP 0x1001a840).
extern "C" int Char_ClampStatus(int index);

#endif
