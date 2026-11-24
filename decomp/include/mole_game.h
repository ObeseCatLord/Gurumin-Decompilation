// The map 0x17 mole game (mole_game.cpp): two mole characters (slots 0x3a and 0x3b) run about, dive into one of nine
// holes (the map's "@p1".."@p9" nodes) and come up at another; hitting one scores a point.
#ifndef GURUMIN_MOLE_GAME_H
#define GURUMIN_MOLE_GAME_H

#include "matrix4.h"

extern "C" int DAT_10c66d14;       // the mini game mode (4 = the one with the count-down banner and its sounds)
extern "C" int DAT_1020c32c;       // the mini game score as the score counter last saw it
extern "C" char DAT_102540ca;      // cleared when the mini game starts
extern "C" char DAT_102540cb;      // cleared when the mini game starts

extern "C" int FUN_1001e1a0(const char *name, const char *alphaName);   // 0x1001e1a0 (texture_load.cpp): loads a texture
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);   // Char_StartEffect

int Map17_UpdateMoleGame();    // 0x100524d0

#endif
