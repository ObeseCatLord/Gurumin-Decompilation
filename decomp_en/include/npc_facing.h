// NPC idle and talk facing, and the enemy watch behaviour (EN build; JP: npc_facing.cpp, enemy_moves.cpp). This is the
// one declaration of these globals and functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_NPC_FACING_H
#define GURUMIN_EN_NPC_FACING_H

#include "char.h"

// Motion layers an NPC plays: walk-in-place is 0, idle 2, talk 3.
const int NPC_MOTION_WALK = 0;
const int NPC_MOTION_IDLE = 2;
const int NPC_MOTION_TALK = 3;
// Blend frames of an NPC motion change.
const int NPC_MOTION_BLEND = 5;
// CharState::values entries of an NPC: facing (value 100 and above = do not turn to the player; the rest is the
// direction in units of 1/100 turn) and the "idle motion started" flag.
const int NPC_VALUE_FACING = 1;
const int NPC_VALUE_IDLE_STARTED = 2;
// Facing arithmetic: values wrap at 100; each unit is 30 degrees; * 3.1415 / 180 converts to radians.
const int NPC_FACING_RANGE = 100;
const float NPC_FACING_DEGREES_PER_UNIT = 30.0f;
const float NPC_FACING_DEGREES = 180.0f;
// Half a turn added to the angle towards the player.
const float NPC_TURN_HALF = 3.1415f;   // also the value of pi used to convert facing degrees
// Enemy_WatchPlayer: watch motion, frames at start, and the end chance (15 in 3000 per frame).
const int WATCH_MOTION = 7;
const int WATCH_START_FRAMES = 150;
const int WATCH_END_RANGE = 3000;
const int WATCH_END_BELOW = 15;
// CharState::values entries of Enemy_WatchPlayer: active flag, phase and frames.
const int WATCH_VALUE_ACTIVE = 0;
const int WATCH_VALUE_PHASE = 1;
const int WATCH_VALUE_FRAMES = 2;

// Slot of the NPC the player is talking to. EN 0x013332c4 (JP DAT_10c6351c).
extern "C" int g_talkingNpc;

// Idle in motion 2 facing the configured direction, talk in motion 3. Returns 0. EN 0x6a3b60.
extern "C" int Npc_TalkFacingSimple(int index);
// Idle in motion 2, talk keeping motion 2 going; turns to the player. Returns 0. EN 0x5bf880.
extern "C" int Npc_TalkFacing(int index);
// Starts motion 7 and keeps the enemy turned towards the player until a random chance ends it. EN 0x5be510.
extern "C" int Enemy_WatchPlayer(int index);

#endif
