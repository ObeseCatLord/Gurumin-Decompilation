// Enemy waves (EN build; JP: decomp/src/enemy_spawn.cpp). Declared once here.
#ifndef GURUMIN_EN_WAVE_UPDATE_H
#define GURUMIN_EN_WAVE_UPDATE_H

#include "char.h"

// Per-slot value set to -1 when a wave brings the enemy in (other code stores script values such as -10001 here). EN 0x009145e0 (JP DAT_10453008).
extern "C" int g_charScriptValue[CHAR_SLOT_COUNT];
// Per-slot variant: the slot number modulo 5, set when an enemy is put in play. EN 0x0091d690 (JP DAT_1021b4c8).
extern "C" char g_charVariant[CHAR_SLOT_COUNT];

// Goes through the four wave groups: clears finished enemies from the grid and counts the living ones; while fewer than
// two enemies of a group are in the world, brings in the waiting one that is first in line (flash effect, turned to the
// player). Stores the number of living wave enemies in g_waveEnemiesAlive. Returns 0. EN 0x6d6580 (JP 0x10049650).
// Called by the scene frame. C++ linkage (the Matrix4 local has an unwind table).
int Wave_Update(void);

#endif
