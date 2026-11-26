// Pickup effects ring and counter globals of the EN build (JP: decomp/include/char_vars.h, game_vars.h). Declared once here.
#ifndef GURUMIN_EN_PICKUPS_H
#define GURUMIN_EN_PICKUPS_H

// Slots of the pickup effect ring.
const int PICKUP_RING_SIZE = 32;
// Value of the event a pickup adds while it waits.
const int PICKUP_EVENT_VALUE = 2;
// Effect kind written into a new ring slot.
const int PICKUP_EFFECT_KIND = 5;
// Costume that doubles the counter step (guarded value 9).
const int PICKUP_COSTUME_DOUBLE = 11;
// Helmet face setting bits that raise the counter step by 2 and by 1.
const int PICKUP_HELMET_TRIPLE = 0x800000;
const int PICKUP_HELMET_DOUBLE = 0x400000;
// Banked hundreds stop at this value, or at the extended one when g_pickupBankExtended is set.
const int PICKUP_BANK_LIMIT = 300;
const int PICKUP_BANK_LIMIT_EXTENDED = 400;
// Frames the HUD counter stays up after a pickup.
const int PICKUP_HUD_FRAMES = 8;
// Frames a ring slot's effect runs before the slot is cleared.
const int PICKUP_EFFECT_LIFETIME = 30;

extern "C" int g_pickupRingIndex;                                // EN 0x009e2410 (JP DAT_10210fb8): next slot of the pickup effect ring
extern "C" int g_pickupEffectKind[PICKUP_RING_SIZE];             // EN 0x009e2528 (JP DAT_10255040): effect kind of each ring slot
extern "C" int g_pickupEffectHandle[PICKUP_RING_SIZE];           // EN 0x009e25a8 (JP DAT_102196f0): effect handle (-1 = none yet)
extern "C" int g_pickupEffectFrame[PICKUP_RING_SIZE];            // EN 0x009e2628 (JP DAT_1020d5c8): frames the effect has run
extern "C" int g_pickupBankExtended;                              // EN 0x01334878 (JP DAT_10c64ad0): non-zero = the stamina bank holds up to 400
extern "C" char g_hudCounterFrames;                               // EN 0x009e33da (JP DAT_104c0119): frames the HUD pickup counter stays up

// Collects a pickup (see pickup_collect.cpp). EN 0x5a1c60 (JP 0x1003ab40).
extern "C" void Pickup_Collect(int id, int quiet);

// Starts the next slot of the pickup effect ring with effect kind `kind` and effect handle `handle` (frame count 0) and
// advances the ring. Returns 0. EN 0x5a24d0.
extern "C" int PickupRing_Push(int kind, int handle);

// Ages every active ring slot by one frame and clears the ones past PICKUP_EFFECT_LIFETIME (see pickup_ring_age.cpp).
// Returns 0. EN 0x5ac910. Called once per frame by the effect update at EN 0x60cb10.
extern "C" int PickupRing_Age(void);

#endif
