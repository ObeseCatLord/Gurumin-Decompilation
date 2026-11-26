// Steps, motions and tuning of the strafing enemies (character kinds 0x4c-0x4e; EN build, JP: decomp/include/strafe_enemy.h).
// Declared once here; only what the ported kinds use. Their CharState::values words: [0] = cooldown before the next
// attack (or the frames left of a strafe), [1] = the StrafeStep, [2] = frames left of an attack, [3] = frames counted
// without moving.
#ifndef GURUMIN_EN_STRAFE_ENEMY_H
#define GURUMIN_EN_STRAFE_ENEMY_H

#include "char.h"

// CharState::values indices of the strafing enemies.
const int STRAFE_VALUE_WAIT = 0;           // cooldown before the next attack, or the frames left of a strafe
const int STRAFE_VALUE_STEP = 1;           // the StrafeStep the enemy is in
const int STRAFE_VALUE_ATTACK_TIMER = 2;   // frames left of the attack
const int STRAFE_VALUE_STUCK = 3;          // frames counted without moving

// Steps of a strafing enemy (STRAFE_VALUE_STEP).
enum StrafeStep
{
    STRAFE_CHASE = 0,        // run (motion 1) or walk (motion 0) at the player, idle when close
    STRAFE_ATTACK = 1,       // attack motion 3 plays
    STRAFE_LEFT = 2,         // move sideways: the direction to the player turned by -90 degrees
    STRAFE_RIGHT = 3,        // move sideways the other way (+90 degrees)
    STRAFE_BACK = 4          // walk away from the player
};

// Motions of these enemies.
enum StrafeMotion
{
    STRAFE_MOTION_WALK = 0,
    STRAFE_MOTION_RUN = 1,
    STRAFE_MOTION_IDLE = 2,
    STRAFE_MOTION_ATTACK = 3
};

const float STRAFE_FAR = 300.0f;             // farther than this: run
const float STRAFE_NEAR = 120.0f;            // farther than this: walk; closer: idle
const float STRAFE_STUCK_DISTANCE = 3.0f;    // moving less than this per frame counts as stuck
const int STRAFE_STUCK_FRAMES = 15;          // stuck for more frames than this: change step
const int STRAFE_ATTACK_COOLDOWN = 100;      // frames before the next attack
const int STRAFE_ATTACK_FRAMES = 30;         // frames an attack lasts
const int STRAFE_FRAMES = 120;               // frames a strafe (left, right or back) lasts
const float ATTACK_MOTION_SPEED = 35.0f;     // added to the motion speed while attacking (AnimModel::AddMotionSpeed)
const int LUNGE_MIN_FRAMES = 14;             // the lunger rushes while at least this many attack frames are left
const int LUNGE_EFFECT_FRAME = 29;           // ... and fires its effect with this many frames left
const float PI_APPROX = 3.1415f;             // pi as the game writes it
const float QUARTER_TURN = PI_APPROX / 2;    // a quarter turn (1.57075)

// The hopping strafer (kind 0x50, JP Enemy_HopAndStrafe) keeps more variables in CharState::values; it keeps its three last
// standing positions in the ShotSlot (step, history2, history3).
const int HOP_VALUE_REQUESTS = 7;     // hop requests counted (a hop starts at the second)
const int HOP_VALUE_REST = 8;         // frames spent resting in the idle motion
const int HOP_VALUE_BACK = 9;         // 1 = the hop under way is the second one (back to chasing)
const int HOP_VALUE_FRAME = 10;       // frame of the hop under way (0 = none)
const int HOP_VALUE_HISTORY = 12;     // a HopHistory
enum HopHistory
{
    HOP_HISTORY_EMPTY = 0,            // before the first frame
    HOP_HISTORY_STARTED = 1,          // the three positions were filled with the start position
    HOP_HISTORY_READY = 2             // the positions were sampled on ground: hops are allowed
};
const int HOP_MOTION = 0x5c;                  // motion played when a hop starts
const int HOP_FRAMES = 20;                    // frames a hop lasts
const int HOP_RISE_FRAMES = 10;               // the first frames of a hop rise; the rest move back to history3
const int HOP_RISE_BASE = 16;                 // rise per frame = (HOP_RISE_BASE - frame) * 2 units
const int HOP_REST_FRAMES = 2;                // standing still longer than this samples the ground
const float HOP_GROUND_PROBE = -5.0f;         // how far below the feet the ground is looked for
const int HOP_STRAFE_FRAMES = 30;             // frames of the step a hop starts
const int HOP_ATTACK_EFFECT = 0x57;           // effect fired by the attack
const int HOP_ATTACK_SOUND = 0x153;           // sound played with it
const int HOP_ATTACK_EFFECT_FRAME = 14;       // the attack fires with this many frames left
const float HOP_ATTACK_EFFECT_SCALE = 1.5f;   // size of that effect
const char HOP_ATTACK_EFFECT_MODE = 1;        // its mode
// Set to 1 by the hopping strafer each frame it runs (per character slot). EN 0x00b49400 (JP DAT_10254310).
extern "C" unsigned char g_hopperActive[CHAR_SLOT_COUNT];

// One frame of the lunging strafer (kind 0x4e) `index`. Returns 0. EN 0x69c9f0 (JP 0x10099040); declared in
// char_dispatch.h under the name CharKind4e_Update.

#endif
