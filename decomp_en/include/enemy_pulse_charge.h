// The pulsing charging enemy (EN build; JP: decomp/include/pulse_charger.h). Declared once here. Used by src/enemy_pulse_charge.cpp.
#ifndef GURUMIN_EN_ENEMY_PULSE_CHARGE_H
#define GURUMIN_EN_ENEMY_PULSE_CHARGE_H

// One frame of the pulsing enemy `index` (kind 0x42): pulses, wanders and charges at the player once hit. Returns 0.
// C++ linkage (the original has an exception frame). EN 0x664f80 (JP 0x100a3900).
int Enemy_PulseAndCharge(int index);

#endif
