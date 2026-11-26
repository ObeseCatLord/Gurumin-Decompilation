// Steps and tuning of the barrage enemies (EN build; JP: decomp/include/barrage_enemy.h, enemy_barrage.cpp): enemies
// that walk with the common enemy behaviour (Enemy_ChaseLungeAndSpit) and now and then stop to call down a barrage of
// effects on the player. Their CharState words: `state` = BARRAGE_RUNNING while the barrage runs (else 0), `values[0]`
// = frames since the barrage started. This is the one declaration of these constants in decomp_en.
#ifndef GURUMIN_EN_BARRAGE_ENEMY_H
#define GURUMIN_EN_BARRAGE_ENEMY_H

// CharState::state while a barrage runs.
const int BARRAGE_RUNNING = 20000;
// Chance per frame (in 10000) that a barrage starts.
const int BARRAGE_CHANCE = 250;
const int BARRAGE_CHANCE_RANGE = 10000;
// No barrage starts while this motion layer plays.
const int BARRAGE_BLOCKING_LAYER = 4;
// Motion played while turning (walk) and once facing the player (idle).
const int BARRAGE_MOTION_WALK = 0;
const int BARRAGE_MOTION_IDLE = 2;
// Motion layer value of "no motion change".
const int BARRAGE_NO_MOTION = -1;

// Enemy_CallBarrage: the charge effect is started four times (frames 0, 3, 6 and 9) when the barrage starts.
const int BARRAGE_CHARGE_EFFECT = 0x3702;
const int BARRAGE_CHARGE_COPIES = 4;
const int BARRAGE_CHARGE_STEP = 3;
// The sound played with the charge.
const int BARRAGE_CHARGE_SOUND = 0x1c4;
// One shot, fired from a random point above the enemy at the player.
const int BARRAGE_SHOT_EFFECT = 0x429;
// Shots are fired on the frames 60, 70, ... 110.
const int BARRAGE_FIRST_SHOT = 60;
const int BARRAGE_LAST_SHOT = 110;
const int BARRAGE_SHOT_PERIOD = 10;
// The barrage ends on this frame.
const int BARRAGE_FRAMES = 150;
// The shots start up to +-150 units around the enemy and 50 to 150 units above it.
const int BARRAGE_SHOT_SPREAD = 300;
const float BARRAGE_SHOT_SPREAD_HALF = 150.0f;
const int BARRAGE_SHOT_HEIGHT = 100;
const float BARRAGE_SHOT_HEIGHT_MIN = 50.0f;
// They aim this high above the player's feet.
const float BARRAGE_AIM_HEIGHT = 75.0f;
// EffectOptions::scale of a shot.
const float BARRAGE_SHOT_SCALE = 1.44f;
// A character farther from the camera target than this is not heard (squared 2500) unless it is below FIRST_DISTANT_SLOT.
const float BARRAGE_HEARING_RANGE = 2500.0f;
// First slot whose sounds only play within the hearing range.
const int BARRAGE_FIRST_DISTANT_SLOT = 0x3a;
// Squared angle (one degree in radians) below which the enemy counts as facing the player.
const float FACING_TOLERANCE_SQUARED = 0.000304503f;

// Enemy_SpinFire: effect fired from the "@firenull" node (else the head) when the spin starts and every 40 frames.
const int SPIN_START_EFFECT = 0x3701;
// Sound played when the spin starts.
const int SPIN_START_SOUND = 0x1c3;
// Effect fired 125 units above the enemy on the frames 30, 40, ... 120.
const int SPIN_SHOT_EFFECT = 0x36ff;
// Sound played on those frames.
const int SPIN_SHOT_SOUND = 0x1c2;
const int SPIN_FIRST_SHOT = 30;
const int SPIN_LAST_SHOT = 120;
const int SPIN_SHOT_PERIOD = 10;
// SPIN_START_EFFECT again every this many frames.
const int SPIN_FIRE_PERIOD = 40;
const float SPIN_SHOT_HEIGHT = 125.0f;
// The spin speeds up over this many frames and slows down over the last ones.
const int SPIN_RAMP_FRAMES = 30;
// Radians a frame at full speed (30 degrees).
const float SPIN_MAX_TURN = 0.5236f;
// EffectOptions::scale of its effects.
const float SPIN_EFFECT_SCALE = 0.8f;
// EffectOptions::owner = character slot + this.
const int EFFECT_OWNER_BASE = 400;
// Node the fire effects start from (else the head).
const char SPIN_FIRE_NODE_NAME[] = "@firenull";
// Motion played while spinning.
const int SPIN_RUN_MOTION = 1;


#endif
