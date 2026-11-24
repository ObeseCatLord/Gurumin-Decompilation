// Per-frame enemy AI updaters and the behaviour functions they run (EN build).
#ifndef GURUMIN_EN_ENEMY_AI_H
#define GURUMIN_EN_ENEMY_AI_H

// CharState::state value of an enemy whose behaviour finished: the picker chooses the next one.
const int ENEMY_STATE_FINISHED = -1;
// Behaviour numbers of the updaters (CharState::state values 0, 1 and 2).
const int ENEMY_BEHAVIOUR_FIRST = 0;
const int ENEMY_BEHAVIOUR_SECOND = 1;
const int ENEMY_BEHAVIOUR_THIRD = 2;
// CharState::values entry that counts frames between two behaviours.
const int ENEMY_VALUE_PICK_DELAY = 2;
// Frames an idle enemy waits before its picker chooses the next behaviour.
const int ENEMY_PICK_DELAY_FRAMES = 0;
// Distance within which kind A wanders around its home.
const float ENEMY_WANDER_RANGE = 300.0f;
// Argument of Enemy_TurnToPlayer used by the updaters: the chance per frame that the turn ends.
const int ENEMY_TURN_END_CHANCE = 2;

// Returns non-zero while the enemy is busy (stunned, hit, ...) and must not move. EN 0x5bac00 (JP 0x100e1c40).
extern "C" int Enemy_IsBusy(int index);
// Walks to random places within `range` of the enemy's home; non-zero when done. EN 0x5bddd0 (JP 0x10026d80).
extern "C" int Enemy_WanderNearHome(int index, float range);
// Keeps the enemy at a distance from the player; non-zero when done. EN 0x5bef20 (JP 0x10027da0).
extern "C" int Enemy_KeepDistanceFromPlayer(int index, int arg);
// Plays one of the idle motions; non-zero when done. EN 0x5be660 (JP 0x10027560).
extern "C" int Enemy_IdleMotion(int index);
// Behaviour pickers (ai_pick.cpp).
extern "C" int Enemy_PickAction_40_25_35(int index);   // EN 0x5b99a0
extern "C" int Enemy_PickAction_60_10_30(int index);   // EN 0x5b9b10

// Updaters of the enemy kinds A, C and D; each returns 0. EN 0x5ba630, 0x5baa30, 0x5baac0.
extern "C" int Enemy_UpdateAI_A(int index);   // EN 0x5ba630 (JP 0x100e2080)
extern "C" int Enemy_UpdateAI_C(int index);   // EN 0x5baa30 (JP 0x100e2440)
extern "C" int Enemy_UpdateAI_D(int index);   // EN 0x5baac0 (JP 0x100e24d0)

#endif
