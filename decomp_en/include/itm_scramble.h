// Scrambling of the game's .itm data files (EN build; JP: decomp/src/itm_scramble.cpp, docs/ITM_FORMAT.md).
#ifndef GURUMIN_EN_ITM_SCRAMBLE_H
#define GURUMIN_EN_ITM_SCRAMBLE_H

// Reverses the .itm scrambling of `size` bytes in place using a key made from `name` (upper-cased in place). Returns 1
// when the byte sum and xor of the plain data equal `sum` and `check`, else 0. EN 0x71c8d0 (JP 0x101615d0).
extern "C" int Itm_Descramble(unsigned char *data, int size, char *name, int sum, unsigned int check);

#endif
