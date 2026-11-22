// Delayed sound effects: up to 10 sounds wait for their delay to run out before they are played with
// FUN_1003ad20 (Sound_PlayEffect: id, optional position, volume).

#include "vec4.h"

extern "C" int DAT_10447310[10];   // frames left before the sound plays (0 = free slot)
extern "C" int DAT_1021a398[10];   // sound id
extern "C" int DAT_1042d870[10];   // 1 when the sound has a position
extern "C" Vec4 DAT_10f08a00[10];  // position of the sound
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);

// Queues `sound` to play after `delay` frames, at `position` (0 = not positional). When all slots are in use, the
// sound that would play soonest is played now and its slot is reused (its position flag stays as it was).
// Returns 0. Not byte-identical yet: the original keeps &DAT_1021a398[soonest] in edi (see open.tsv).
extern "C" int FUN_10054550(int sound, int delay, const Vec4 *position)
{
    int soonest = 0;
    int shortest = 999999;
    for (int i = 0; i < 10; i++)
    {
        if (DAT_10447310[i] == 0)
        {
            DAT_10447310[i] = delay;
            DAT_1021a398[i] = sound;
            if (position != 0)
            {
                DAT_1042d870[i] = 1;
                DAT_10f08a00[i].Copy(*position);
                return 0;
            }
            DAT_1042d870[i] = 0;
            return 0;
        }
        if (shortest > DAT_10447310[i])
        {
            shortest = DAT_10447310[i];
            soonest = i;
        }
    }
    int *slotSound = &DAT_1021a398[soonest];
    if (DAT_1042d870[soonest] != 0)
        FUN_1003ad20(*slotSound, &DAT_10f08a00[soonest], 1.0f);
    else
        FUN_1003ad20(*slotSound, 0, 1.0f);
    *slotSound = sound;
    DAT_10447310[soonest] = delay;
    return 0;
}
