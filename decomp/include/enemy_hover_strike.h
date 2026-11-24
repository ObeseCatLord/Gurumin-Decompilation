// The floating enemy that keeps its distance and calls effects down on the player (enemy_hover_strike.cpp, behaviours
// 8 and 9 of Enemy_UpdateBehaviour 0x100bcb30).
#ifndef GURUMIN_ENEMY_HOVER_STRIKE_H
#define GURUMIN_ENEMY_HOVER_STRIKE_H

// 0x100ade30 (Enemy_HoverAndCallStrike; C++ linkage: the original has an EH frame). `variant` 1 (behaviour 9) adds a
// second burst around the enemy itself.
int Enemy_HoverAndCallStrike(int index, int variant);

#endif
