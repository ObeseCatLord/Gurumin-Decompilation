// Queued event registration (JP twin: flag_list.cpp, FlagOwner::Register).

#include "game.h"

// Registers event `id` once: returns 1 if it is already in the queue or was added to the first free entry (with the
// handled flag set when `id` is also among the events being run), 0 when the queue is full. Called by the event
// triggers of the map scripts.
int Game::RegisterEvent(int id)
{
    int running = 0;
    int i;
    for (i = 0; i < QUEUED_EVENT_COUNT; i++)
    {
        if (runningEvents[i] == id)
            running = 1;
    }
    for (i = 0; i < QUEUED_EVENT_COUNT; i++)
    {
        if (queuedEvents[i][0] == id)
            return 1;
    }
    for (i = 0; i < QUEUED_EVENT_COUNT; i++)
    {
        if (queuedEvents[i][0] == 0)
        {
            queuedEvents[i][0] = id;
            queuedEvents[i][1] = running;
            return 1;
        }
    }
    return 0;
}
