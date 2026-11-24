// Steps and tuning of the barrage enemies (behaviour numbers of Enemy_UpdateBehaviour 0x100bcb30, enemy_barrage.cpp):
// enemies that walk with the common enemy behaviour (Enemy_ChaseLungeAndSpit) and now and then stop to call down a barrage of
// effects on the player. Their CharState words: `state` = BARRAGE_RUNNING while the barrage runs (else 0), `wait` =
// frames since the barrage started.
#ifndef GURUMIN_BARRAGE_ENEMY_H
#define GURUMIN_BARRAGE_ENEMY_H

#define BARRAGE_RUNNING 20000           // CharState::state while a barrage runs
#define BARRAGE_CHANCE 250              // chance per frame (in 10000) that a barrage starts
#define BARRAGE_BLOCKING_LAYER 4        // no barrage starts while this motion layer plays

// Enemy_CallBarrage (0x100a77f0)
#define BARRAGE_CHARGE_EFFECT 0x3702    // started four times (frames 0, 3, 6 and 9) when the barrage starts
#define BARRAGE_CHARGE_SOUND 0x1c4      // played with it
#define BARRAGE_SHOT_EFFECT 0x429       // one shot, fired from a random point above the enemy at the player
#define BARRAGE_FIRST_SHOT 60           // shots are fired on the frames 60, 70, ... 110
#define BARRAGE_LAST_SHOT 110
#define BARRAGE_SHOT_PERIOD 10
#define BARRAGE_FRAMES 150              // the barrage ends on this frame
#define BARRAGE_SHOT_SPREAD 300         // the shots start up to +-150 units around the enemy ...
#define BARRAGE_SHOT_HEIGHT 100         // ... and 50 to 150 units above it
#define BARRAGE_AIM_HEIGHT 75.0f        // they aim this high above the player's feet
#define BARRAGE_SHOT_SCALE 1.44f        // EffectOptions::m_0008 of a shot

// Enemy_SpinFire (0x100a7db0)
#define SPIN_START_EFFECT 0x3701        // fired from the "@firenull" node when the spin starts and every 40 frames
#define SPIN_START_SOUND 0x1c3          // played when the spin starts
#define SPIN_SHOT_EFFECT 0x36ff         // fired 125 units above the enemy on the frames 30, 40, ... 120
#define SPIN_SHOT_SOUND 0x1c2           // played on those frames
#define SPIN_FIRST_SHOT 30
#define SPIN_LAST_SHOT 120
#define SPIN_SHOT_PERIOD 10
#define SPIN_FIRE_PERIOD 40             // SPIN_START_EFFECT again every this many frames
#define SPIN_SHOT_HEIGHT 125.0f
#define SPIN_RAMP_FRAMES 30             // the spin speeds up over this many frames and slows down over the last ones
#define SPIN_MAX_TURN 0.5236f           // radians a frame at full speed (30 degrees)
#define SPIN_EFFECT_SCALE 0.8f          // EffectOptions::m_0008 of its effects
#define EFFECT_OWNER_BASE 400           // EffectOptions::owner = character slot + this
#define FIRE_NODE_NAME "@firenull"      // node the fire effects start from (else the head)
#define RUN_MOTION 1                    // motion played while spinning

// Enemy_StarVolley (0x100a7150). CharState words: `state` 0 = starting, 1 = running; `talk` (0x08) = volleys left,
// `range` (0x0c) = frames to the next step of the volley, `action` (0x10) = frames before the next run may start.
#define STAR_POINT_COUNT 5              // shot points in a ring around the enemy (DAT_10868838[index][0..4])
#define STAR_POINT_STEP 1.25664f        // angle between the shot points (a fifth of a turn)
#define STAR_POINT_DISTANCE -50.0f      // the ring's radius (behind the enemy before turning)
#define STAR_POINT_HEIGHT 75.0f         // the ring's height above the enemy's feet
#define STAR_SHOT_EFFECT 600            // one shot, fired from a ring point at the player
#define STAR_SHOT_SOUND 0x1c5           // played with each step that fires
#define STAR_SHOT_SCALE 1.1f            // EffectOptions::m_0008 of a shot
#define STAR_START_DISTANCE 1000.0f     // runs start only while the player is nearer than this
#define STAR_MAX_VOLLEYS 10             // a run fires 1 to 10 volleys
#define STAR_VOLLEY_STEP 5              // a volley fires on the steps 10, 5 and 0 of its countdown
#define STAR_VOLLEY_START 10
#define STAR_VOLLEY_FRAMES 15           // the next run waits (volleys + 8) * this frames
#define STAR_VOLLEY_GAP 25              // countdown between volleys ...
#define STAR_VOLLEY_GAP_MODE2 15        // ... and when DAT_10c635f8 is 2
#define STAR_SIDE_ANGLE 1.25664f        // the second step fires two shots this far left and right of straight ahead
#define STAR_WIDE_ANGLE 2.51327f        // the third step fires two shots this far left and right
#define STAR_TURN_LIMIT 0.5236f         // radians the enemy turns per frame towards the player (30 degrees)
#define ONE_DEGREE 0.017453f            // radians
#define MIN_AIM_DISTANCE 1.0f           // nearer than this a shot goes straight ahead instead of up or down

// Enemy_AimedShots (0x100a6a50): the same words as Enemy_StarVolley; one shot point (DAT_10868838[index][0]) and the
// same ring constants, a run of 1 to 10 shots 15 frames apart.
#define AIMED_SHOT_GAP 15               // frames between two shots of a run
#define AIMED_RUN_EXTRA 4               // the next run waits (shots + this) * STAR_VOLLEY_FRAMES frames
#define AIMED_SIDE_ANGLE 0.5236f        // with DAT_10c635f8 == 2 two more shots fly 30 degrees left and right

// Enemy_CallBursts (0x100a8370): `state` = BARRAGE_RUNNING while it runs, `wait` = frames since it started (set to
// BURST_FIRED once the bursts are out).
#define BURST_MOTION_SPEED 15.0f        // AnimModel::AddMotionSpeed when the call starts
#define BURST_FIRED 1000                // `wait` jumps here once the bursts are fired ...
#define BURST_END 1060                  // ... and the enemy walks again after this
#define BURST_EFFECT 0x3705             // one burst
#define BURST_SCALE 1.3f                // EffectOptions::m_0008 of a burst
#define BURST_FRAME_SPREAD 30           // each burst starts 0 to 29 frames late
#define BURST_NODE_NAME "Bip01 Spine"   // the bursts follow this node of the player (else the head, else the player)

#define FACING_TOLERANCE_SQUARED 0.000304503f  // (one degree in radians) squared: closer than this counts as facing

// The behaviours (C++ functions: their temporaries are unwound on exceptions).
int Enemy_CallBarrage(int index);       // 0x100a77f0 (enemy_barrage.cpp)
int Enemy_SpinFire(int index);          // 0x100a7db0 (enemy_barrage.cpp)
int Enemy_StarVolley(int index);        // 0x100a7150 (enemy_barrage.cpp)
int Enemy_AimedShots(int index);        // 0x100a6a50 (enemy_barrage.cpp)
int Enemy_CallBursts(int index, int variant);  // 0x100a8370 (enemy_barrage.cpp)

#endif
