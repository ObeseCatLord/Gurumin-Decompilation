#include "world.h"

struct ActorSlot
{
    int m_0000;
    char pad_0004[0x98];
    int m_009c;
    char pad_00a0[0xe8];
};

extern "C" ActorSlot DAT_10daaaec[0x80];
extern "C" char DAT_1021f3c0[0x80];

extern "C" int FUN_10011c60(void)
{
    for (int i = 0x3a; i < 0x7c; i++)
    {
        if (DAT_1021f3c0[i] == 3 && i >= 0x3a && DAT_10db6ed8[i].m_2368.x > -49999.0f && DAT_10daaaec[i].m_0000 > 0 &&
            DAT_10210d30[i] == 0 && DAT_10db6ed8[i].m_22fc != 0 && DAT_10daaaec[i].m_009c == 0)
            DAT_10daaaec[i].m_0000 = 0;
    }
    return 1;
}

extern "C" int FUN_10011cd0(void)
{
    for (int i = 0x3a; i < 0x7c; i++)
    {
        if (DAT_1021f3c0[i] == 3 && i >= 0x3a)
        {
            if ((DAT_10db6ed8[i].m_2368.x < -50000.0f || (DAT_10daaaec[i].m_0000 <= 0 && DAT_10210d30[i] == 2)) &&
                DAT_10db6ed8[i].m_22fc != 0)
                return i;
        }
    }
    return -1;
}
