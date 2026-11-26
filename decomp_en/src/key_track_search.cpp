// Key frame searches of KeyTrack (EN build, EN-only: no JP twin found).
#include "containers.h"

// Returns the frame number of the first key later than `frame`; when every key is at or before `frame` it returns the
// last key's frame, and 0 for an empty track. Walks the keys from the end. No direct caller.
int KeyTrack::NextFrame(int frame)
{
    if (count == 0)
        return 0;
    int result = frames[count - 1];
    for (int i = count - 1; i >= 0; i--)
    {
        if (frames[i] <= frame)
            break;
        result = frames[i];
    }
    return result;
}

// Returns the frame number of the last key earlier than `frame`; when every key is at or after `frame` it returns the
// first key's frame, and 0 for an empty track. Walks the keys from the start. No direct caller.
int KeyTrack::PreviousFrame(int frame)
{
    if (count == 0)
        return 0;
    int result = frames[0];
    for (int i = 0; i < count; i++)
    {
        if (frames[i] >= frame)
            break;
        result = frames[i];
    }
    return result;
}
