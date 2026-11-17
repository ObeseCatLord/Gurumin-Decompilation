// Script resource banks: 8 banks of up to 0x80 allocated buffers.

#include <stdlib.h>

struct ResourceBanks
{
    char pad_0000[0x440c];
    void *items[8][0x100];
    char pad_640c[0x8400];
    char borrowed[8][0x80];
    char pad_ec0c[0x1b9d0];
    int counts[8];
    char pad_2a5fc[0xbd28];
    int m_36324;
    int FreeAll();
};

// Frees every buffer in every bank, except buffers marked as borrowed (owned elsewhere), which are only
// forgotten. Clears the counts.
int ResourceBanks::FreeAll()
{
    m_36324 = 0;
    for (int bank = 0; bank < 8; bank++)
    {
        counts[bank] = 0;
        for (int i = 0; i < 0x80; i++)
        {
            if (borrowed[bank][i] == 0)
            {
                if (items[bank][i] != 0)
                    free(items[bank][i]);
                items[bank][i] = 0;
            }
            else
            {
                items[bank][i] = 0;
                borrowed[bank][i] = 0;
            }
        }
    }
    return 0;
}
