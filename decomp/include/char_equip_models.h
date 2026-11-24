// Putting on a character's equipment models (char_equip_models.cpp).
#ifndef GURUMIN_CHAR_EQUIP_MODELS_H
#define GURUMIN_CHAR_EQUIP_MODELS_H

// 0x100404e0 (Char_PutOnEquipment, C++ linkage: the original has an EH frame)
int FUN_100404e0(int index, int mount, int clothes, int head, int weapon, int skipKindSelect, int alternative);

extern "C" int FUN_1001b0a0(int index, int alternative, int quiet);  // Char_SelectKind

#endif
