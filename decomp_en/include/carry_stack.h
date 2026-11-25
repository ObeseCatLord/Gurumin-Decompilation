// Carried object stacking (EN build; JP: DAT_104bfdd8, DAT_104c0384, DAT_104c0388, DAT_10867df8). This is the one
// declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_CARRY_STACK_H
#define GURUMIN_EN_CARRY_STACK_H

#include "vec4.h"

extern "C" int g_carriedSlot;          // EN 0x00914554 (JP DAT_104bfdd8): slot of the object being carried (below the first object slot = none)
extern "C" int g_carriedSlotLast;      // EN 0x009160d8 (JP DAT_104c0388): the carried slot the stack search last ran for
extern "C" int g_stackTargetSlot;      // EN 0x009160d4 (JP DAT_104c0384): object slot the carried object rests on (0 = none yet)
extern "C" Vec4 g_stackPosition;       // EN 0x00dac930 (JP DAT_10867df8): position of the object below, z relative to the carried object

// Finds the object under the carried one and keeps the carried object on top of it. Returns 0. EN 0x6df520 (JP 0x1001d920).
extern "C" int Carry_StackOnObject(void);

#endif
