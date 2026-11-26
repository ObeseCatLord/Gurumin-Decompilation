// Debris pieces of the EN build: the slot tables and the spawn functions (JP: decomp/include/debris.h, char_vars.h).
// This is the one declaration of the debris tables in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_DEBRIS_H
#define GURUMIN_EN_DEBRIS_H

#include "vec4.h"
#include "node.h"

// Debris slots: 0..DEBRIS_SIMPLE_SLOTS - 1 hold simple pieces, the rest thrown pieces.
const int DEBRIS_SLOTS = 0x100;
const int DEBRIS_SIMPLE_SLOTS = 0x80;
// Range of the random sideways velocity of a simple piece (0..5 each way) and its upward speed.
const int DEBRIS_SIDEWAYS_RANGE = 6;
const float DEBRIS_UPWARD_SPEED = 20.0f;
// Range of the random spin of a new piece.
const int DEBRIS_SPIN_RANGE = 100;

extern "C" int g_debrisType[DEBRIS_SLOTS];            // EN 0x0091bc70 (JP DAT_1030bdf8): piece type, 0 = free slot
extern "C" Vec4 g_debrisPosition[DEBRIS_SLOTS];       // EN 0x0147a248 (JP DAT_10da9400): where the piece is
extern "C" char g_debrisSettled[DEBRIS_SLOTS];        // EN 0x0091ca00 (JP DAT_1021b628): cleared on spawn
extern "C" unsigned char g_debrisBounces[DEBRIS_SLOTS];   // EN 0x0091cb00 (JP DAT_104677a0): bounces in a row, cleared on spawn
extern "C" char g_debrisHeld[DEBRIS_SLOTS];           // EN 0x0091c070 (JP DAT_102195f0): cleared on spawn
extern "C" int g_debrisSpin[DEBRIS_SLOTS];            // EN 0x0091c170 (JP DAT_104465f8): spin, rotation in tenths of a radian
extern "C" int g_debrisLife[DEBRIS_SLOTS];            // EN 0x0091c5d8 (JP DAT_104469f8): frames left (0 for the simple pieces)
extern "C" Vec4 g_debrisVelocity[DEBRIS_SLOTS];       // EN 0x01337528 (JP DAT_10c66ef0): movement per frame
extern "C" Vec4 g_debrisScale[DEBRIS_SLOTS];          // EN 0x00dac980 (JP DAT_10866df8): model scale of the piece

extern "C" int g_debrisColour[DEBRIS_SLOTS];          // EN 0x0091ce00 (JP DAT_10446eb0): colour of a thrown piece
extern "C" int g_characterColour[];                   // EN 0x009e2f40 (JP DAT_10467288): colour of each character slot's model

extern "C" Node g_debrisModels[];                     // EN 0x00dae1c0 (JP DAT_104d2a08): the model of each piece type (the table is shared with the map objects and preloaded models)
extern "C" int g_debrisDrawColour;                    // EN 0x021a34ac (JP DAT_10f655b4): colour index the pieces draw with (0 = none); set around the draw of a thrown piece

// Draws every used debris piece. Returns 0. EN 0x5ce240 (JP 0x1000e340).
int Debris_Draw(void);

// Starts a thrown piece of type `type` or `type + 1` at (x, y, z) in the first free thrown slot, along `direction`,
// with the colour and model scale of character `owner`. Returns 1, or 0 when the slots are all taken. EN 0x5a2190.
extern "C" int Debris_SpawnFromCharacter(float x, float y, float z, const Vec4 *direction, int type, int owner);
// Like Debris_SpawnFromCharacter with exactly `type`, colour `colour`, smaller turns and full scale. EN 0x5a2bd0.
extern "C" int Debris_SpawnFrom(float x, float y, float z, const Vec4 *direction, int type, int colour);

// Starts a simple piece of type `type` at (x, y, z) in the first free slot below DEBRIS_SIMPLE_SLOTS: random spin, a
// random sideways velocity and 20 upwards, full scale. Returns 1, or 0 when the slots are all taken. EN 0x5a2510.
extern "C" int Debris_Spawn(float x, float y, float z, int type);

#endif
