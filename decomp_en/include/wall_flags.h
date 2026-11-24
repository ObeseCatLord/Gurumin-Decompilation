// Map-specific invisible walls (EN build). Declared once here.
#ifndef GURUMIN_EN_WALL_FLAGS_H
#define GURUMIN_EN_WALL_FLAGS_H

// Nonzero once the four probe meshes of the wall map have been hidden. EN 0x01333118 (JP DAT_10c63370).
extern "C" int g_wallProbesHidden;

// Updates the four walls of the current map. EN 0x5a80e0 (JP Map_UpdateWalls).
extern "C" int Map_UpdateWalls(void);

#endif
