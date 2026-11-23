// Steps, motions and tuning of the slashing enemy (character kind 0x4f, enemy_slash.cpp). Its CharState words:
// `state` = the SlashStep, `wait` = frames spent in the current step, `stuckFrames` = frames counted without moving.
#ifndef GURUMIN_SLASH_ENEMY_H
#define GURUMIN_SLASH_ENEMY_H

// Steps of the slashing enemy (CharState::state).
enum SlashStep
{
    SLASH_CHASE = 0,        // run at the player (motion 2) until close enough to get ready
    SLASH_GUARD = 1,        // stand guard facing the player (motion 4); from here it slashes or taunts
    SLASH_READY = 2,        // got close: face the player for a moment (motion 3), then guard
    SLASH_TAUNT = 3,        // far from the player: taunt (motion 6) for a moment, then chase again
    SLASH_ATTACK = 4,       // the slash (motion 5): the trail effect fires once, part-way through
    SLASH_SIDESTEP_LEFT = 6,  // stuck while chasing: run with the direction to the player turned by +90 degrees
    SLASH_SIDESTEP_RIGHT = 7, // ... turned by -90 degrees
    SLASH_BACK_OFF = 8      // ... away from the player
};

// Motions of this enemy.
enum SlashMotion
{
    SLASH_MOTION_RUN = 2,
    SLASH_MOTION_READY = 3,
    SLASH_MOTION_GUARD = 4,
    SLASH_MOTION_SLASH = 5,
    SLASH_MOTION_TAUNT = 6
};

#define SLASH_RUN_SPEED 22.0f          // units a frame while chasing or side-stepping
#define SLASH_STOP_DISTANCE 90.0f      // closer than this it stops running
#define SLASH_READY_DISTANCE 200.0f    // closer than this it gets ready at once
#define SLASH_LUNGE_DISTANCE 300.0f    // closer than this it may get ready (6 % a frame) after SLASH_CHASE_MIN_FRAMES
#define SLASH_ATTACK_DISTANCE 320.0f   // a guarding one closer than this may slash (51 % a frame)
#define SLASH_TAUNT_DISTANCE 350.0f    // a guarding one farther than this may taunt (4 % a frame)
#define SLASH_STUCK_DISTANCE 5.0f      // moving less than this per frame counts as stuck
#define SLASH_STUCK_FRAMES 15          // stuck for more frames than this: side-step (or stop side-stepping)
#define SLASH_CHASE_MIN_FRAMES 30      // frames of chasing before it may get ready at random
#define SLASH_SIDESTEP_FRAMES 120      // frames a side-step lasts at most
#define SLASH_GUARD_MIN_FRAMES 5       // frames of guarding before it may slash
#define SLASH_TAUNT_MIN_FRAMES 20      // frames of guarding before it may taunt
#define SLASH_READY_FRAMES 30          // frames the ready step lasts
#define SLASH_TAUNT_FRAMES 30          // frames the taunt lasts
#define SLASH_ATTACK_FRAMES 50         // frames the slash lasts
#define SLASH_EFFECT_FRAME 20          // frame of the slash on which the trail effect fires
#define SLASH_EFFECT 0x4c              // the trail effect (Char_StartEffect), drawn between hand and forearm
#define SLASH_SOUND 0x152              // sound of the slash
#define SLASH_EFFECT_SCALE 1.5f        // EffectOptions::m_0008 of the trail
#define SLASH_EFFECT_SETTING 5         // EffectOptions::m_000e of the trail

#endif
