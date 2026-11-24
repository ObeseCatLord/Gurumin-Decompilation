// The perching swooper (perch_swooper.cpp, behaviour 10 of Enemy_UpdateBehaviour 0x100bcb30): a flier that sits on a
// perch until the player comes near, takes off, flies at the player and now and then swoops down to a landing point.
// Its CharState words: `state` = the step below, `wait` = frames in the step (negative while it circles), `startHeight`
// = the player's height when it took off, `anchorPoint` = the landing point it flies to, `flapTicks` = wing beats.
// DAT_10255160[slot][0] is its flap speed. The four LastPosition vectors of its slot hold: [0] the smoothed flight
// vector, [1] the perch where it started, [2] and [3] landing points found on the ground nearby ([3] also follows the
// player while the player stands still); x <= PERCH_UNSET_LIMIT = no point yet.
#ifndef GURUMIN_PERCH_SWOOPER_H
#define GURUMIN_PERCH_SWOOPER_H

#define PERCH_STEP_START 0              // binds nothing yet: sets the perch point
#define PERCH_STEP_PERCHED 1            // waits on the perch for the player
#define PERCH_STEP_CLIMB 2              // rises straight up
#define PERCH_STEP_FLY 3                // flies at the player; may pick a landing point and swoop
#define PERCH_STEP_SWOOP 4              // flies to the landing point
#define PERCH_STEP_LAND 5               // settles on the landing point

// C++ linkage: the original has an EH frame.
int Enemy_PerchAndSwoop(int index);     // 0x100a5960 (perch_swooper.cpp)

#endif
