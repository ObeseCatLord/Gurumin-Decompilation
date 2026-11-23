// Steps and tuning of the barrage enemies (behaviour numbers of Enemy_UpdateBehaviour 0x100bcb30, enemy_barrage.cpp):
// enemies that walk with the common enemy behaviour (FUN_10076790) and now and then stop to call down a barrage of
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

#define FACING_TOLERANCE_SQUARED 0.000304503f  // (one degree in radians) squared: closer than this counts as facing

// The behaviours (C++ functions: their temporaries are unwound on exceptions).
int Enemy_CallBarrage(int index);       // 0x100a77f0 (enemy_barrage.cpp)
int Enemy_SpinFire(int index);          // 0x100a7db0 (enemy_barrage.cpp)

#endif
