// The leaping and ledge-climbing enemy (enemy_leap_climb.cpp, a per-kind behaviour run from 0x100f9670).
#ifndef GURUMIN_LEAP_CLIMB_ENEMY_H
#define GURUMIN_LEAP_CLIMB_ENEMY_H

// 0x10068f70 (Enemy_LeapAndClimb; C++ linkage: the original has an EH frame). `variant` 0 is the plain enemy, 1 the
// one that reacts to hits and spits with the stronger effect.
int Enemy_LeapAndClimb(int index, int variant);

#endif
