// Small EN-only functions found in batch 23 (no JP twin) that belong to no larger header. This is the one declaration of
// them in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_SMALL_B23_H
#define GURUMIN_EN_SMALL_B23_H

// Plays the cue sound when `event` is 1 (EVENT_CUE) and does nothing for other events. Returns 0. EN 0x647a40
// (event_cue_sound.cpp). Only reached through a function pointer.
extern "C" int Event_PlayCueSound(int event);

#endif
