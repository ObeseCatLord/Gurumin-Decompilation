// Timed event slots (EN build). JP twin: event_slots.cpp.
#include "event_slots.h"

// A waiting event's duration is this minus |value|: the update treats such slots as waiting for something else.
const int EVENT_WAIT_BASE = -100000;

// Adds an event `id` with a negative duration (-100000 - |value|) to the first free slot: the update treats such
// slots as waiting for something else rather than counting frames. Nothing happens when all slots are in use.
// Returns 0. Called through the script command table.
extern "C" int EventSlot_AddWaiting(int id, int value)
{
    int i;
    for (i = 0; i < EVENT_SLOT_COUNT; i++)
    {
        if (g_eventSlotActive[i] == 0)
            break;
    }
    if (i != EVENT_SLOT_COUNT)
    {
        g_eventSlotActive[i] = 1;
        g_eventSlotElapsed[i] = 0;
        if (value < 0)
            value = -value;
        g_eventSlotDuration[i] = EVENT_WAIT_BASE - value;
        g_eventSlotId[i] = id;
    }
    return 0;
}

// Adds an event `id` lasting |value| frames with `flag` to the first free slot (nothing happens when all slots are
// in use). Returns 0. Called through the script command table.
extern "C" int EventSlot_Add(int id, int value, char flag)
{
    int i;
    for (i = 0; i < EVENT_SLOT_COUNT; i++)
    {
        if (g_eventSlotActive[i] == 0)
            break;
    }
    if (i != EVENT_SLOT_COUNT)
    {
        g_eventSlotActive[i] = 1;
        g_eventSlotElapsed[i] = 0;
        if (value < 0)
            value = -value;
        g_eventSlotDuration[i] = value;
        g_eventSlotId[i] = id;
        g_eventSlotFlag[i] = flag;
    }
    return 0;
}
