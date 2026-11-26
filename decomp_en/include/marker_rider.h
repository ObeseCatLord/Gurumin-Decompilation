// A character riding a marker (EN build; JP: decomp/src/marker_rider.cpp).
#ifndef GURUMIN_EN_MARKER_RIDER_H
#define GURUMIN_EN_MARKER_RIDER_H

#include "char.h"

// Display state of each character slot (2 = down, 7 = idle, 0x1e = revived). EN 0x00915a28 (JP DAT_104686b0).
extern "C" unsigned char g_charPoseState[CHAR_SLOT_COUNT];

// Rides a taken marker until it falls out of the world, then vanishes; returns 1 while riding or vanishing, else 0.
// EN 0x6adbd0 (JP 0x10043fc0).
int Npc_RideMarker(int index);

#endif
