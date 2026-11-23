// Debris pieces: the models they are drawn with and the colour override they are drawn under (debris.cpp).
#ifndef GURUMIN_DEBRIS_H
#define GURUMIN_DEBRIS_H

#include "node.h"

#define DEBRIS_MODEL_COUNT 0x10   // models by piece type (index 0 is never drawn: type 0 = free slot)

extern "C" Node DAT_104d2a08[DEBRIS_MODEL_COUNT];   // the model of each piece type
extern "C" int DAT_10f655b4;                       // the colour index drawn pieces use (0 = none); set around the draw of a thrown piece

#endif
