// Height maps of the ground objects (EN build; JP: decomp/include/ground_height.h, game_vars.h).
#ifndef GURUMIN_EN_GROUND_HEIGHT_H
#define GURUMIN_EN_GROUND_HEIGHT_H

// Number of ground object slots.
const int GROUND_OBJECT_SLOTS = 0x60;
// Floats per side of one height map (14x14 are used).
const int GROUND_MAP_SIDE = 16;

// Height samples per ground object: [object][x][y]. EN 0x00b498c0 (JP DAT_1049f200).
extern "C" float g_groundHeights[GROUND_OBJECT_SLOTS][GROUND_MAP_SIDE][GROUND_MAP_SIDE];
// Copy of g_groundHeights made when the heights are built. EN 0x00b61940 (JP DAT_1042e3f0).
extern "C" float g_groundHeightsCopy[GROUND_OBJECT_SLOTS][GROUND_MAP_SIDE][GROUND_MAP_SIDE];

// Fills the height map of ground object `slot` for hill style `style`. EN 0x5f4060 (JP 0x100169c0).
extern "C" void Ground_BuildHeights(int slot, int style);

#endif
