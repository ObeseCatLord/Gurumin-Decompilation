// Character movement helpers of the EN build (JP: decomp/include/char_move.h).
#ifndef GURUMIN_EN_CHAR_MOVE_H
#define GURUMIN_EN_CHAR_MOVE_H

struct Playback;

// AnimModel::mode value of the walking mode.
const int MODEL_MODE_WALKING = 1;
// AnimModel::idleMotionEnabled value that excludes the walking-mode slowdown.
const int IDLE_MOTION_ACTIVE = 1;
// Share of a movement impulse applied in the walking mode, in the other modes, and in the other modes while the
// character is nearly standing still.
const float MOVE_SCALE_WALKING = 0.5f;
const float MOVE_SCALE_NORMAL = 0.7f;
const float MOVE_SCALE_STANDING = 1.4f;
// Squared speed below which a character counts as standing still.
const float STANDING_SPEED_SQUARED = 1.0f;

// Idle motion of an NPC: motion layer, blend frames, and the time between two talk prompts.
const int NPC_IDLE_MOTION = 2;
const int NPC_IDLE_BLEND_FRAMES = 5;
const int NPC_TALK_GAP_FRAMES = 75;
// Chance per frame (out of NPC_TALK_CHANCE_RANGE) that an idle NPC with a talk line speaks.
const int NPC_TALK_CHANCE = 300;
const int NPC_TALK_CHANCE_RANGE = 10000;
// Voice sounds an idle NPC picks from (first id and count) and how long its prompt stays up.
const int NPC_TALK_SOUND_FIRST = 0x209;
const int NPC_TALK_SOUND_COUNT = 3;
const int NPC_PROMPT_FRAMES = 60;
// A talk value is key script * NPC_TALK_SCRIPT_SCALE + line; a prompt key is script * NPC_PROMPT_KEY_SCALE + line - 1.
const int NPC_TALK_SCRIPT_SCALE = 100;
const int NPC_PROMPT_KEY_SCALE = 1000;
// CharState::state values of an idle NPC.
const int NPC_STATE_START = 0;
const int NPC_STATE_IDLE = 1;

// Plays voice `sound` for character `index`. EN 0x649cd0 (JP 0x10065b10).
extern "C" int Character_PlaySound(int index, int sound, int always, int optional);
// Keeps NPC `index` standing in idle motion and lets it speak now and then. EN 0x6d68b0 (JP 0x100b5ae0).
extern "C" int Npc_Idle(int index);
// Waiting NPC that keeps its back to the player: the reaction range is the talk line times this; it turns until within
// TURN_SQUARED_LIMIT (squared radians, about one degree) of facing away; once the player is nearer than its range it
// starts walking away (state 2, behaviour 0xf) 16 units per frame along its facing.
const int NPC_RANGE_PER_LINE = 10;
const float TURN_SQUARED_LIMIT = 0.000304503f;
const int NPC_WALK_AWAY_WAIT = 0x1d;
const int NPC_WALK_AWAY_TALK = 999;
const int NPC_WALK_AWAY_BEHAVIOUR = 0xf;
const int NPC_STATE_WALK_AWAY = 2;
const int NPC_WALK_IDLE_MOTION = 0;
const float NPC_WALK_AWAY_STEP = -16.0f;
const float NPC_WALK_AWAY_SPEED = 20.0f;
// Keeps NPC `index` with its back to the player until the player comes near. EN 0x6d5dd0 (JP 0x10045c30).
extern "C" int Npc_WaitFacingAway(int index);
// Sets the movement impulse of character `index` from (x, y, z). EN 0x5b98a0 (JP 0x100241e0).
extern "C" int Character_SetMove(int index, float x, float y, float z);

// Wakes the enemies within `radius` of character `index`. `count` behaviour numbers follow (0 = wake every enemy; callers pass 1 and
// 15 to wake only enemies with behaviour 15). EN 0x6ac0e0 (JP 0x1001b670).
extern "C" int Enemies_WakeNearby(int index, float radius, int count, ...);
// Keeps NPC `index` idling until the player comes near, then makes it face the player and talk. EN 0x6d6220 (JP 0x100b58a0).
extern "C" int Npc_TalkWhenNear(int index, int wakeEnemies);

// Starts the alert reaction of NPC `index` once it sees the player. EN 0x6c3210 (JP 0x100b5f80).
int Npc_AlertWhenSeen(int index);

// Like Npc_AlertWhenSeen, and turns to face the player at once. EN 0x6c34f0 (JP 0x100b5c30).
int Npc_TurnWhenSeen(int index);

// An NPC that waits underground and rises when the player comes near. EN 0x6b0d60 (JP 0x100b3430).
int Npc_RiseWhenSeen(int index);

// A guard that belongs to a group: counts its group mates, now and then turns to look at one, and alarms on seeing the
// player. EN 0x6b3b20.
int Npc_GroupWatch(int index);

// A character held in place by a spell: sparkles until the scripts release it, then it is freed with a flash. EN 0x6af410 (JP 0x100469d0).
int Npc_Held(int index);

// A door that swings about its hinge. EN 0x6ae950 (JP 0x10048b70).
int Char_SwingDoor(int index);

// The motion playback of the enemy that moved last (left there for the scripts; cleared at the start of each move). EN 0x0162ef64 (JP DAT_1101d880).
extern "C" Playback *g_lastEnemyPlayback;
// An enemy that follows the player and blasts. EN 0x699f20 (JP 0x1009dfd0).
extern "C" int Enemy_FollowAndBlast(int index);

// An NPC that keeps facing the player and talks. EN 0x6b4220 (JP 0x100b1ff0).
int Npc_FaceAndTalk(int index);

// An enemy that drops in from above. EN 0x6de520 (JP 0x10048e90).
int Enemy_DropIn(int index);

// An enemy that runs and walks to the player and blasts. EN 0x699990 (JP 0x1009d100).
extern "C" int Enemy_ChaseAndBlast(int index);

// Enemy_DropIn that first waits hidden until the player comes near. EN 0x6de0c0 (JP 0x10049240).
int Enemy_DropInWhenNear(int index);

// An enemy that idles, shoves (attacks) and blasts. EN 0x69a550 (JP 0x10098be0).
extern "C" int Enemy_ShoveAndBlast(int index);

#endif

// Messenger that claims a marker and runs to it once it has seen the player. EN 0x6d0f20 (JP 0x100b6290).
extern "C" int Npc_RunToMarker(int index);

struct Vec4;
// 1 when `eye` (looking along `heading`) sees `target` within `range` and `angleRange` (or `nearRange`), optionally with a wall test. EN 0x5b9500 (JP 0x10043e20).
extern "C" int Char_CanSee(const Vec4 *target, const Vec4 *eye, float heading, float range, float angleRange, float nearRange, int checkWalls);
// Bits of the axis mask of Char_ScaleMoveAxes.
const unsigned int MOVE_AXIS_X = 1;
const unsigned int MOVE_AXIS_Y = 2;
const unsigned int MOVE_AXIS_Z = 4;
// Multiplies the pending movement impulse of character `index` by the model's speed scale on the axes in `axes`
// (MOVE_AXIS_* bits). Returns 0. EN 0x6458e0 (EN-only, see char_move.cpp; no direct caller).
extern "C" int Char_ScaleMoveAxes(int index, unsigned int axes);
