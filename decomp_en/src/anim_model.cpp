// Animated model channels and queued push (EN build). Ported from the matched JP source decomp/src/anim_model.cpp.
#include "anim_model.h"

// Sets track C's two playback options (playing flag and loop flag).
int AnimModel::SetTrackCOptions(int playing, int looping)
{
    trackC.playing = playing;
    trackC.looping = looping;
    return 0;
}

// Sets the motion channel's two playback options.
int AnimModel::SetMotionOptions(int playing, int looping)
{
    motion.playing = playing;
    motion.looping = looping;
    return 0;
}

// Sets track B's two playback options.
int AnimModel::SetTrackBOptions(int playing, int looping)
{
    trackB.playing = playing;
    trackB.looping = looping;
    return 0;
}

// Adds key (x, y, z) at `frame` to track C and widens its frame range.
int AnimModel::AddTrackCKey(int frame, float x, float y, float z)
{
    trackC.keys.Set(frame, x, y, z, 0);
    if (trackC.last < frame + 1)
        trackC.last = frame + 1;
    if (trackC.first > frame)
        trackC.first = frame;
    return 0;
}

// Adds a motion key at `frame` to the motion channel (flagged as a motion frame) and widens its frame range.
int AnimModel::AddMotionKey(int frame, float value)
{
    motion.keys.flags[motion.keys.Set(frame, value, 0, 0, 0)] = KEY_FLAG_MOTION;
    if (motion.last < frame)
        motion.last = frame;
    if (motion.first > frame)
        motion.first = frame;
    return 0;
}

// Queues a push (x, y, z) with `flag` unless the model is locked.
int AnimModel::SetPendingPush(float x, float y, float z, char flag)
{
    if (locked == 0)
    {
        pushFlag = flag;
        pushVector.x = x;
        pushVector.y = y;
        pushVector.z = z;
    }
    return 0;
}

// Clears track C (no keys, first frame after any key so the first key becomes the start).
int AnimModel::ClearTrackC()
{
    trackC.keys.Clear();
    trackC.last = 0;
    trackC.first = CHANNEL_EMPTY_FIRST_FRAME;
    trackC.frame = 0;
    trackC.playing = 0;
    return 0;
}

// Clears the motion channel.
int AnimModel::ClearMotionKeys()
{
    motion.keys.Clear();
    motion.last = 0;
    motion.first = CHANNEL_EMPTY_FIRST_FRAME;
    motion.frame = 0;
    motion.playing = 0;
    return 0;
}

// Clears track B.
int AnimModel::ClearTrackB()
{
    trackB.keys.Clear();
    trackB.last = 0;
    trackB.first = CHANNEL_EMPTY_FIRST_FRAME;
    trackB.frame = 0;
    trackB.playing = 0;
    return 0;
}
