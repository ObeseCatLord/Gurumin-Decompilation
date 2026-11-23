// Enemy group and formation tables (reset when a map is loaded).
#include <string.h>

// Three groups of twelve values; slot 9 of each group is the group leader (-1 = none).
#include "map_vars.h"

#include "formation.h"

extern "C" Formation DAT_1049efdc[6];
extern "C" int DAT_102199b8[0x5c];  // per-character group timers

#include "matrix4.h"


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

// Per-character attachment matrices (6 per character slot) for kind-0x58 characters.

#include "char_slot.h"

#include "char_state.h"


// Finds the first free attachment slot among the kind-0x58 characters 0x5c..0x7b, stores `matrix` there with the
// four values a..d in its fourth column, records `value` for the slot and returns the character and slot index in
// *outChar and *outSlot. Returns 1, or 0 when no slot is free.
extern "C" int FUN_1001c410(int value, const Matrix4 *matrix, int *outChar, int *outSlot, float a, float b, float c,
                            float d)
{
    for (int i = 0x5c; i < 0x7c; i++)
    {
        if (DAT_10daaae8[i].kind == 0x58)
        {
            for (int j = 1; j < 7; j++)
            {
                if (DAT_1020b430[i].values[j - 1] == 0)
                {
                    DAT_108687f8[i * 6 + j] = *matrix;
                    DAT_108687f8[i * 6 + j].m[3] = a;
                    DAT_108687f8[i * 6 + j].m[7] = b;
                    DAT_108687f8[i * 6 + j].m[11] = c;
                    DAT_108687f8[i * 6 + j].m[15] = d;
                    DAT_1020b430[i].values[j - 1] = value;
                    *outChar = i;
                    *outSlot = j - 1;
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Moves `current` 55 % of the way towards `target`, halving `steps` each time (as an integer), and repeats until
// one step or less is left; the result goes to *out. With `first` set, fewer than 2 steps store `current`
// unchanged. Returns 0.
extern "C" int FUN_10034810(float target, float current, float *out, float steps, int first)
{
    if (first != 0 && steps < 2.0f)
    {
        *out = current;
        return 0;
    }
    if (target > current)
    {
        current = (target - current) * 0.55f + current;
        steps = steps * 0.5f;
        steps = (float)(int)steps;
        if (steps <= 1.0f)
        {
            *out = current;
            return 0;
        }
    }
    else
    {
        current = (current - target) * 0.55f + target;
        steps = steps * 0.5f;
        steps = (float)(int)steps;
        if (steps <= 1.0f)
        {
            *out = current;
            return 0;
        }
    }
    FUN_10034810(target, current, out, steps, 0);
    return 0;
}
