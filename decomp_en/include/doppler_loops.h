// The two positional sound loops that follow the player (streams 0x23b/0x23c of the effect table) and their globals, EN
// build (JP: decomp/include/doppler_loops.h). Declared once here. Used by src/sound_doppler.cpp; the scene frame calls the
// update.
#ifndef GURUMIN_EN_DOPPLER_LOOPS_H
#define GURUMIN_EN_DOPPLER_LOOPS_H

#include "vec4.h"

// Positions of the player of the last three frames (newest first); x at or below DOPPLER_UNSET_POSITION = not set yet.
const int DOPPLER_TRAIL_LENGTH = 3;
extern "C" Vec4 g_playerTrail[DOPPLER_TRAIL_LENGTH];   // EN 0x01338538 (JP DAT_10edbd58)
extern "C" int g_dopplerLoopFirst;     // EN 0x0133337c (JP DAT_10c635d4): handle of the first loop (effect 0x23c); -1 once the loops are stopped
extern "C" int g_dopplerLoopSecond;    // EN 0x01333380 (JP DAT_10c635d8): handle of the second loop (effect 0x23b)
extern "C" int g_dopplerLoopsRunning;  // EN 0x01333384 (JP DAT_10c635dc): 1 while the loops run

// Sound engine: moves the 3D sound `stream` to (x, y, z) with velocity (dx, dy, dz); the values travel as raw bits.
// EN 0x009094d4 (JP FUN_10001890).
extern "C" int (*g_soundMove)(int stream, const float *x, const float *y, const float *z, const float *dx, const float *dy,
                              const float *dz);

// Sound_UpdateDopplerLoops: the scene frame's per-frame update of the loops (see sound_doppler.cpp). Returns 0. EN 0x6dedc0
// (JP 0x1004fbe0).
extern "C" int Sound_UpdateDopplerLoops(void);

#endif
