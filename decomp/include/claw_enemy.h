// The stalking enemy that claws at the player (enemy_stalk_claw.cpp, the normal behaviour of the enemies run from
// 0x100bcb30 and 0x100d2110).
#ifndef GURUMIN_CLAW_ENEMY_H
#define GURUMIN_CLAW_ENEMY_H

// 0x100aa820 (Enemy_StalkAndClaw; C++ linkage: the original has an EH frame). `variant` is passed by every caller
// (0, 1 or 2) but never read.
int Enemy_StalkAndClaw(int index, int variant);

#endif
