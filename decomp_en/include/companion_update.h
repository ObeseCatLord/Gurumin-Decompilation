// The player's companions (EN build; JP: decomp/src/companion_update.cpp). Companion k (0 to 2) of character `index`
// lives in character slot index + 2 + k and keeps its health and its down / revive cycle in its own CharState; the
// owner's CharState only holds the health cap a revived companion gets. This is the one declaration of the constants
// and updates in decomp_en.
#ifndef GURUMIN_EN_COMPANION_UPDATE_H
#define GURUMIN_EN_COMPANION_UPDATE_H

#include "char.h"

// Indices into CharState::values of a companion slot.
const int COMPANION_HIDE_COUNT = 1;      // frames the companion stays hidden (2 in the areas 0x13 and 0x30), counted down
const int COMPANION_DOWN_TIMER = 2;      // frames counted in the down / revive cycle
const int COMPANION_DOWN_MODE = 3;       // 0 = active, 1 = reviving, 2 = recovering, 10 and above = going down
const int COMPANION_REVIVE_HEALTH = 4;   // health it revives with (0 = its maximum + 2000)
// Index into CharState::values of the owner slot: a revived companion gets at most 2000 + this health.
const int COMPANION_HEALTH_CAP = 2;
// Index into CharState::values of the owner slot: non-zero = a hidden companion 1 stays where it is (else it sinks).
const int COMPANION_KEEP_UP = 1;

// Values of COMPANION_DOWN_MODE.
const int COMPANION_ACTIVE = 0;
const int COMPANION_REVIVING = 1;
const int COMPANION_RECOVERING = 2;
const int COMPANION_GOING_DOWN = 10;     // and above: counts down to COMPANION_COMES_BACK
const int COMPANION_COMES_BACK = 11;
// Frames the revive takes (mode 1), and the going-down count at which the down effect starts.
const int COMPANION_REVIVE_FRAMES = 60;
const int COMPANION_DOWN_EFFECT_AT = 70;
// Revived companions add this to the score (guarded value 7), start this effect and play this sound.
const int COMPANION_REVIVE_BONUS = 30;
const int COMPANION_REVIVE_SOUND = 0x194;
const int COMPANION_DOWN_SOUND = 0x192;
// Pose state (g_charPoseState) of a companion that is down or recovering, and of one that was hit this frame.
const int COMPANION_POSE_DOWN = 2;
const int COMPANION_POSE_HIT = 6;
const int COMPANION_POSE_REVIVED = 0x3c;
// Health above which a companion is fine, above which it only hovers near death, and the value a dying one sits at.
const int COMPANION_HEALTH_FULL = 70000;
const int COMPANION_HEALTH_LOW = 2000;
// A companion goes down when its health falls this far below its maximum (any loss while guarded value 8 is 1).
const int COMPANION_DAMAGE_LIMIT = 10;
// The same for companion 1.
const int COMPANION1_DAMAGE_LIMIT = 5;
// Companion 1 sinks to this height when hidden and rises back to ground level (0) when shown.
const float COMPANION1_HIDDEN_Z = -50.0f;
// Pose state companion 1 gets when it revives, and the sound it plays when it goes down.
const int COMPANION1_POSE_REVIVED = 0x1e;
const int COMPANION1_DOWN_EFFECT = 0x36d4;
const int COMPANION1_REVIVE_EFFECT = 0x36d1;
const int COMPANION1_GOING_DOWN_SOUND = 0x193;
// Health a recovered companion gets on top of its current value, capped by the owner's cap + COMPANION_HEALTH_LOW.
const int COMPANION_RECOVER_HEALTH = 200;
// Map areas in which greetings hide the companion, and the hide count they set.
const int COMPANION_HIDE_AREA_A = 0x13;
const int COMPANION_HIDE_AREA_B = 0x30;
const int COMPANION_HIDE_FRAMES = 2;
// The value stats[STAT_VALUE_4] of a hidden companion.
const int COMPANION_HIDDEN_VALUE = 999;
// Game mode (guarded value 9) in which companions never hide.
const int COMPANION_NO_HIDE_MODE = 6;
// Node flag set while a companion is down, and the node colour it comes back with.
const unsigned int COMPANION_NODE_DOWN_FLAG = 0x20;
const unsigned int COMPANION_COLOUR_MASK = 0x400;
const int COMPANION_COLOUR_VALUE = 0x2f;
// g_charKind of a recovered companion (an enemy).
const char COMPANION_KIND_ENEMY = 3;
// Pose state of an owner that follows its player, the display state above which it is "far", and the display states
// it gets.
const int COMPANION_FOLLOW_POSE = 7;
const int COMPANION_FAR_STATE_LIMIT = 4;
const int COMPANION_FAR_STATE = 6;
const int COMPANION_NEAR_STATE = 4;

// Hit flag of each slot (set when the character was hurt this frame, cleared by the companion update). EN 0x00a363e8 (JP DAT_1042d9e8).
extern "C" unsigned char g_charHitFlag[CHAR_SLOT_COUNT];

// Companion 2 of character `index` (slot index + 4): keeps its counters in range and runs its down / revive cycle.
// Returns 0. Called by the character updater. EN 0x5a6940 (JP 0x100b0f00).
int Companion2_Update(int index);
// Companion 1 of character `index` (slot index + 3). Returns 0. EN 0x5a71e0 (JP 0x100b0880).
int Companion1_Update(int index);

#endif
