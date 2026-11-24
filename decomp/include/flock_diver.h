// The flock diver (enemy_flock_diver.cpp, kind 0x2c run from 0x100f9670): five fliers that circle the arena, rise out
// of the pit, swoop at the player and follow each other in chains.
#ifndef GURUMIN_FLOCK_DIVER_H
#define GURUMIN_FLOCK_DIVER_H

// 0x10086380 (Enemy_FlockDiver; C++ linkage: the original has an EH frame)
int Enemy_FlockDiver(int index);

extern "C" float DAT_10e44b74;   // read by the flock diver's chain steps (a height compared with a flier's)

#endif
