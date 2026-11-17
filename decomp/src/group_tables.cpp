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

struct Matrix4
{
    float m[16];  // row-major; m[12], m[13], m[14] = translation
};

extern "C" int DAT_102108d0[6];       // owner of each attached matrix (0 = free)
extern "C" Matrix4 DAT_10f08880[6];   // attached matrices

// Stores a copy of `matrix` moved by (x, y, z) in the first free attachment slot and records `owner` for it.
// Returns the slot, or 0 when all six are in use.
extern "C" int FUN_10034f70(const Matrix4 *matrix, float x, float y, float z, int owner)
{
    for (int i = 0; i < 6; i++)
    {
        if (DAT_102108d0[i] == 0)
        {
            DAT_102108d0[i] = owner;
            DAT_10f08880[i] = *matrix;
            DAT_10f08880[i].m[12] += x;
            DAT_10f08880[i].m[13] += y;
            DAT_10f08880[i].m[14] += z;
            return i;
        }
    }
    return 0;
}

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
