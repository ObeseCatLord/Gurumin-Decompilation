// Timed event slots of the EN build (JP: event_slots.cpp). This is the one declaration of these globals in
// decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_EVENT_SLOTS_H
#define GURUMIN_EN_EVENT_SLOTS_H

// Event slots that can be in use at once.
const int EVENT_SLOT_COUNT = 0x60;

extern "C" int g_eventSlotActive[EVENT_SLOT_COUNT];     // EN 0x00a1de00 (JP DAT_1020d660): 1 while the slot is in use
extern "C" int g_eventSlotElapsed[EVENT_SLOT_COUNT];    // EN 0x00a1e000 (JP DAT_104463f8): frames the event has run
extern "C" int g_eventSlotDuration[EVENT_SLOT_COUNT];   // EN 0x00b48fc0 (JP DAT_10468230): frames it lasts; below -100000 = waits for something else
extern "C" int g_eventSlotId[EVENT_SLOT_COUNT];         // EN 0x00915ab0 (JP DAT_10447fd8): event id
extern "C" char g_eventSlotFlag[EVENT_SLOT_COUNT];      // EN 0x009e1c30 (JP DAT_1021f640): flag given with the event

// Adds event `id` that waits for something else (duration -100000 - |value|). Returns 0. EN 0x5c9b00.
extern "C" int EventSlot_AddWaiting(int id, int value);
// Adds event `id` lasting |value| frames with `flag`. Returns 0. EN 0x5c9ba0.
extern "C" int EventSlot_Add(int id, int value, char flag);

#endif
