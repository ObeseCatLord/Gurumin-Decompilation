// Steps, motions and tuning of the strafing enemies (character kinds 0x4c-0x4e, enemy_strafe.cpp). Their CharState
// words: `wait` = cooldown before the next attack (or the frames left of a strafe), `attacking` = the StrafeStep,
// `attackTimer` = frames left of an attack, `action` = frames counted without moving.
#ifndef GURUMIN_STRAFE_ENEMY_H
#define GURUMIN_STRAFE_ENEMY_H

// Steps of a strafing enemy (CharState::attacking).
enum StrafeStep
{
    STRAFE_CHASE = 0,        // run (motion 1) or walk (motion 0) at the player, idle when close
    STRAFE_ATTACK = 1,       // attack motion 3 plays; the effect fires every frame while m_238c is set
    STRAFE_LEFT = 2,         // move sideways: the direction to the player turned by -90 degrees
    STRAFE_RIGHT = 3,        // move sideways the other way (+90 degrees)
    STRAFE_BACK = 4,         // walk away from the player
    STRAFE_FLEE = 10         // Enemy_FlyAndStrafe: back away and rise while the player is close
};

// Motions of these enemies.
enum StrafeMotion
{
    STRAFE_MOTION_WALK = 0,
    STRAFE_MOTION_RUN = 1,
    STRAFE_MOTION_IDLE = 2,
    STRAFE_MOTION_ATTACK = 3
};

#define STRAFE_FAR 300.0f              // farther than this: run
#define STRAFE_NEAR 120.0f             // farther than this: walk; closer: idle
#define STRAFE_STUCK_DISTANCE 3.0f     // moving less than this per frame counts as stuck
#define STRAFE_STUCK_FRAMES 15         // stuck for more frames than this: change step
#define STRAFE_ATTACK_COOLDOWN 100     // frames before the next attack
#define STRAFE_ATTACK_FRAMES 30        // frames an attack lasts
#define STRAFE_FRAMES 120              // frames a strafe (left, right or back) lasts
#define ATTACK_MOTION_SPEED 35.0f      // added to the motion speed while attacking (AnimModel::AddMotionSpeed)
#define MOVE_MOTION_SPEED 20.0f        // Enemy_StrafeAndBurst: added to the motion speed while it moves
#define SHOT_SOUND_PENDING 5           // CharState::shotSound: play the shot sound with the next shot
#define DEFEATED_MARK 0x9c             // DAT_10467ba0 of a defeated Enemy_StrafeAndBurst
#define LUNGE_MIN_FRAMES 14            // Enemy_StrafeAndLunge rushes while at least this many attack frames are left
#define LUNGE_EFFECT_FRAME 29          // ... and fires its effect with this many frames left
#define PI_APPROX 3.1415f              // pi as the game writes it
#define QUARTER_TURN (PI_APPROX / 2)   // a quarter turn (1.57075)

// Enemy_HopAndStrafe (kind 0x50): a strafing enemy that keeps the last three positions where it stood on ground
// (LastPosition step / history2 / history3) and, when it leaves the idle motion, hops back to the oldest of them.
enum HopHistory
{
    HOP_HISTORY_EMPTY = 0,         // CharState::hopHistory before the first frame
    HOP_HISTORY_STARTED = 1,       // the three positions were filled with the start position
    HOP_HISTORY_READY = 2          // the positions were sampled on ground: hops are allowed
};

#define HOP_MOTION 0x5c                // motion played when a hop starts
#define HOP_FRAMES 20                  // frames a hop lasts
#define HOP_RISE_FRAMES 10             // the first frames of a hop rise; the rest move back to history3
#define HOP_RISE_BASE 16               // rise per frame = (HOP_RISE_BASE - frame) * 2 units
#define HOP_RETURN_DIVISOR 9.0f        // the way back to history3 is covered in steps of a ninth
#define HOP_REST_FRAMES 2              // standing still longer than this samples the ground
#define HOP_GROUND_PROBE (-5.0f)       // how far below the feet the ground is looked for
#define HOP_STRAFE_FRAMES 30           // frames of the step a hop starts
#define HOP_ATTACK_EFFECT 0x57         // effect fired by the attack
#define HOP_ATTACK_SOUND 0x153         // sound played with it
#define HOP_ATTACK_EFFECT_FRAME 14     // the attack fires with this many frames left

// Enemy_FlyAndStrafe (kind 0x51): a flying strafing enemy.
#define FLY_LOW_HEIGHT 40.0f           // walking, it flies this high above the player
#define FLY_HIGH_HEIGHT 200.0f         // running, it flies this high above the player
#define FLY_CLIMB_LIMIT 5.0f           // at most this many units up or down a frame
#define FLY_GROUND_PERIOD 30           // the shadow is put on the ground once in this many frames
#define FLY_ATTACK_EFFECT 7            // effect fired by the attack
#define FLY_ATTACK_SOUND 0x158         // sound played with it
#define FLY_ATTACK_EFFECT_FRAME 14     // the attack fires with this many frames left
#define FLEE_DISTANCE 150.0f           // a player closer than this makes it back away (while greetings are on)
#define FLEE_FRAMES 15                 // frames it backs away
#define FLEE_STEP (-20.0f)             // units a frame it backs away (along the direction to the player)
#define FLEE_RISE 8.0f                 // units a frame it rises while backing away

#endif
