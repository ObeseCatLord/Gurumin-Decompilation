// Searching a key bank (EN build, EN-only: no JP twin found).
#include "key_bank.h"

// Returns the index of the key that sits at `frame`, or -1 when no key does. Scans the keys in order. No direct caller.
int KeyBank::FindFrame(int frame)
{
    for (int i = 0; i < count; i++)
    {
        if (frames[i] == frame)
            return i;
    }
    return -1;
}
