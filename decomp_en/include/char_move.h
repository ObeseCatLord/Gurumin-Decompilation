// Character movement helpers of the EN build (JP: decomp/include/char_move.h).
#ifndef GURUMIN_EN_CHAR_MOVE_H
#define GURUMIN_EN_CHAR_MOVE_H

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
// Sets the movement impulse of character `index` from (x, y, z). EN 0x5b98a0 (JP 0x100241e0).
extern "C" int Character_SetMove(int index, float x, float y, float z);

#endif
