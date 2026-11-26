// The flying carrier NPC (EN build; JP: decomp/include/npc_hover_carrier.h). Declared once here. Used by src/npc_hover_carrier.cpp.
#ifndef GURUMIN_EN_NPC_HOVER_CARRIER_H
#define GURUMIN_EN_NPC_HOVER_CARRIER_H

#include "char.h"

// Frames each character slot has been awake (counts up while it is near the player); the carrier zeroes its rider's count.
// EN 0x00bc8ea8 (JP DAT_10254598).
extern "C" int g_charAwakeTimer[CHAR_SLOT_COUNT];

// One frame of the carrier `index` (behaviour 0x21): floats to the player, bounces off walls and holds its rider. Returns 0.
// EN 0x6d3a00 (JP 0x10048050).
int Npc_HoverCarrier(int index);

#endif
