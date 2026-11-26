// Finding a switch by its door id (EN build, EN-only: no JP twin found).
#include "switches.h"

// Returns the first slot from SWITCH_FIRST_DOOR_SLOT on whose door id equals `doorId`, or -1 when none does.
extern "C" int Switch_FindByDoorId(int doorId)
{
    for (int slot = SWITCH_FIRST_DOOR_SLOT; slot < CHAR_SLOT_COUNT; slot++)
    {
        if (g_switchDoorId[slot] == doorId)
            return slot;
    }
    return -1;
}
