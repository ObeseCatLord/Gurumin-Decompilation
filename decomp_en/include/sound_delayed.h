// Delayed sound effects of the EN build (JP: decomp/src/sound_queue.cpp). This is the one declaration of the queue and
// its functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_SOUND_DELAYED_H
#define GURUMIN_EN_SOUND_DELAYED_H

#include "vec4.h"

// Sounds that can wait at once.
const int DELAYED_SOUND_COUNT = 10;

extern "C" int g_delayedSoundFrames[DELAYED_SOUND_COUNT];          // EN 0x0091c5b0 (JP DAT_10447310): frames each queued sound still waits (0 = free slot)
extern "C" int g_delayedSoundId[DELAYED_SOUND_COUNT];              // EN 0x0091c588 (JP DAT_1021a398): effect number of each queued sound
extern "C" int g_delayedSoundPositional[DELAYED_SOUND_COUNT];      // EN 0x0091c9d8 (JP DAT_1042d870): 1 = the sound plays at g_delayedSoundPosition
extern "C" Vec4 g_delayedSoundPosition[DELAYED_SOUND_COUNT];       // EN 0x015e9cc8 (JP DAT_10f08a00): where each positional sound plays

// Queues effect `sound` to play after `delay` frames at `position` (0 = not positional). With every slot in use the sound
// that would play soonest is played now and its slot is reused. Returns 0. EN 0x5ca1d0 (JP 0x10054550).
extern "C" int Sound_PlayDelayed(int sound, int delay, const Vec4 *position);

#endif
