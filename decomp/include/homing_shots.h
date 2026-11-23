// The homing shots of the kind 0x43 enemies (homing_shots.cpp). The enemy's slot `owner` has six shot slots owner+2 ..
// owner+7 (see char_shots.cpp: shot angle, heading and speed in DAT_10255160[slot][0..2], life frames in
// CharState::values[0]); this update flies them at the player.
#ifndef GURUMIN_HOMING_SHOTS_H
#define GURUMIN_HOMING_SHOTS_H

#define KIND_HOMING_SHOOTER 0x43        // CharSlot::kind of the enemy that fires the shots
#define SHOOTER_SLOT_FIRST 0x3a         // enemy slots searched for the shooter
#define SHOOTER_SLOT_END 0x5c           // also the "no shooter" result
#define SHOT_SLOT_FIRST 2               // the shot slots are owner + 2 ...
#define SHOT_SLOT_END 8                 // ... to owner + 7
#define SHOT_AIM_RAISE 70.0f            // the shots aim this far above the player's feet
#define SHOT_MAX_SPEED 30.0f            // the speed grows by SHOT_ACCELERATION until it reaches this
#define SHOT_ACCELERATION 0.4f
#define SHOT_STRAIGHT_LIFE 285          // the shot steers only once its life counter is below this
#define SHOT_PITCH_RISE 0.06f           // pitch step per frame upwards ...
#define SHOT_PITCH_FALL 0.0125f         // ... and downwards
#define SHOT_YAW_STEP 0.025f            // heading step per frame
#define SHOT_ANGLE_HALF_TURN 3.1415f    // pi as the game writes it
#define SHOT_ANGLE_FULL_TURN 6.283f
#define SHOT_MIN_AIM_DISTANCE 1.0f      // nearer than this the shot's pitch goal is level
#define SHOT_HIT_RADIUS 60.0f           // within this horizontal distance (and 120 vertically) the shot hits the player
#define SHOT_HIT_HEIGHT 120.0f
#define SHOT_WALL_RADIUS 30.0f          // radius of the sphere tested against the map
#define SHOT_TRAIL_PERIOD 3             // the trail effect starts every this many frames
#define SHOT_TRAIL_OFFSET 80.0f         // ... this far behind the shot
#define SHOT_EFFECT_MAIN 0x7c9          // flies with the shot
#define SHOT_EFFECT_SPARK 0x26          // added on three of every four frames
#define SHOT_EFFECT_TRAIL 0x7cb
#define SHOT_EFFECT_BURST 0x36cf        // where the shot ends
#define SHOT_EFFECT_SCALE 1.15f          // EffectOptions::m_0008 of the burst
#define SHOT_EFFECT_SETTING 5           // EffectOptions::m_000e of the burst

// 0x1003fc40 (Kind43_UpdateShots, homing_shots.cpp): the scene frame's per-frame update of the shots. A C++ function
// (its temporaries are unwound on exceptions). Returns 0.
int Kind43_UpdateShots(void);

#endif
