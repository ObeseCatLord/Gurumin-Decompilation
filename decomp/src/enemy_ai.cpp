#include "world.h"

extern "C" int FUN_100e1c40(int index);
extern "C" int FUN_10026af0(int index);
extern "C" int FUN_10026b70(int index);
extern "C" int FUN_10026c50(int index);
extern "C" int FUN_10026c80(int index);
extern "C" int FUN_10026d80(int index, float range);
extern "C" int FUN_10027400(int index);
extern "C" int FUN_10027560(int index);
extern "C" int FUN_10027630(int index, int flag);
extern "C" int FUN_100279b0(int index, int a, int b);
extern "C" int FUN_10027da0(int index, int a);
extern "C" int FUN_10028360(int index, int a);

extern "C" int FUN_100e2080(int index)
{
    if (FUN_100e1c40(index) != 0)
        return 0;
    int done;
    switch (DAT_1020b430[index].m_0000)
    {
    case 0:
        done = FUN_10026d80(index, 300.0f);
        break;
    case 1:
        done = FUN_10027560(index);
        break;
    case 2:
        done = FUN_10028360(index, 2);
        break;
    default:
        if (++DAT_1020b430[index].m_0004[2] > 0)
        {
            DAT_1020b430[index].m_0004[2] = 0;
            FUN_10026af0(index);
        }
        return 0;
    }
    if (done != 0)
    {
        DAT_1020b430[index].m_0004[2] = 0;
        DAT_1020b430[index].m_0000 = -1;
    }
    return 0;
}

extern "C" int FUN_100e2110(int index)
{
    if (FUN_100e1c40(index) != 0)
        return 0;
    int done;
    switch (DAT_1020b430[index].m_0000)
    {
    case 0:
        done = FUN_10028360(index, 2);
        break;
    case 1:
        done = FUN_100279b0(index, 4, 0);
        break;
    case 2:
        done = FUN_10027630(index, 0);
        break;
    default:
        if (++DAT_1020b430[index].m_0004[2] > 0)
        {
            DAT_1020b430[index].m_0004[2] = 0;
            FUN_10026b70(index);
        }
        return 0;
    }
    if (done != 0)
    {
        DAT_1020b430[index].m_0004[2] = 0;
        DAT_1020b430[index].m_0000 = -1;
    }
    return 0;
}

extern "C" int FUN_100e2440(int index)
{
    if (FUN_100e1c40(index) != 0)
        return 0;
    int done;
    switch (DAT_1020b430[index].m_0000)
    {
    case 0:
        done = FUN_10027400(index);
        break;
    case 1:
        done = FUN_10027560(index);
        break;
    case 2:
        done = FUN_10028360(index, 2);
        break;
    default:
        if (++DAT_1020b430[index].m_0004[2] > 0)
        {
            DAT_1020b430[index].m_0004[2] = 0;
            FUN_10026c50(index);
        }
        return 0;
    }
    if (done != 0)
    {
        DAT_1020b430[index].m_0004[2] = 0;
        DAT_1020b430[index].m_0000 = -1;
    }
    return 0;
}

extern "C" int FUN_100e24d0(int index)
{
    if (FUN_100e1c40(index) != 0)
        return 0;
    int done;
    switch (DAT_1020b430[index].m_0000)
    {
    case 0:
        done = FUN_10028360(index, 2);
        break;
    case 1:
        done = FUN_10027da0(index, 2);
        break;
    case 2:
        done = FUN_10027560(index);
        break;
    default:
        if (++DAT_1020b430[index].m_0004[2] > 0)
        {
            DAT_1020b430[index].m_0004[2] = 0;
            FUN_10026c80(index);
        }
        return 0;
    }
    if (done != 0)
    {
        DAT_1020b430[index].m_0004[2] = 0;
        DAT_1020b430[index].m_0000 = -1;
    }
    return 0;
}
