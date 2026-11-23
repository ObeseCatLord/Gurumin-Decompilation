// The map cursor behaviour (character behaviour 8): its per-frame update and the route step it uses.
#ifndef GURUMIN_MAP_CURSOR_H
#define GURUMIN_MAP_CURSOR_H

// 0x100249c0 (actor_scan.cpp): non-zero when a character stands within the given height and squared distance.
extern "C" int FUN_100249c0(int lower, int upper, int maxHeight, int maxDistanceSquared);

// 0x10051190 (map_cursor.cpp, MapCursor_Step): moves a cursor along its route record.
extern "C" int FUN_10051190(int index, int step, int *outStep, float *x, float *y, int mode, int target, int unused,
                            int *kind, int *previousStep, int *wasRidden);

// 0x10051570 (map_cursor_walk.cpp, MapCursor_Walk): one frame of the map cursor character `index`.
int MapCursor_Walk(int index);

#endif
