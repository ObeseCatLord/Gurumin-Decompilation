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

struct Vec4;
struct Matrix4;

// 0x10042140 (Shaga_BreakOff, shaga_spawn.cpp; C++ linkage). An equipment piece of `model` breaks off character `index`.
int FUN_10042140(int index, const char *model, int radius, const Vec4 *position, const Matrix4 *matrix, int scaled,
                 float force, int tired);

// 0x1001af10 (Shaga_PushOut, shaga_pushout.cpp; C++ linkage): moves shaga model `index` out of the world collision.
int Shaga_PushOut(int index);

// 0x10042a30 (Shaga_Update, shaga_update.cpp; C++ linkage): the per-frame update of all shaga slots; the scene frame
// calls it.
int FUN_10042a30(void);

extern "C" char DAT_104c011f;   // HUD timer of the money counter (set to 8 when money was added)

#endif
