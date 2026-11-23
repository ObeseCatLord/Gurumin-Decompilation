// LastPosition: one record per character slot at DAT_10802bf8 (0x40 bytes each).
#ifndef GURUMIN_LAST_POSITION_H
#define GURUMIN_LAST_POSITION_H

#include "vec4.h"

// The position of the last frame and the last step of a character. Some movers reuse `position` for their own
// vector (char_move.cpp: the character's move vector; route_carrier.cpp: the velocity; char_reset.cpp and
// char_leash.cpp: the start position; collide_query.cpp: a direction).
struct LastPosition
{
    Vec4 position;            // 0x00
    Vec4 step;                // 0x10: the last step taken (route_walk.cpp); Enemy_HopAndStrafe: the last position
                              // it stood on ground, and its position while it hops
    Vec4 history2;            // 0x20: Enemy_HopAndStrafe: the position before `step`; while it hops, the step
                              // per frame back to `history3`
    Vec4 history3;            // 0x30: Enemy_HopAndStrafe: the oldest position kept, where a hop lands
};

extern "C" LastPosition DAT_10802bf8[0x80];  // per character slot (CHAR_SLOT_COUNT)

#endif
