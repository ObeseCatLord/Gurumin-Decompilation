// Enemy group and formation tables (reset when a map is loaded).
#include <string.h>

// Three groups of twelve values; slot 9 of each group is the group leader (-1 = none).
extern "C" int DAT_10255960[3][12];

// Six 24-byte formation records; only the first field (the active flag) is reset here.
struct Formation
{
    int active;
    char pad_0004[0x14];
};

extern "C" Formation DAT_1049efdc[6];
extern "C" int DAT_1042dea8[0x5c];  // per-character group numbers (characters 0..0x5b)
extern "C" int DAT_102199b8[0x5c];  // per-character group timers

// Clears all group, formation and per-character group data.
extern "C" void FUN_10034fe0(void)
{
    int group;
    for (group = 0; group < 3; group++)
    {
        for (int i = 0; i < 11; i++)
        {
            DAT_10255960[group][i] = 0;
            if (i == 9)
                DAT_10255960[group][9] = -1;
        }
    }
    for (int f = 0; f < 6; f++)
        DAT_1049efdc[f].active = 0;
    memset(DAT_1042dea8, 0, sizeof(DAT_1042dea8));
    memset(DAT_102199b8, 0, sizeof(DAT_102199b8));
}
