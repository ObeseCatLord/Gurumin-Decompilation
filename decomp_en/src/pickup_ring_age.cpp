// Per-frame aging of the pickup effect ring (EN build, EN-only: no JP twin found).
#include "pickups.h"

// PickupRing_Age: every slot that holds an effect counts one more frame; after PICKUP_EFFECT_LIFETIME frames its
// frame count and effect kind are cleared so the slot is free again. Returns 0. Called once per frame (EN 0x60cb10).
extern "C" int PickupRing_Age(void)
{
    for (int slot = 0; slot < PICKUP_RING_SIZE; slot++)
    {
        if (g_pickupEffectKind[slot] != 0)
        {
            g_pickupEffectFrame[slot]++;
            if (g_pickupEffectFrame[slot] > PICKUP_EFFECT_LIFETIME)
            {
                g_pickupEffectFrame[slot] = 0;
                g_pickupEffectKind[slot] = 0;
            }
        }
    }
    return 0;
}
