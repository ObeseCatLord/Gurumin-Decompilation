// Shaga pieces (thrown equipment pieces) of the EN build (JP: decomp/src/shaga_spawn.cpp, char_vars.h).
#ifndef GURUMIN_EN_SHAGA_H
#define GURUMIN_EN_SHAGA_H

#include "vec4.h"

struct Node;

// Number of shaga piece slots.
const int SHAGA_SLOTS = 0x28;
// Value of the two target tables for "no target".
const int SHAGA_NO_TARGET = -1;

extern "C" Node *g_shagaNode[SHAGA_SLOTS];     // EN 0x0091ef70 (JP DAT_10446e00): node of each piece (0 = slot free)
extern "C" float g_shagaRadius[SHAGA_SLOTS];  // EN 0x0091f370 (JP DAT_1030cd98): radius of the piece
extern "C" int g_shagaTargetA[SHAGA_SLOTS];    // EN 0x0091f440 (JP DAT_1042db08): first target of the piece (-1 = none)
extern "C" int g_shagaTargetB[SHAGA_SLOTS];    // EN 0x0091f210 (JP DAT_1021d018): second target of the piece (-1 = none)
extern "C" int g_shagaCounter[SHAGA_SLOTS];    // EN 0x0091f538 (JP DAT_10254798): counter of the piece
extern "C" int g_colourPlugFrames;             // EN 0x00a207a8 (JP DAT_104bf8a4): blend frames passed to RestartColourPlugs

extern "C" char g_shagaSizeClass[SHAGA_SLOTS];  // EN 0x0091f508 (JP DAT_1020da24): size class of the piece (1 normal, 2 = 1.2 times, 4 = twice)
extern "C" char g_shagaSettled[SHAGA_SLOTS];    // EN 0x0091f4e0 (JP DAT_102194c8): cleared when the piece is placed
extern "C" Vec4 g_shagaVelocity[SHAGA_SLOTS];   // EN 0x015e9fb8 (JP DAT_10da9160): velocity of the piece

// An equipment piece of model `model` (radius `radius`) breaks off character `index`: starts from `matrix` turned at
// random about z at `position`, launched away from the character's facing with `force`; `scaled` sizes it by the
// character's base kind and `tired` first raises the guarded stamina value by 2. Returns the slot, -1 when all are
// used, 0 without a map. EN 0x64b6d0 (JP 0x10042140).
struct Matrix4;
int Shaga_BreakOff(int index, const char *model, int radius, const Vec4 *position, const Matrix4 *matrix, int scaled,
                   float force, int tired);

// Creates a shaga piece (model `model`, radius `radius`) at `position`; returns the slot or -1. EN 0x64b210 (JP 0x100425f0).
extern "C" int Shaga_Create(const Vec4 *position, const char *model, int radius, float d, float e);
// Creates a piece and sets it to stop. EN 0x64b640 (JP 0x10062750).
extern "C" int Shaga_Spawn(const Vec4 *position, const char *model, int radius, float d, float e);

// Moves piece `index` out of the world collision: its centre is tested as a sphere of its radius plus one and, while it hits
// something, moved along the hit normal, a few times at most. Returns 0. EN 0x64bb90 (JP 0x1001af10).
int Shaga_PushOut(int index);

#endif
