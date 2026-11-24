// Water and ice of the current map (EN build; JP: water_ice.cpp). This is the one declaration of these globals in
// decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_WATER_ICE_H
#define GURUMIN_EN_WATER_ICE_H

#include "node.h"

// Node of the map's ice (its first child is the ice surface). EN 0x009e3350 (JP DAT_104c0080); 0 = the map has no ice.
extern "C" Node *g_iceNode;
// Node of the map's water collision. EN 0x009e3354 (JP DAT_104c0084); 0 = the map has no water.
extern "C" Node *g_waterNode;
// Non-zero while the water of the map is frozen. EN 0x00b79941 (JP DAT_104c02dc).
extern "C" char g_waterFrozen;

// Freezes (`frozen` != 0) or thaws the water of the current map. EN 0x644c90 (JP 0x10011fb0).
extern "C" int Water_SetFrozen(int frozen);

#endif
