// Idle watching of the player (char_proximity.cpp).
#ifndef GURUMIN_EN_CHAR_PROXIMITY_H
#define GURUMIN_EN_CHAR_PROXIMITY_H

// CharState::values index of the distance unit an idle character notices the player at (squared, times
// WATCH_RANGE_SCALE).
const int CHARSTATE_WATCH_RANGE = 1;
// Factor of the squared range unit: the character notices the player within 10 range units (10 * 10 = 100).
const int WATCH_RANGE_SCALE = 100;
// Motion layer and blend values of the idle motion.
const int IDLE_MOTION_LAYER = 2;
const int IDLE_MOTION_BLEND = 5;

// Plays the idle motion of character `index` and resets its state when the player comes close. EN 0x6d59e0 (JP 0x10045b80).
extern "C" int Char_IdleWatchPlayer(int index);

#endif
