// The two positional sound loops that follow the player (streams 0x23b/0x23c of the effect table) and their
// globals, declared once. Used by src/doppler_loops.cpp; the scene frame (FUN_100f5a80) calls the update.
#ifndef GURUMIN_DOPPLER_LOOPS_H
#define GURUMIN_DOPPLER_LOOPS_H

#include "vec4.h"

// 0x1004fbe0 (Sound_UpdateDopplerLoops, doppler_loops.cpp): the scene frame's per-frame update of the loops.
extern "C" int FUN_1004fbe0(void);

// 0x1003ad20 (Sound_PlayEffect, sound_effects.cpp): plays effect `sound` at `position`, returns the loop's handle.
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);
// 0x10001890 (app_0002.cpp): moves 3D sound `stream` to (x, y, z) with velocity (dx, dy, dz). The sound process
// receives the raw bits of each float.
extern "C" int FUN_10001890(int stream, const float *x, const float *y, const float *z, const float *dx,
                            const float *dy, const float *dz);

extern "C" int DAT_10c635d4;   // handle of the first loop (0x23c); -1 once the loops are stopped
extern "C" int DAT_10c635d8;   // handle of the second loop (0x23b)
extern "C" int DAT_10c635dc;   // 1 while the loops run

#endif
