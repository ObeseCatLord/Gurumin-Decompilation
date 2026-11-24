// Ability names shown in the costume description (EN build). Declared once here.
#ifndef GURUMIN_EN_ABILITY_TEXT_H
#define GURUMIN_EN_ABILITY_TEXT_H

// Characters of an ability name buffer.
const int ABILITY_NAME_SIZE = 128;

// Copies the name of the first ability set in `flags` to `out` and returns its bit (0 = none). EN 0x6472b0 (JP 0x100144c0).
extern "C" unsigned int Ability_GetDescription(unsigned int flags, char *out);

// Writes the names of all abilities in `flags` to `out`, separated by a middle dot. EN 0x647990 (JP 0x1003bba0).
extern "C" int Ability_BuildNameList(unsigned int flags, char *out);

#endif
