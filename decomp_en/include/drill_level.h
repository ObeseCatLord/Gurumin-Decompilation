// The drill gauge and level (EN build; JP: decomp/src/drill_level.cpp). This is the one declaration of these globals and
// functions in decomp_en: include this header, never redeclare them. (The gauge is g_guardedValue7 and the level
// g_guardedValue6, declared in anti_tamper.h.)
#ifndef GURUMIN_EN_DRILL_LEVEL_H
#define GURUMIN_EN_DRILL_LEVEL_H

#include "node.h"

// Number of claws ("@tume01" attachments) the player gets at the full level.
const int DRILL_CLAW_COUNT = 4;

extern "C" char g_drillLevelChanged;                      // EN 0x009e3387 (JP DAT_104c00bd): set to 1 when the drill level changes (a frame counter once set)
extern "C" char g_drillLevelWentDown;                     // EN 0x00a1513a (JP DAT_104c01b4): 1 = the drill level just went down, 0 = it went up
extern "C" Node *g_drillClaws[DRILL_CLAW_COUNT];          // EN 0x00a13924 (JP DAT_102559f4): the four "@tume01" attachments that follow the player

// Clamps the drill gauge and moves the level to match it. Returns 0. EN 0x5a9fe0 (JP 0x10057ee0).
int UpdateDrillLevel(void);

#endif
