// Freeing the value storage of a ListBank (EN build). JP twin: resource_banks.cpp.
#include <stdlib.h>

#include "listbank.h"

// Frees the buffers of one group (the per-group version of FreeAll). Buffers marked as borrowed are kept untouched.
// Called by ListBank when a nesting level ends. Returns 0.
int ListBank::FreeBank(int group)
{
    variableCounts[group] = 0;
    for (int i = 0; i < LISTBANK_GROUP_ENTRIES; i++)
    {
        if (borrowed[group][i] == 0)
        {
            if (values[group][i].storage != 0)
                free(values[group][i].storage);
            values[group][i].storage = 0;
        }
    }
    return 0;
}

// Frees every buffer in every group, except buffers marked as borrowed (owned elsewhere), which are only forgotten.
// Clears the counts and the nesting level. Called when a script bank is unloaded. Returns 0.
int ListBank::FreeAll()
{
    depth = 0;
    for (int group = 0; group < LISTBANK_GROUP_COUNT; group++)
    {
        variableCounts[group] = 0;
        for (int i = 0; i < LISTBANK_GROUP_ENTRIES; i++)
        {
            if (borrowed[group][i] == 0)
            {
                if (values[group][i].storage != 0)
                    free(values[group][i].storage);
                values[group][i].storage = 0;
            }
            else
            {
                values[group][i].storage = 0;
                borrowed[group][i] = 0;
            }
        }
    }
    return 0;
}
