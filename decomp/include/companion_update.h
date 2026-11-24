// The player's companions' updates (companion_update.cpp) and the status clamp (char_status.cpp), run by the characters
// that own companions (the player, the BS_006 boss).
#ifndef GURUMIN_COMPANION_UPDATE_H
#define GURUMIN_COMPANION_UPDATE_H

struct Matrix4;
struct Vec4;

// 0x100b0250, 0x100b0880, 0x100b0f00 (C++ linkage: the originals have an EH frame for their matrices): keep
// companion 0, 1 or 2 of character `index` (slot index + 2 + k) in range and run its revival.
int Companion0_Update(int index);
int Companion1_Update(int index);
int Companion2_Update(int index);

// 0x1001a840 (Char_ClampStatus, char_status.cpp): clamps the counter and level of character `index`.
extern "C" int FUN_1001a840(int index);

// 0x1001bfb0 (Char_LimitTurn, char_turn.cpp): limits how far character `index` turns this step to `maxTurn`.
extern "C" int FUN_1001bfb0(int index, float maxTurn);

// 0x1001c410 (group_tables.cpp): hands out a free shot slot of the kind-0x58 characters 0x5c..0x7b: stores `matrix`
// with the four values a..d in its fourth column and `value` as its state; the character and slot go to *outChar and
// *outSlot (the shot's matrix is DAT_10868838[*outChar][*outSlot]). Returns 1, or 0 when no slot is free.
extern "C" int FUN_1001c410(int value, const Matrix4 *matrix, int *outChar, int *outSlot, float a, float b, float c,
                            float d);

// 0x1001c4c0 (app_0012.cpp): clears the state of every kind-0x58 shot whose state lies in [low, high].
extern "C" int FUN_1001c4c0(int low, int high);

// 0x1001c510 (attach_slots.cpp, Point_Transform): transforms `point` by `matrix` in place.
extern "C" int FUN_1001c510(Vec4 *point, Matrix4 *matrix);

#endif
