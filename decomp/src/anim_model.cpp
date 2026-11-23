// Animated model key tracks. An AnimModel (the CharModel inside a Character) has three key tracks:
// track A = motion frame keys (which frame of the motion file plays when), track B = a second timeline, track C =
// position/colour keys with flags. Each track remembers its first and last key frame and two playback options.
// Scripts build tracks key by key, then start playback (char_motion.cpp shows a typical sequence).

#include <string.h>

#include "key_track.h"

#include "anim_model.h"

#include "node.h"


// Clears the model table DAT_10faaa88 (all 0x100 entries unused).
extern "C" void FUN_10161980(void)
{
    memset(DAT_10faaa88, 0, sizeof(DAT_10faaa88));
}

// Applies material swap `count` to both motion nodes (Node::M_1019e830 with key -1 = all subsets). Returns 1.
int AnimModel::M_10161a20(int count)
{
    if (count > 0)
    {
        if (swapNode1 != 0)
            swapNode1->ReplaceMaterial(-1, count);
        if (swapNode2 != 0)
            swapNode2->ReplaceMaterial(-1, count);
    }
    return 1;
}

// Clears track C (no keys; first frame 10000 so the first key always becomes the start).
void AnimModel::M_10161a60()
{
    trackC.keys.Clear();
    trackC.last = 0;
    trackC.first = 10000;
    trackC.frame = 0;
    trackC.playing = 0;
}

// Adds key (a, b, c) at `frame` to track C and widens its frame range.
int AnimModel::M_10161af0(int frame, float x, float y, float z)
{
    trackC.keys.Set(frame, x, y, z, 0);
    if (trackC.last < frame + 1)
        trackC.last = frame + 1;
    if (trackC.first > frame)
        trackC.first = frame;
    return 0;
}

// Sets track C's two playback options (loop mode and speed).
int AnimModel::M_10161b40(int a, int b)
{
    trackC.playing = a;
    trackC.looping = b;
    return 0;
}

// Clears track A.
void AnimModel::M_10161b60()
{
    trackA.keys.Clear();
    trackA.last = 0;
    trackA.first = 10000;
    trackA.frame = 0;
    trackA.playing = 0;
}

// Adds a motion key at `frame` to track A (key flag 4 = motion frame) and widens its frame range.
int AnimModel::M_10161b90(int frame, float value)
{
    trackA.keys.flags[trackA.keys.Set(frame, value, 0, 0, 0)] = KEY_FLAG_MOTION;
    if (trackA.last < frame)
        trackA.last = frame;
    if (trackA.first > frame)
        trackA.first = frame;
    return 0;
}

// Sets track A's two playback options (char_motion.cpp: 1 and "loop").
int AnimModel::M_10161be0(int a, int b)
{
    trackA.playing = a;
    trackA.looping = b;
    return 0;
}

// Clears track B.
void AnimModel::M_10161f30()
{
    trackB.keys.Clear();
    trackB.last = 0;
    trackB.first = 10000;
    trackB.frame = 0;
    trackB.playing = 0;
}

// Sets track B's two playback options.
int AnimModel::M_10161f60(int a, int b)
{
    trackB.playing = a;
    trackB.looping = b;
    return 0;
}

// 0x101628b0. Queues a push (x, y, z) with `flag` unless the model is locked. Returns 0.
int AnimModel::SetPendingPush(float x, float y, float z, char flag)
{
    if (locked == 0)
    {
        m_2288 = flag;
        m_228c.x = x;
        m_228c.y = y;
        m_228c.z = z;
    }
    return 0;
}
// Adds key (a, b, c) at `frame` to track C with key flag `flag`.
int AnimModel::M_10161a90(int frame, float x, float y, float z, char flag)
{
    trackC.keys.flags[trackC.keys.Set(frame, x, y, z, 0)] = flag;
    if (trackC.last < frame + 1)
        trackC.last = frame + 1;
    if (trackC.first > frame)
        trackC.first = frame;
    return 0;
}
// Advances track A by one frame while it plays: samples the motion frame for the current track frame, shows it
// (also on the playback object) and moves on. At the end the track loops back to its first frame, or stops on the
// last one. Returns 0.
int AnimModel::StepTrackA()
{
    if (trackA.playing != 0)
    {
        float frame, y, z, w;
        trackA.keys.Sample(trackA.frame, &frame, &y, &z, &w, 0);
        motionFrame = (int)frame;
        if (motion != 0)
            motion->frame = motionFrame;
        if (++trackA.frame >= trackA.last)
        {
            if (trackA.looping != 0)
            {
                trackA.frame = trackA.first;
                return 0;
            }
            trackA.playing = 0;
            trackA.frame--;
        }
    }
    return 0;
}
