// The NPC that rides behind a marker character (EN build; JP: decomp/include/npc_ride.h). Declared once here. Used by src/npc_ride.cpp.
#ifndef GURUMIN_EN_NPC_RIDE_H
#define GURUMIN_EN_NPC_RIDE_H

#include "char.h"

// CharState::values index holding the slot of the marker character the rider follows (-1 = none).
const int RIDE_VALUE_MARKER = 2;

// One frame of the rider `index`: follows the nearest marker character at a fixed distance. Returns 0. EN 0x6d14f0
// (JP 0x10047a10).
int Npc_RideBehindMarker(int index);

#endif
