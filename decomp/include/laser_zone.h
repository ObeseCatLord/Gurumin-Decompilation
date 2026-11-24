// The laser zone (map record type 6 of the area zone character, kind 0x23): sweeping beams inside a box of the map.
#ifndef GURUMIN_LASER_ZONE_H
#define GURUMIN_LASER_ZONE_H

#define LASER_PATTERN_COUNT 12         // beam pairs a zone can run at once
#define LASER_CHILD_COUNT 24           // beam nodes "laserc00".."laserc23" (two per pair) under the node "@tekiatari1"
#define LASER_START_WAIT 0x40          // frames between two new beam pairs at the least
#define LASER_START_CHANCE 100         // out of 1000 per frame once the wait is over
#define LASER_HIDDEN 0x20              // node flag: the model itself is not drawn, only its beam effects
#define LASER_EFFECT 0x768             // the effect that draws a pair: a beam between its two nodes
#define LASER_EFFECT_OWNER 1000        // + the pair's number: the owner id of its effect
#define LASER_EFFECT_VALUE 0xf         // EffectOptions::m_0004 of the beam effect
#define LASER_SPEED_EASY 0.75f         // speed factor when DAT_10c635f8 is 1
#define LASER_SPEED_HARD 1.25f         // and when it is 2
#define LASER_CORNER_STEP_SHORT 8.0f   // corner chasers: x step + y step (flag 0)
#define LASER_CORNER_STEP_LONG 16.0f   // and with the flag set
#define LASER_CODE_TYPE 100            // pattern code = flag * 10000 + speed * 100 + type
#define LASER_CODE_FLAG 10000

// Pattern types: where the pair starts and how it moves. Types 0 to 3 sweep both ends together along one axis from
// one side of the box to the other; types 4 to 7 start both ends in one corner and send them round the box in
// opposite directions.
#define LASER_SWEEP_UP 0               // along the lower y edge, moving up
#define LASER_SWEEP_DOWN 1             // along the upper y edge, moving down
#define LASER_SWEEP_RIGHT 2            // along the lower x edge, moving right
#define LASER_SWEEP_LEFT 3             // along the upper x edge, moving left
#define LASER_CORNER_LOW_LOW 4         // from (x0, y0)
#define LASER_CORNER_HIGH_LOW 5        // from (x1, y0)
#define LASER_CORNER_LOW_HIGH 6        // from (x0, y1)
#define LASER_CORNER_HIGH_HIGH 7       // from (x1, y1)

// 0x1002d4e0 (Char_LaserZone, laser_zone.cpp)
int Char_LaserZone(int index);

#endif
