// Script resource banks: 8 banks of up to 0x80 allocated buffers.

#include <stdlib.h>

#include "listbank.h"
// Frees every buffer in every bank, except buffers marked as borrowed (owned elsewhere), which are only
// forgotten. Clears the counts.
int ListBank::FreeAll()
{
    depth = 0;
    for (int bank = 0; bank < 8; bank++)
    {
        variableCounts[bank] = 0;
        for (int i = 0; i < 0x80; i++)
        {
            if (borrowed[bank][i] == 0)
            {
                if (values[bank][i].storage != 0)
                    free(values[bank][i].storage);
                values[bank][i].storage = 0;
            }
            else
            {
                values[bank][i].storage = 0;
                borrowed[bank][i] = 0;
            }
        }
    }
    return 0;
}

// Frees the buffers of one bank (the per-bank version of FreeAll). Borrowed buffers are kept untouched here.
int ListBank::FreeBank(int bank)
{
    variableCounts[bank] = 0;
    for (int i = 0; i < 0x80; i++)
    {
        if (borrowed[bank][i] == 0)
        {
            if (values[bank][i].storage != 0)
                free(values[bank][i].storage);
            values[bank][i].storage = 0;
        }
    }
    return 0;
}
