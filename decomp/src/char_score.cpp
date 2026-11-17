#include "world.h"

extern "C" char DAT_1030ce38[0x80];
extern "C" int DAT_1030b668[0x80];
extern "C" int DAT_104679a0[0x80];
extern "C" int DAT_10314ec0[0x80];
extern "C" int DAT_1021f440[0x80];
extern "C" int DAT_10daaaf8[];
extern "C" int DAT_10daaafc[];

extern "C" int FUN_1001b5b0(int index)
{
    int value = DAT_10daab84[index].m_003c;
    DAT_1020b430[index].m_0004[1] = value / 100000;
    int rest = value / 10 % 10000;
    DAT_1020b430[index].m_003c = rest;
    if (rest != 0)
    {
        DAT_1030ce38[index] = 1;
        if (DAT_1030b668[index] == 0)
        {
            DAT_1030b668[index] = DAT_10daab84[index].m_0000;
            if (DAT_1030b668[index] == 0)
                DAT_1030b668[index] = -1;
            DAT_104679a0[index] = DAT_10daab84[index].m_00a7;
            DAT_10314ec0[index] = DAT_10daaaf8[index * 0x62];
            DAT_1021f440[index] = DAT_10daaafc[index * 0x62];
        }
        DAT_10daab84[index].m_0000 = 0x62;
    }
    return 0;
}
