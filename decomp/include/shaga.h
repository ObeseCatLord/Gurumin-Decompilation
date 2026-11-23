// Shaga slots: the models built by shaga_spawn.cpp (DAT_10446e00) with their timers (DAT_1021d018) and kinds
// (DAT_1042db08). Enemies look for a shaga to go to in shaga_seek.cpp.
#ifndef GURUMIN_SHAGA_H
#define GURUMIN_SHAGA_H

// Number of shaga slots (DAT_10446e00, DAT_1021d018, DAT_1042db08).
#define SHAGA_SLOT_COUNT 0x28

// One shaga an enemy could go to (shaga_seek.cpp): the shaga slot and its partner slot (-2 when the shaga's kind
// needs no partner).
struct ShagaCandidate
{
    int target;     // shaga slot to go to
    int partner;    // shaga slot of the partner, or -2
};

#endif
