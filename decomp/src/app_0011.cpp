#include <stdlib.h>
#include "world.h"

extern "C" int DAT_10255f10[];
extern "C" int FUN_1013f880(void);
extern "C" void FUN_10043cc0(int);

extern "C" int FUN_10017b80(void)
{
    for (int i = 0; i < 0x40; i++)
    {
        if (DAT_10860df0[i].m_00ec == 0 && DAT_10860df0[i].active == 0)
            return i;
    }
    return -1;
}

extern "C" int FUN_10017bb0(int id)
{
    for (int i = 0; i < 0x40; i++)
    {
        if (DAT_10860df0[i].active != 0 && DAT_10860df0[i].type == 10 && DAT_10860df0[i].id == id)
            return DAT_10860df0[i].value;
    }
    return -1;
}

extern "C" int FUN_1001b840(int index)
{
    if (DAT_10db6ed8[index].m_239c != 0)
    {
        if (FUN_1013f880() % 10000 < 100)
            DAT_10db6ed8[index].model.M_101630b0(30.0f);
    }
    return 0;
}

extern "C" int FUN_10034c80(int index)
{
    if (DAT_10210d30[index] != 0)
        return 1;
    DAT_1020b430[index].m_003c = 1;
    DAT_10daab84[index].m_00a5 = 1;
    return 0;
}

extern "C" int FUN_10047400(int index)
{
    int value = DAT_1020b430[index].m_0004[1];
    if (value == 100)
    {
        DAT_10255f10[index] = -1;
        FUN_10043cc0(index);
        return 0;
    }
    DAT_10255f10[index] = value;
    FUN_10043cc0(index);
    return 0;
}
