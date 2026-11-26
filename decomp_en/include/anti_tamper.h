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

// Guarded extras: 15 values whose copies are slots 100..114, and the offsets of the two guard formulas.
const int GUARD_ITEM_FIRST_SLOT = 10;  // slot of item 0
const int GUARD_ITEM_COUNT = 0x50;    // items guarded
const int GUARD_EXTRA_COUNT = 15;
const int GUARD_EXTRA_FIRST_SLOT = 100;
const int GUARD_STOCK_BASE = 0x14;
const int GUARD_EXTRA_BASE = 6500;
const int GUARD_EXTRA_STEP = 2;
extern "C" int g_guardedExtras[GUARD_EXTRA_COUNT];  // EN 0x01335eec (JP DAT_10c66144): guarded extra values

// Refreshes the guard copies of the ten single values. EN 0x5f4650.
extern "C" int AntiTamper_StoreValues(void);
// Refreshes the guard copies of the item stock and the extra values. Returns 0. EN 0x5f4700.
extern "C" int AntiTamper_StoreArrays(void);

// Entries of the noise table AntiTamper_Check writes random values into when a guard copy does not match.
const int TAMPER_NOISE_COUNT = 16;
extern "C" int g_tamperNoise[TAMPER_NOISE_COUNT];   // EN 0x00a363a8 (JP DAT_104c0270): anti-tamper noise table
extern "C" int g_debuggerDetected;                  // EN 0x0217fb04 (JP DAT_11b044f0): non-zero = a debugger was found (the check then adds noise)
extern "C" short g_tamperCheckDepth;                // EN 0x00bc8890 (JP DAT_104c034c): raised while AntiTamper_Check runs, lowered when it ends
// Compares every guard copy with its value, adds noise to g_tamperNoise for each mismatch and checks the array copies
// every 30 frames. Returns 0. EN 0x5b43f0 (JP 0x1000e870).
extern "C" int AntiTamper_Check(void);

#endif
