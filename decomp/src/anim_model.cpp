// Animated model key tracks. An AnimModel (the CharModel inside a Character) has three key tracks:
// track A = motion frame keys (which frame of the motion file plays when), track B = a second timeline, track C =
// position/colour keys with flags. Each track remembers its first and last key frame and two playback options.
// Scripts build tracks key by key, then start playback (char_motion.cpp shows a typical sequence).

#include <string.h>

struct KeyList
{
    char keys[0xa00];
    char flags[0x84];
    void Clear();
    int Add(int frame, int a, int b, int c, int d);
};

struct AnimTrack
{
    KeyList keys;
    int last;
    int first;
    int m_008;
    int m_00c;
    int m_010;
};

struct Motion
{
    void M_1019e830(int from, int count);
};

struct AnimModel
{
    char pad_0000[0x2ac];
    AnimTrack trackA;
    char pad_0d44[0xc];
    AnimTrack trackB;
    char pad_17e8;
    char m_17e9;
    char pad_17ea[2];
    AnimTrack trackC;
    char pad_2284[4];
    char m_2288;
    char pad_2289[3];
    int m_228c;
    int m_2290;
    int m_2294;
    char pad_2298[0x68];
    Motion *motion1;
    Motion *motion2;

    int M_10161a20(int count);
    void M_10161a60();
    int M_10161af0(int frame, int a, int b, int c);
    int M_10161b40(int a, int b);
    void M_10161b60();
    int M_10161b90(int frame, int value);
    int M_10161be0(int a, int b);
    void M_10161f30();
    int M_10161f60(int a, int b);
    int M_101628b0(int a, int b, int c, char d);
    int M_10161a90(int frame, int a, int b, int c, char flag);
};

extern "C" int DAT_10faaa88[0x100];

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
        if (motion1 != 0)
            motion1->M_1019e830(-1, count);
        if (motion2 != 0)
            motion2->M_1019e830(-1, count);
    }
    return 1;
}

// Clears track C (no keys; first frame 10000 so the first key always becomes the start).
void AnimModel::M_10161a60()
{
    trackC.keys.Clear();
    trackC.last = 0;
    trackC.first = 10000;
    trackC.m_008 = 0;
    trackC.m_00c = 0;
}

// Adds key (a, b, c) at `frame` to track C and widens its frame range.
int AnimModel::M_10161af0(int frame, int a, int b, int c)
{
    trackC.keys.Add(frame, a, b, c, 0);
    if (trackC.last < frame + 1)
        trackC.last = frame + 1;
    if (trackC.first > frame)
        trackC.first = frame;
    return 0;
}

// Sets track C's two playback options (loop mode and speed).
int AnimModel::M_10161b40(int a, int b)
{
    trackC.m_00c = a;
    trackC.m_010 = b;
    return 0;
}

// Clears track A.
void AnimModel::M_10161b60()
{
    trackA.keys.Clear();
    trackA.last = 0;
    trackA.first = 10000;
    trackA.m_008 = 0;
    trackA.m_00c = 0;
}

// Adds a motion key at `frame` to track A (key flag 4 = motion frame) and widens its frame range.
int AnimModel::M_10161b90(int frame, int value)
{
    trackA.keys.flags[trackA.keys.Add(frame, value, 0, 0, 0)] = 4;
    if (trackA.last < frame)
        trackA.last = frame;
    if (trackA.first > frame)
        trackA.first = frame;
    return 0;
}

// Sets track A's two playback options (char_motion.cpp: 1 and "loop").
int AnimModel::M_10161be0(int a, int b)
{
    trackA.m_00c = a;
    trackA.m_010 = b;
    return 0;
}

// Clears track B.
void AnimModel::M_10161f30()
{
    trackB.keys.Clear();
    trackB.last = 0;
    trackB.first = 10000;
    trackB.m_008 = 0;
    trackB.m_00c = 0;
}

// Sets track B's two playback options.
int AnimModel::M_10161f60(int a, int b)
{
    trackB.m_00c = a;
    trackB.m_010 = b;
    return 0;
}

// Stores a pending action (a, b, c, flag d) unless the model is locked (m_17e9).
int AnimModel::M_101628b0(int a, int b, int c, char d)
{
    if (m_17e9 == 0)
    {
        m_2288 = d;
        m_228c = a;
        m_2290 = b;
        m_2294 = c;
    }
    return 0;
}
// Adds key (a, b, c) at `frame` to track C with key flag `flag`.
int AnimModel::M_10161a90(int frame, int a, int b, int c, char flag)
{
    trackC.keys.flags[trackC.keys.Add(frame, a, b, c, 0)] = flag;
    if (trackC.last < frame + 1)
        trackC.last = frame + 1;
    if (trackC.first > frame)
        trackC.first = frame;
    return 0;
}