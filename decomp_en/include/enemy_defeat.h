// Defeated enemies (EN build; JP: decomp/src/enemy_defeat.cpp): the character is taken out of the world and leaves its
// loot behind. This is the one declaration of the constants and the function in decomp_en.
#ifndef GURUMIN_EN_ENEMY_DEFEAT_H
#define GURUMIN_EN_ENEMY_DEFEAT_H

#include "char.h"
#include "vec4.h"

// g_charPhase value of a character that has not been handled yet, the same after a wait, and once its loot is out.
const int DEFEAT_PHASE_FRESH = 0;
const int DEFEAT_PHASE_FRESH_DELAYED = 1000;
const int DEFEAT_PHASE_DONE = 1;
// CharRecord::stats entry that, when set, raises the coins by half.
const int LOOT_STAT_BONUS = 11;
// Height (matrix z) of a character that was taken out of the world.
const float OUT_OF_WORLD_HEIGHT = -3390.0f;
// Coins and items appear this far above the character.
const float LOOT_HEIGHT = 50.0f;
// Piece type of the burst of debris (the thrown spawner uses this type or the next).
const int PUFF_PIECE = 100;
// Factor of the coin bonuses.
const float BONUS_COIN_FACTOR = 1.5f;
// Gold-bonus bits of the worn helmet's face settings (g_helmetFaceSettings): coins x3, x2 and x1.5.
const int HELMET_GOLD_X3 = 0x30000000;
const int HELMET_GOLD_X2 = 0x20000000;
const int HELMET_GOLD_X1_5 = 0x10000000;
const float HELMET_GOLD_X3_FACTOR = 3.0f;
const float HELMET_GOLD_X2_FACTOR = 2.0f;
// Debris piece types of the coin denominations, and their values.
const int COIN_1 = 0x51;
const int COIN_5 = 0x52;
const int COIN_10 = 0x53;
const int COIN_50 = 0x54;
const int COIN_100 = 0x55;
const int COIN_500 = 0x56;
const int COIN_VALUE_1 = 1;
const int COIN_VALUE_5 = 5;
const int COIN_VALUE_10 = 10;
const int COIN_VALUE_50 = 50;
const int COIN_VALUE_100 = 100;
const int COIN_VALUE_500 = 500;
// The item roll is a random number below DROP_ROLL_RANGE; the three item drops are checked in turn against these limits.
const int DROP_ROLL_RANGE = 100;
const int DROP_CHANCE_ITEM_C4 = 2;
const int DROP_CHANCE_ITEM_C0 = 4;
const int DROP_CHANCE_ITEM_BC = 6;
// Node colour value cleared (mask -1) on the model of a defeated character.
const int DEFEAT_COLOUR_VALUE = 0;
const unsigned int DEFEAT_COLOUR_MASK = 0xffffffff;

// Position the debris of each character slot is thrown away from. EN 0x00dad980 (JP DAT_10836fc8).
extern "C" Vec4 g_charLootOrigin[CHAR_SLOT_COUNT];

// Handles character `index` once it has been defeated (phase 0 or 1000, else returns 0): zeroes its health, moves it
// far below the world, throws a burst of 15 debris pieces around its position, and the first time marks its map id
// killed, tells it to react and clears the colour value of its model. Then drops its coins as debris pieces by
// denomination (500, 100, 50, 10, 5, 1), the amount raised by half when the slot's bonus stat is set and again by the
// helmet's gold bonus, and rolls once for one of its three items. Sets the phase to 1. Returns 0. EN 0x6cce80 (JP 0x100627d0).
extern "C" int Enemy_Defeat(int index);

#endif
