// The BS_005 boss: the stone golem of the dodge game on map 0x132 (bs005_update.cpp), a per-kind behaviour (kind 0x3f)
// of 0x100f9670. Its fists and rocks are the character slots 0x3d..0x4c (dodge_golem.h).
#ifndef GURUMIN_BS005_H
#define GURUMIN_BS005_H

#include "vec4.h"
#include "actor_scan.h"
#include "enemy_voice.h"
#include "volume_fade.h"

// CharSlot::kind of the golem (also of 0x43, 0x2d, 0x46, 0x69 and 100: the boss kinds that share its voice cooldowns).
#define BS005_KIND 0x3f

// 0x100cc2a0 (Bs005_Update; C++ linkage: the original has an EH frame for its matrices and effect options). One
// frame of the golem in slot `index`; before the fight it runs the ending scene. Returns 0 (1 in the ending's first
// frame).
int Bs005_Update(int index);

// One footstep or fist landing: the frame it plays on, the sound and which fist's node it is heard at.
struct Bs005StepSound
{
    int frame;
    int sound;
    int fist;
};


#endif
