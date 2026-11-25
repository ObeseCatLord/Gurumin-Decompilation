// Collecting a counted pickup (EN build).
#include "char.h"
#include "anti_tamper.h"
#include "event_slots.h"
#include "char_attach.h"
#include "pickups.h"

// Collects a pickup: unless `quiet`, event `id` waits with value 2. A pickup effect goes into the ring of
// PICKUP_RING_SIZE slots and the player's pickup counter goes up by 1, by 2 with costume 11 or helmet bit 0x400000, by
// 3 with helmet bit 0x800000. Past 99 a hundred is banked and the counter restarts at 0, or it stays at 99 when the
// bank is full. The HUD counter shows for 8 frames and the anti-tamper copies are refreshed. Called when a pickup is
// touched. EN 0x5a1c60 (JP 0x1003ab40).
extern "C" void Pickup_Collect(int id, int quiet)
{
    if (quiet == 0)
        EventSlot_AddWaiting(id, PICKUP_EVENT_VALUE);
    g_pickupEffectKind[g_pickupRingIndex] = PICKUP_EFFECT_KIND;
    g_pickupEffectHandle[g_pickupRingIndex] = -1;
    g_pickupEffectFrame[g_pickupRingIndex] = 0;
    g_pickupRingIndex++;
    if (g_pickupRingIndex > PICKUP_RING_SIZE - 1)
        g_pickupRingIndex = 0;
    int count = g_charRecords[0].stats[STAT_PICKUPS] + 1;
    if (g_guardedValue9 == PICKUP_COSTUME_DOUBLE)
        count++;
    else if (g_helmetFaceSettings & PICKUP_HELMET_TRIPLE)
        count += 2;
    else if (g_helmetFaceSettings & PICKUP_HELMET_DOUBLE)
        count++;
    if (count > 99)
    {
        int limit = PICKUP_BANK_LIMIT;
        if (g_pickupBankExtended != 0)
            limit = PICKUP_BANK_LIMIT_EXTENDED;
        if (g_guardedValue7 >= limit)
            count = 99;
        else
        {
            g_guardedValue7 += 100;
            count = 0;
        }
    }
    g_charRecords[0].stats[STAT_PICKUPS] = count;
    g_hudCounterFrames = PICKUP_HUD_FRAMES;
    AntiTamper_StoreValues();
}
