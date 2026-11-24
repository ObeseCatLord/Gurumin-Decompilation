// Enemy turning behaviour (EN build; JP: enemy_moves.cpp). This is the one declaration of these globals and functions in
// decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_ENEMY_MOVES_LONG_H
#define GURUMIN_EN_ENEMY_MOVES_LONG_H

#include "char.h"

// Enemy_TurnToPlayer: CharState::values entries (active flag, turn-lock, turn frames left, cooldown).
const int TURN_VALUE_ACTIVE = 0;
const int TURN_VALUE_LOCKED = 1;
const int TURN_VALUE_FRAMES = 2;
const int TURN_VALUE_COOLDOWN = 3;
// Motion layers (walk in place 0, idle 2) and the blend frames.
const int TURN_MOTION_WALK = 0;
const int TURN_MOTION_IDLE = 2;
const int TURN_MOTION_BLEND = 5;
// Node masks and colour values the first call sets on the model.
const unsigned int TURN_NODE_MASK_A = 0x400;
const unsigned int TURN_NODE_MASK_B = 0x200;
const unsigned char TURN_COLOUR_A = 0xfe;
const unsigned char TURN_COLOUR_B = 0x2f;
// Half a turn and a whole turn in radians, the wrap limits of the angle still to turn.
const float TURN_HALF = 3.1415f;
const float TURN_FULL = 6.28319f;
// Facing limits: facing the player within this angle; turn towards the player within this angle; most turn per frame.
const float TURN_FACING_ANGLE = 0.05236f;
const float TURN_AHEAD_ANGLE = 3.14159f;
const float TURN_STEP = 0.349066f;
// Distance within which the enemy keeps its attention on the player.
const float TURN_ATTENTION_RANGE = 1500.0f;
// Frames of cooldown, frames of walking after a turn, and the end chance range.
const int TURN_COOLDOWN_FRAMES = 30;
const int TURN_WALK_FRAMES = 4;
const int TURN_END_RANGE = 400;
// Per-slot flag cleared by the first call of Enemy_TurnToPlayer. EN 0x009e2420 (JP DAT_1021f6c0).
extern "C" char g_enemyTurnFlag[CHAR_SLOT_COUNT];

// Turns the enemy towards the player, at most 20 degrees per frame, playing walk motion 0 while it turns and idle motion
// 2 otherwise; ends with a chance of `endChance` in 400 per frame (returns 1 then, else 0). EN 0x5bf520.
extern "C" int Enemy_TurnToPlayer(int index, int endChance);

#endif
