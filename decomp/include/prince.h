// The prince (prince_update.cpp), a per-kind behaviour (kind 0x43) of 0x100f9670: a fighter with a swappable weapon
// and a gatling gun, with an ending scene when the game is over.
#ifndef GURUMIN_PRINCE_H
#define GURUMIN_PRINCE_H

#include "vec4.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"

// CharSlot::kind of the prince (it shares its voice cooldowns with the other boss kinds, see bs005.h).
#define PRINCE_KIND 0x43

// 0x100edea0 (Prince_Update; C++ linkage: the original has an EH frame for its matrices and effect options). One
// frame of the prince in slot `index`. Returns 1 while the ending scene runs, else 0.
int Prince_Update(int index);

#endif
