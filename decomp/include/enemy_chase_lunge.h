// The common enemy behaviour (0x10076790, the default case of Enemy_UpdateBehaviour 0x100bcb30, behaviour 11 with
// variant 1, and the walking part of the barrage enemies of enemy_barrage.cpp): it chases the player, circles round
// obstacles, lunges at him from close range, spits now and then and wanders when it cannot get on. Its CharState
// words: `state` = the step below, `wait` = frames in the step.
#ifndef GURUMIN_ENEMY_CHASE_LUNGE_H
#define GURUMIN_ENEMY_CHASE_LUNGE_H

// Steps of CharState::state (values of 200 and more count down to 200 and then start a lunge).
enum ChaserStep
{
    CHASER_STEP_CHASE = 0,          // walks at the player, circles round obstacles, may lunge or spit
    CHASER_STEP_LUNGE = 1,          // lunges along a fixed direction for 40 frames (hits on frames 4 to 34)
    CHASER_STEP_WANDER = 2          // wanders and looks around while the way to the player is shut
};

// Motions.
#define CHASER_MOTION_WALK 0
#define CHASER_MOTION_RUN 1             // played while it is dazed
#define CHASER_MOTION_IDLE 2
#define CHASER_MOTION_LUNGE 4
#define CHASER_MOTION_BLEND 5           // blend frames of every motion change
#define CHASER_NO_MOTION (-1)

// Tuning.
#define CHASER_VARIANT_SPEED 1.5f       // variant 1: motion speed scale of the slot times this
#define CHASER_PI 3.1415f               // pi as the game writes it
#define CHASER_PI_LONG 3.14159f         // ... and the longer form used by the wander turn
#define CHASER_QUARTER_TURN 1.57075f
#define CHASER_PIT_MARGIN 25.0f         // below the pit floor by this much it counts as over the pit
#define CHASER_PIT_LEAVE_MARGIN 15.0f   // ... and it leaves the pit state once above floor - this
#define CHASER_NEAR_RANGE 300.0f        // nearer than this it does not circle in from afar and may lunge
#define CHASER_LUNGE_ROLL 15            // chance in 100 per frame that it lunges
#define CHASER_LUNGE_COOLDOWN 200       // `state` counts down from here to start a lunge
#define CHASER_LUNGE_FRAMES 40          // a lunge lasts this many frames
#define CHASER_LUNGE_HIT_FROM 3         // frames 4 to 34 of the lunge hit (DAT_1021f6c0)
#define CHASER_LUNGE_HIT_TO 35
#define CHASER_LUNGE_SLOW_BEFORE 5      // before frame 5 and after frame 30 the lunge only creeps
#define CHASER_LUNGE_SLOW_AFTER 30
#define CHASER_LUNGE_CREEP 0.3f         // the creep is this fraction of the full lunge step
#define CHASER_LUNGE_START_FRAME 4      // its start position is saved on this frame (DAT_10868838[index][0])
#define CHASER_LUNGE_STEP 5.0f          // the lunge step is the direction to the player times this ...
#define CHASER_LUNGE_STEP_LONG 3.0f     // ... times this (15 units a frame)
#define CHASER_LUNGE_EFFECT 0x655       // the lunge's effect, started on its first frame
#define CHASER_LUNGE_SOUND 0x2cc
#define CHASER_LUNGE_PIT_JUMP 50.0f     // vertical speed of a lunge started over the pit
#define CHASER_LUNGE_DELAY 45           // DAT_10254390 after a lunge (15 for variant 1)
#define CHASER_LUNGE_DELAY_VARIANT 15
#define CHASER_LUNGE_CIRCLE_RANGE 100.0f   // ends within speed scale * this of its start: it circles for a while
#define CHASER_CIRCLE_FRAMES 30         // frames of circling
#define CHASER_CIRCLE_TURN 0.62832f     // a lunge blocked by a ledge turns by this much per frame (36 degrees)
#define CHASER_CIRCLE_SIDE_FRAMES 6     // ... for this many frames, one way or the other
#define CHASER_SIGHT_DEPTH 300.0f       // ledge probes look this far down ...
#define CHASER_SIGHT_REACH 15           // ... this many probe lengths ahead
#define CHASER_SIGHT_STEP (-6.0f)       // the way ahead is probed with a step of this length (and 24 high)
#define CHASER_STEP_HEIGHT 24.0f
#define CHASER_PROBE_THIRD 3            // a lunging enemy probes every 3rd frame by slot, a circling one every 10th
#define CHASER_PROBE_TENTH 10
#define CHASER_RETEST_PERIOD 60         // a halted enemy re-tests its way every 60th frame
#define CHASER_WANDER_STEPS 120         // turn around on the spot for 120 frames, then idle
#define CHASER_WANDER_END 210
#define CHASER_WANDER_START_BEARING 180 // the first wander frame turns by up to half of this many degrees
#define CHASER_WANDER_ROLL 20           // later turns when blocked: up to 10 degrees either way
#define CHASER_WANDER_RESUME 0x77       // `wait` after a blocked step: 120 frames are over at once
#define CHASER_WANDER_WALK_AFTER 30     // the chase only resumes after this many wander frames
#define CHASER_SIDE_ROLL 120            // circling lasts 30 to 149 frames in one direction
#define CHASER_SIDE_MIN 30
#define CHASER_FLIP_ROLL 60             // 1 in 6 frames, a blocked circler flips direction
#define CHASER_FLIP_LIMIT 10
#define CHASER_GIVE_UP_ROLL 10          // 7 in 10: a halted enemy wanders (step 2)
#define CHASER_GIVE_UP_LIMIT 7
#define CHASER_PIT_STEP 4.0f            // z of the move over the pit (it pushes itself up)
#define CHASER_PIT_PROBE 12.0f          // over the pit it probes 12 units up
#define CHASER_PIT_LAND 1.0f            // z move when that is blocked
#define CHASER_PIT_RISE 12.0f           // ... and at least this when it is free
#define CHASER_DAZE_PERIOD 2            // the dazed sparkle every second frame
#define CHASER_DAZE_EFFECT 0x2da
#define CHASER_SPIT_LEVELS 8            // it spits after (8 - shotRate) * 10 frames
#define CHASER_SPIT_PERIOD 10
#define CHASER_SPIT_EFFECT 0x258        // the spit, started at its head
#define CHASER_SPIT_SOUND 0x2cd
#define CHASER_SPIT_FORWARD 50.0f       // the spit starts this far in front of the head
#define CHASER_SPIT_AIM_STEP 5.0f
#define CHASER_FLASH_COLOUR 0xfe        // identification colours of the body and shadow nodes
#define CHASER_SHADOW_COLOUR 0xff
#define CHASER_COLOUR_BLUE 0x2f
#define CHASER_BODY_MASK 0x400
#define CHASER_SHADOW_MASK 0x200
#define CHASER_OUT_OF_PLAY (-50001.0f)  // x of DAT_104c4438 of a character that was an idle NPC
#define CHASER_HEAD_NODE "Bip01 Head"
#define CHASER_SPINE_NODE "Bip01 Spine"

// C++ linkage: the original has an EH frame (its Matrix4 temporaries are unwound on exceptions).
int Enemy_ChaseLungeAndSpit(int index, int variant);   // 0x10076790 (enemy_chase_lunge.cpp)

#endif
