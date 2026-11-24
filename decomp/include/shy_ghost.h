// The shy ghost (0x10075350, behaviour 0x11 of the per-kind update 0x100f9670): an enemy that stays invisible
// (alpha 0, flagged busy in its slot) while the player looks at it, shows itself when the player looks away or comes
// within 300 units, creeps up behind the player, lunges and spits, and sinks away again when the player turns to it.
// A hit shows it for 46 frames before it sinks. Its CharState words: `state` = the step below, `wait` = frames in the
// step, `sightBlocked` = a wall between it and the player, `strafeFrames` = sidestep timer, `alerted` = has shown
// itself, `hurtFrames` = frames since it was hit.
#ifndef GURUMIN_SHY_GHOST_H
#define GURUMIN_SHY_GHOST_H

#define GHOST_STEP_START 0              // binds its head, spine and pelvis nodes, hides and starts the steps
#define GHOST_STEP_HIDDEN 1             // invisible: waits for the player to look away or come close
#define GHOST_STEP_STALK 2              // visible: creeps up, sidesteps, picks a lunge or a spit
#define GHOST_STEP_LUNGE 3              // lunges forward at the player (motion 4)
#define GHOST_STEP_SPIT 4               // faces the player and spits an effect from its head at motion frame 0xc0
#define GHOST_STEP_APPEAR 5             // fades in
#define GHOST_STEP_VANISH 6             // fades out, then waits hidden again
#define GHOST_STEP_REAPPEAR 7           // fades in again after a vanish, and plays the last appearance motion
#define GHOST_STEP_SINK 8               // after a hit: fades out slowly while the player looks away

// C++ linkage: the original has an EH frame.
int Enemy_ShyGhost(int index);          // 0x10075350 (enemy_shy_ghost.cpp)

#endif
