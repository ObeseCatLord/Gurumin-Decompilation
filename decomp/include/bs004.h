// The BS_004 boss: the moss root creature (bs004_update.cpp), a per-kind behaviour (kind 0x46) of 0x100f9670. It hovers
// over a path of nodes, dives at the player, sweeps its tail and breathes fire; three minion slots follow it.
#ifndef GURUMIN_BS004_H
#define GURUMIN_BS004_H

#include "vec4.h"
#include "actor_scan.h"
#include "enemy_voice.h"
#include "volume_fade.h"

// CharSlot::kind of the creature (it shares its voice cooldowns with the other boss kinds, see bs005.h).
#define BS004_KIND 0x46

// 0x100c39c0 (Bs004_Update; C++ linkage: the original has an EH frame for its matrices and effect options). One frame
// of the creature in slot `index`; once the game is over (DAT_10210d30) it runs the ending scene instead. Returns 0 (1
// while the ending scene runs).
int Bs004_Update(int index);

extern "C" int DAT_10c646f4;   // non-zero = the camera trigger of the defeat scene is armed

#endif
