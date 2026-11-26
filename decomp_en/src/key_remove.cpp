// Key removal and frame range of the key containers (EN build, EN-only: no JP twin found).
#include "containers.h"
#include "key_bank.h"

// Takes key `index` out of the bank: the keys after it move up one place and the count drops by one. Returns 0. Called
// by the key-moving code at EN 0x776f10 and EN 0x777120.
int KeyBank::RemoveAt(int index)
{
    count--;
    for (int i = index; i < count; i++)
    {
        frames[i] = frames[i + 1];
        Vec4 *key = &values[i];
        const Vec4 *following = &values[i + 1];
        key->x = following->x;
        key->y = following->y;
        key->z = following->z;
        key->w = following->w;
        flags[i] = flags[i + 1];
    }
    return 0;
}

// Removes the key that sits at frame `frame`, moving the later keys up one place. Returns 1 when there was such a key,
// else 0. Called by the key-moving code at EN 0x776ce0.
int KeyTrack::RemoveFrame(int frame)
{
    for (int i = 0; i < count; i++)
    {
        if (frames[i] == frame)
        {
            count--;
            for (int next = i; next < count; next++)
            {
                frames[next] = frames[next + 1];
                Vec4 *key = &values[next];
                const Vec4 *following = &values[next + 1];
                key->x = following->x;
                key->y = following->y;
                key->z = following->z;
                key->w = following->w;
                flags[next] = flags[next + 1];
            }
            return 1;
        }
    }
    return 0;
}

// The highest frame number among the keys (0 when the track is empty or all frames are negative). The compiler
// vectorises the scan with SSE4.1 when the CPU has it. Called by ApplyCommandText at EN 0x76b780.
int KeyTrack::MaxFrame()
{
    int highest = 0;
    for (int i = 0; i < count; i++)
    {
        if (frames[i] > highest)
            highest = frames[i];
    }
    return highest;
}
