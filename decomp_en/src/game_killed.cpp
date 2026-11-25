// Killed-character list and the DEAD events (EN build; JP twin: Game::MarkKilled, 0x10054770).
#include <string.h>
#include "game.h"

// Event name of the events raised once a group of ids is dead.
const char EVENT_DEAD[] = "DEAD";

// Number of the killed-list entry an id is looked up in: ids above the last entry share the last one.
inline int KilledSlot(int id)
{
    int slot = KILLED_LAST;
    if (id < slot)
        slot = id;
    return slot;
}

// Marks `id` (and entry 0) as killed, then registers every map event named "DEAD" that applies now and whose argument
// and five ids are all killed. Returns 0. Called when a scripted enemy or boss is defeated.
// EN 0x63c170 (JP 0x10054770).
int Game::MarkKilled(int id)
{
    killed[0] = 1;
    killed[id] = 1;
    for (int i = 0; i < mapEventCount; i++)
    {
        GameTrigger *event = &mapEvents[i];
        if (strcmp(event->name, EVENT_DEAD) == 0 && TriggerApplies(event, 1))
        {
            char count = 0;
            if (killed[KilledSlot(event->argument)] != 0)
                count++;
            if (killed[KilledSlot(event->ids[0])] != 0)
                count++;
            if (killed[KilledSlot(event->ids[1])] != 0)
                count++;
            if (killed[KilledSlot(event->ids[2])] != 0)
                count++;
            if (killed[KilledSlot(event->ids[3])] != 0)
                count++;
            if (killed[KilledSlot(event->ids[4])] != 0)
                count++;
            if (count == TRIGGER_ID_COUNT + 1)
                RegisterEvent(event->value);
        }
    }
    return 0;
}
