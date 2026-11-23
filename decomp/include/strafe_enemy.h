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

#endif
