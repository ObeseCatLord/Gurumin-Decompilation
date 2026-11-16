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
};

extern "C" int DAT_10faaa88[0x100];

extern "C" void FUN_10161980(void)
{
    memset(DAT_10faaa88, 0, sizeof(DAT_10faaa88));
}

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

void AnimModel::M_10161a60()
{
    trackC.keys.Clear();
    trackC.last = 0;
    trackC.first = 10000;
    trackC.m_008 = 0;
    trackC.m_00c = 0;
}

int AnimModel::M_10161af0(int frame, int a, int b, int c)
{
    trackC.keys.Add(frame, a, b, c, 0);
    if (trackC.last < frame + 1)
        trackC.last = frame + 1;
    if (trackC.first > frame)
        trackC.first = frame;
    return 0;
}

int AnimModel::M_10161b40(int a, int b)
{
    trackC.m_00c = a;
    trackC.m_010 = b;
    return 0;
}

void AnimModel::M_10161b60()
{
    trackA.keys.Clear();
    trackA.last = 0;
    trackA.first = 10000;
    trackA.m_008 = 0;
    trackA.m_00c = 0;
}

int AnimModel::M_10161b90(int frame, int value)
{
    trackA.keys.flags[trackA.keys.Add(frame, value, 0, 0, 0)] = 4;
    if (trackA.last < frame)
        trackA.last = frame;
    if (trackA.first > frame)
        trackA.first = frame;
    return 0;
}

int AnimModel::M_10161be0(int a, int b)
{
    trackA.m_00c = a;
    trackA.m_010 = b;
    return 0;
}

void AnimModel::M_10161f30()
{
    trackB.keys.Clear();
    trackB.last = 0;
    trackB.first = 10000;
    trackB.m_008 = 0;
    trackB.m_00c = 0;
}

int AnimModel::M_10161f60(int a, int b)
{
    trackB.m_00c = a;
    trackB.m_010 = b;
    return 0;
}

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