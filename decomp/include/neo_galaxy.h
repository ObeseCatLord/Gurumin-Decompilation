// The Neo Galaxy boss (neo_galaxy_update.cpp): a per-kind behaviour (CharSlot::kind 0x32) of the scene slot loops
// 0x100f9670 and 0x100f5a80. Its three bodies are the slots 0x3a, 0x3c and 0x3e.
#ifndef GURUMIN_NEO_GALAXY_H
#define GURUMIN_NEO_GALAXY_H

#include "vec4.h"
#include "char_slot.h"

// 0x100e7280 (NeoGalaxy_Update; C++ linkage: the original has an EH frame for its matrices and effect options). One
// frame of the body in slot `index`; slots below 0x3a take the default enemy step. Returns 0.
int NeoGalaxy_Update(int index);

// 0x100e21a0 (Enemy_DefaultStep, enemy_react.cpp): the step of the slots below 0x3a.
extern "C" int FUN_100e21a0(int index);

// Engine calls with no header of their own yet.
extern "C" int FUN_1001dc20(int index);                                       // Char_InitSlot (char_init.cpp)
#include "char_update.h"                                                       // Char_UpdateByKind (0x100f9670)
extern "C" int FUN_100246b0(const Vec4 *from, const Vec4 *to, Vec4 *hit, int testOnly, const char *name);  // Collision_RayCast

extern "C" char DAT_10255ac2;                             // cleared every frame the main body runs
extern "C" char DAT_1021f9fa;                             // cleared every frame the main body runs (and during the finish)
extern "C" float DAT_10e3c78c;                            // the finish: brightness of the fade (1 -> 0)
extern "C" int DAT_10e3ddcc;                              // non-zero = the player model is restricted
extern "C" int DAT_10e3ded8;                              // set to 1 by the finish
extern "C" int DAT_1020c2cc;                              // set to 1 when the fight asks for the next round
extern "C" int DAT_1020c2d4;                              // attacks counted towards the round change (x3 = percent chance)
extern "C" int DAT_1020c3d4;                              // cleared with DAT_1020c2d4
extern "C" int DAT_10254478;                              // 0x4b frames after the angry line

#endif
