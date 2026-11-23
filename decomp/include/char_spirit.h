// The spirit character (behaviour 0x25 of the per-kind update 0x100f9670, char_spirit.cpp): a ghost that appears
// for SPIRIT_VISIT_FRAMES frames. CharState words: spiritTimer counts the visit down, spiritHaunting is 1 while at
// least SPIRIT_HAUNT_FRAMES frames are left, spiritSize is 100 or 200 (the swarm size), spiritStep the motion step.
#ifndef GURUMIN_CHAR_SPIRIT_H
#define GURUMIN_CHAR_SPIRIT_H

#define SPIRIT_VISIT_FRAMES 0x5eb       // the timer starts here: the spirit dims over its first 15 frames
#define SPIRIT_DIM_END 0x5dc            // timer value at which the dimming has finished
#define SPIRIT_DIM_FRAMES 15.0f
#define SPIRIT_BRIGHTEN_FRAMES 90.0f    // it brightens again over the last 90 frames
#define SPIRIT_DIM_STRENGTH 0.7f        // the darkest tint is 1 - this
#define SPIRIT_HAUNT_FRAMES 0x96        // while at least this many frames are left it haunts, then it floats
#define SPIRIT_SPARK_PERIOD 15          // haunting: an effect every 15 frames
#define SPIRIT_SPARK_EFFECT 0x65
#define SPIRIT_SMALL_SIZE 100           // spiritSize with the "@O_047b" node
#define SPIRIT_LARGE_SIZE 200
#define SPIRIT_SMALL_RADIUS 90.0f       // spark distance for the small size (large: SPIRIT_LARGE_RADIUS)
#define SPIRIT_LARGE_RADIUS 180.0f
#define SPIRIT_SMALL_HEIGHTS 0x73
#define SPIRIT_LARGE_HEIGHTS 0xfa
#define SPIRIT_DEGREES 360
#define SPIRIT_HALF_TURN 180.0f
#define SPIRIT_PI 3.1415f
#define SPIRIT_DRIFT_SPEED 12.0f        // units a frame towards the motion step's target
#define SPIRIT_DRIFT_FRAMES 150.0f
#define SPIRIT_SINK_SCALE 2.5f          // moving down is sped up by this
#define SPIRIT_MOVED_SQUARED 10.0f      // squared distance from the last position that counts as moving
#define SPIRIT_SWARM_PERIOD 4           // a swarm every 4th frame of moving
#define SPIRIT_SWARM_UNIT 50            // swarm count = spiritSize / this
#define SPIRIT_SWARM_SLOTS 24           // random slots of a full circle (distinct per swarm)
#define SPIRIT_SWARM_HALF 12.0f
#define SPIRIT_SWARM_EFFECT_BASE 0x460  // + spiritSize / 100
#define SPIRIT_EFFECT_LIST 3

int Char_SpiritUpdate(int index);   // Char_SpiritUpdate

#endif
