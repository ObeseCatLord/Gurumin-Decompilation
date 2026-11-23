// The switch hold slots (switches.cpp, switch_effects.cpp, game_reset.cpp): each of the 128 slots records the object
// held, who holds it (a character slot below 0x80, or 0x80 + an object index) and the effect started on it.
#ifndef GURUMIN_SWITCH_SLOTS_H
#define GURUMIN_SWITCH_SLOTS_H

struct SwitchObject;

// Number of hold slots in each table.
#define SWITCH_HOLD_SLOTS 0x80
// Slots that Game_Reset clears: only the first half of each table (a leftover of a smaller table; Switch_Hold has
// the same 0x40 when the holder holds its own model).
#define SWITCH_HOLD_SLOTS_CLEARED 0x40
// Holders below this are character slots; 0x80 + n is object n.
#define SWITCH_HOLDER_OBJECT_BASE 0x80

extern "C" SwitchObject *DAT_1030bbf8[SWITCH_HOLD_SLOTS];  // object held in each slot (0 = slot unused)
extern "C" int DAT_10255b08[SWITCH_HOLD_SLOTS];            // holder of each slot
extern "C" int DAT_10219fc0[SWITCH_HOLD_SLOTS];            // effect kind of each slot (0, 1 or 2)
extern "C" int DAT_1042e0a8[SWITCH_HOLD_SLOTS];            // 1 once the slot's effect was started
extern "C" int DAT_10219c40[SWITCH_HOLD_SLOTS];            // 1 while the slot is set up (written by Switch_Hold)

#endif
