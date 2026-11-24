// The roaming spitter (enemy_roam_spit.cpp, caller 0x100f9670): a ghost-like enemy that follows a recorded path, steers
// clear of the other roamers and now and then spits an effect at the player.
#ifndef GURUMIN_ENEMY_ROAM_SPIT_H
#define GURUMIN_ENEMY_ROAM_SPIT_H

extern "C" int DAT_10db03bc;   // the roamers only act while this is above 0
extern "C" int DAT_10210e18;   // non-zero = the roamers stand still
extern "C" int DAT_1020c2c4;   // chapter of the recorded path (100 frames each; the roamer waits while it is under 50)

int Enemy_RoamAndSpit(int index);   // 0x10084e80 (C++ linkage: the original has an EH frame)

#endif
