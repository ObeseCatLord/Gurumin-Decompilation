// Switches and levers held by characters or objects. Each of the 128 hold slots records who holds it
// (DAT_10255b08): a character slot (< 0x80) or 0x80 + an object index; DAT_1030bbf8 marks the slot as used.

#include "world.h"

// Map object (0x10874848) that animates the switch nodes ("LEVERON" and its off state).
struct MapObject
{
    int M_10054f30(int node, int on);
};

extern "C" MapObject DAT_10874848;
extern "C" int DAT_1030bbf8[0x80];       // 1 while the hold slot is used
extern "C" int DAT_10255b08[0x80];       // holder of each slot
extern "C" short DAT_10254838[0x80];     // per-object switch state; 1000 is added while it is held

// Releases the hold slot of `holder`: a character's hold count drops by one and its switch node is turned off; an
// object loses its "held" bonus of 1000. Returns 1, or 0 when `holder` holds nothing.
extern "C" int FUN_10057b90(int holder)
{
    for (int i = 0; i < 0x80; i++)
    {
        if (DAT_1030bbf8[i] != 0 && DAT_10255b08[i] == holder)
        {
            int who = DAT_10255b08[i];
            if (who < 0x80)
            {
                DAT_10daab84[who].m_00a4--;
                DAT_10874848.M_10054f30(DAT_10db6ed8[who].m_0280, 0);
            }
            else if (DAT_10254838[who % 0x80] >= 1000)
                DAT_10254838[who % 0x80] -= 1000;
            DAT_1030bbf8[i] = 0;
            return 1;
        }
    }
    return 0;
}
