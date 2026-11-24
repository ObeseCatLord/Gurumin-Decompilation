// The stalking melee enemy with three dash attacks and a slam (enemy_stalk_strike.cpp, a per-kind behaviour run from
// 0x100bcb30).
#ifndef GURUMIN_STRIKE_ENEMY_H
#define GURUMIN_STRIKE_ENEMY_H

// 0x100a9240 (Enemy_StalkAndStrike; C++ linkage: the original has an EH frame). `variant` 0 picks one of the three
// dash attacks when it reacts, any other value also allows the slam (and the greeting wave).
int Enemy_StalkAndStrike(int index, int variant);

#endif
