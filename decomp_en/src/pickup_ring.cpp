// The pickup effect ring (EN build, EN-only: Pickup_Collect has the same push open-coded).
#include "pickups.h"

// PickupRing_Push: fills the next ring slot with the effect kind and handle, resets its frame count and moves on to the
// following slot (wrapping after the last). Returns 0. No direct caller (reached through a pointer).
extern "C" int PickupRing_Push(int kind, int handle)
{
    int slot = g_pickupRingIndex;
    g_pickupEffectKind[slot] = kind;
    g_pickupEffectHandle[slot] = handle;
    g_pickupEffectFrame[slot] = 0;
    slot++;
    if (slot > PICKUP_RING_SIZE - 1)
        slot = 0;
    g_pickupRingIndex = slot;
    return 0;
}
