// Anti-tamper guard copies of the EN build (JP: anti_tamper.cpp): each guarded value has a copy stored as
// ~value + slot * 0x41, which the game compares to detect memory editing. This is the one declaration of these
// globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_ANTI_TAMPER_H
#define GURUMIN_EN_ANTI_TAMPER_H

// Slots of the guard copy table: ten single values, 0x50 inventory counts, then 15 more values from slot 100.
const int GUARD_SLOT_COUNT = 115;
// Added per slot index to the inverted value of single values.
const int GUARD_SLOT_STEP = 0x41;

extern "C" int g_guardCopies[GUARD_SLOT_COUNT];    // EN 0x009e13e0 (JP DAT_104684b0): the guard copies
extern "C" int g_guardedValue5;                    // EN 0x013364b4 (JP DAT_10c6670c): guarded value, slot 5
extern "C" int g_guardedValue6;                    // EN 0x0133487c (JP DAT_10c64ad4): guarded value, slot 6
extern "C" int g_guardedValue7;                    // EN 0x01334880 (JP DAT_10c64ad8): guarded value, slot 7
extern "C" int g_guardedValue8;                    // EN 0x013333a0 (JP DAT_10c635f8): guarded value, slot 8
extern "C" int g_guardedValue9;                    // EN 0x01335398 (JP DAT_10c655f0): guarded value, slot 9 (the game mode)

// Refreshes the guard copies of the ten single values. EN 0x5f4650.
extern "C" int AntiTamper_StoreValues(void);

#endif
