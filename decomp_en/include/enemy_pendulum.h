// The pendulum enemy (EN build; JP: enemy_pendulum.cpp). This is the one declaration of these constants and of
// Enemy_Pendulum in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_ENEMY_PENDULUM_H
#define GURUMIN_EN_ENEMY_PENDULUM_H

// CharState::values entries of the pendulum (the talk id is set by the map: swing speed + 100 * flag).
const int PENDULUM_VALUE_WAIT = 0;     // frames since the current state started
const int PENDULUM_VALUE_TALK = 1;     // talk id: the swing speed in its last two digits, hundreds = does not wait to be told
const int PENDULUM_VALUE_ANGLE = 2;    // swing angle in tenths of a degree
const int PENDULUM_VALUE_BACK = 3;     // 1 = swinging back (speed rising), 0 = swinging forward
const int PENDULUM_VALUE_SWING = 4;    // swing speed in tenths of a degree a frame
const int PENDULUM_VALUE_FALL = 5;     // vertical speed while dropping into place
// The talk id splits into hundreds and the swing speed.
const int PENDULUM_TALK_SCALE = 100;
// CharState::state values.
const int PENDULUM_STATE_START = 0;     // places the model above its spot
const int PENDULUM_STATE_DROPPING = 1;  // fades in and falls to the pivot
const int PENDULUM_STATE_SWINGING = 2;  // swings on the line
const int PENDULUM_STATE_RELEASED = 3;  // let go, plays the released motion
// Line of sight check: every this many frames (staggered by slot), from this far above the feet, against this mask.
const int PENDULUM_SIGHT_PERIOD = 10;
const float PENDULUM_SIGHT_HEIGHT = 50.0f;
const unsigned int PENDULUM_SIGHT_MASK = 4;
// Height of the pivot above its place and of the model's start above it.
const float PENDULUM_PIVOT_HEIGHT = 100.0f;
const float PENDULUM_START_HEIGHT = 500.0f;
// Motions: hanging, and the one played after letting go; blend frames.
const int PENDULUM_MOTION_HANG = 3;
const int PENDULUM_MOTION_RELEASED = 2;
const int PENDULUM_MOTION_FRAMES = 5;
// Vertical speed change while dropping, and the squared distance to the pivot that ends the drop (20 units).
const int PENDULUM_FALL_STEP = 3;
const float PENDULUM_ARRIVE_SQUARED = 400.0f;
// Frames of the fade in.
const int PENDULUM_FADE_FRAMES = 30;
// Swing speed change per frame.
const int PENDULUM_SWING_STEP = 2;
// Length of the line it swings on, and the angle arithmetic (half a turn as the game writes it, tenths of a degree).
const float PENDULUM_LINE_LENGTH = 400.0f;
const float PENDULUM_HALF_TURN = 3.1415f;
const float PENDULUM_ANGLE_SCALE = 1800.0f;
// Distance within which the player makes it let go.
const float PENDULUM_LET_GO_RANGE = 800.0f;

// Runs one frame of pendulum enemy `index`. EN 0x6b10c0 (JP 0x10044560).
extern "C" int Enemy_Pendulum(int index);

#endif
