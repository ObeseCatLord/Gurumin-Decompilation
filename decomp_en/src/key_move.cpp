// Moving and changing the keys of the key containers (EN build, EN-only: no JP twin found).
#include "containers.h"
#include "key_bank.h"

// A frame or value argument at or below KEY_UNCHANGED_LIMIT means "keep the key's own"; the callers pass KEY_UNCHANGED.
const float KEY_UNCHANGED = -999999.0f;
const float KEY_UNCHANGED_LIMIT = -999998.0f;
const int KEY_FRAME_UNCHANGED_LIMIT = -999998;

// KeyTrack::MoveFrame: takes the key at frame `frame` out and puts it back at frame `newFrame` with the values x, y, z, w;
// an argument that is KEY_UNCHANGED (frame or value) keeps the key's own. The key's flags are not kept. Returns 1, or 0
// when there is no key at `frame`. Called by the key editing code.
int KeyTrack::MoveFrame(int frame, int newFrame, float x, float y, float z, float w)
{
    int index = 0;
    int total = count;
    for (; index < total; index++)
    {
        if (frames[index] != frame)
            continue;
        if (x <= KEY_UNCHANGED_LIMIT)
            x = values[index].x;
        if (y <= KEY_UNCHANGED_LIMIT)
            y = values[index].y;
        if (z <= KEY_UNCHANGED_LIMIT)
            z = values[index].z;
        if (w <= KEY_UNCHANGED_LIMIT)
            w = values[index].w;
        int targetFrame = newFrame;
        if (targetFrame <= KEY_FRAME_UNCHANGED_LIMIT)
            targetFrame = frames[index];
        RemoveFrame(frame);
        Insert(targetFrame, x, y, z, w);
        return 1;
    }
    return 0;
}

// KeyBank::MoveFrame: the KeyBank twin of KeyTrack::MoveFrame; takes the key at frame `frame` out and puts it back at
// `newFrame` with the values x, y, z, w, where KEY_UNCHANGED keeps the key's own. Returns 1, or 0 when there is no key
// at `frame`. Called by the key editing code.
int KeyBank::MoveFrame(int frame, int newFrame, float x, float y, float z, float w)
{
    int index = 0;
    int total = count;
    for (; index < total; index++)
    {
        if (frames[index] != frame)
            continue;
        if (x == KEY_UNCHANGED)
            x = values[index].x;
        if (y == KEY_UNCHANGED)
            y = values[index].y;
        if (z == KEY_UNCHANGED)
            z = values[index].z;
        if (w == KEY_UNCHANGED)
            w = values[index].w;
        RemoveFrame(frame);
        Insert(newFrame, x, y, z, w);
        return 1;
    }
    return 0;
}

// KeyBank::MoveKeyTo: gives key `index` the frame `frame` (KEY_FRAME_UNCHANGED_LIMIT or lower keeps its own) and the
// values x, y, z, w (at KEY_UNCHANGED_LIMIT or lower keeps its own). The frame stays strictly between the neighbouring
// keys, the key keeps its flags. Returns 1. Called by the key editing code.
int KeyBank::MoveKeyTo(int index, int frame, float x, float y, float z, float w)
{
    if (x <= KEY_UNCHANGED_LIMIT)
        x = values[index].x;
    if (y <= KEY_UNCHANGED_LIMIT)
        y = values[index].y;
    if (z <= KEY_UNCHANGED_LIMIT)
        z = values[index].z;
    if (w <= KEY_UNCHANGED_LIMIT)
        w = values[index].w;
    int newFrame = frame;
    if (newFrame <= KEY_FRAME_UNCHANGED_LIMIT)
        newFrame = frames[index];
    if (index > 0 && newFrame <= frames[index - 1])
        newFrame = frames[index - 1] + 1;
    if (index < count - 1 && newFrame >= frames[index + 1])
        newFrame = frames[index + 1] - 1;
    char keptFlags = flags[index];
    RemoveAt(index);
    int newIndex = Insert(newFrame, x, y, z, w);
    flags[newIndex] = keptFlags;
    return 1;
}

// KeyTrack::MoveKeyTo: the KeyTrack twin of KeyBank::MoveKeyTo. Returns 1.
int KeyTrack::MoveKeyTo(int index, int frame, float x, float y, float z, float w)
{
    if (x <= KEY_UNCHANGED_LIMIT)
        x = values[index].x;
    if (y <= KEY_UNCHANGED_LIMIT)
        y = values[index].y;
    if (z <= KEY_UNCHANGED_LIMIT)
        z = values[index].z;
    if (w <= KEY_UNCHANGED_LIMIT)
        w = values[index].w;
    if (frame <= KEY_FRAME_UNCHANGED_LIMIT)
        frame = frames[index];
    if (index > 0 && frame <= frames[index - 1])
        frame = frames[index - 1] + 1;
    if (index < count - 1 && frame >= frames[index + 1])
        frame = frames[index + 1] - 1;
    char keptFlags = flags[index];
    RemoveAt(index);
    int newIndex = Insert(frame, x, y, z, w);
    flags[newIndex] = keptFlags;
    return 1;
}

// KeyBank::ShiftKey: like KeyBank::MoveKeyTo, but the new frame is the key's own frame plus `offset`; a result at
// KEY_FRAME_UNCHANGED_LIMIT or lower keeps the key's own frame. Returns 1. Called by the key editing code.
int KeyBank::ShiftKey(int index, int offset, float x, float y, float z, float w)
{
    int ownFrame = frames[index];
    int newFrame = ownFrame + offset;
    if (x <= KEY_UNCHANGED_LIMIT)
        x = values[index].x;
    if (y <= KEY_UNCHANGED_LIMIT)
        y = values[index].y;
    if (z <= KEY_UNCHANGED_LIMIT)
        z = values[index].z;
    if (w <= KEY_UNCHANGED_LIMIT)
        w = values[index].w;
    if (newFrame <= KEY_FRAME_UNCHANGED_LIMIT)
        newFrame = ownFrame;
    if (index > 0 && newFrame <= frames[index - 1])
        newFrame = frames[index - 1] + 1;
    if (index < count - 1 && newFrame >= frames[index + 1])
        newFrame = frames[index + 1] - 1;
    char keptFlags = flags[index];
    RemoveAt(index);
    int newIndex = Insert(newFrame, x, y, z, w);
    flags[newIndex] = keptFlags;
    return 1;
}
