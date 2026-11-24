// The map 0x15 mini game (target_race.cpp): targets drop from the sky into the arena and the player races two rival
// characters (slots 0x3a and 0x3b) to destroy 50 of them. Each target has a marker entry (CharState slots 0x3c..0x43,
// see CharState::targetSlot); each rival runs at the nearest target, pounces on it and is sometimes stunned by a hit.
// The arena (DAT_10803a78, dodge_game.h) is shared with the map 0x16 game: here spawnY is the smallest and endY the
// largest y.
#ifndef GURUMIN_TARGET_RACE_H
#define GURUMIN_TARGET_RACE_H

extern "C" int FUN_100541f0(int index);          // 0x100541f0 (enemy_destroy.cpp): destroys character `index`
extern "C" int FUN_1016b230(const char *text);   // 0x1016b230 (camera.cpp): sets the title caption

int Map15_UpdateTargetRace();    // 0x1007dab0

#endif
