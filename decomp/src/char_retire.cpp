#include "world.h"

struct Marker
{
    float y;
    char pad_0004[0xc];
};

extern "C" char DAT_1020d540[0x80];
extern "C" int DAT_10c635a8;
extern "C" char DAT_1049efb0[10][10];
extern "C" Marker DAT_10867e18[0x80];

extern "C" int FUN_1001b7b0(int index)
{
    if (DAT_1020b430[index].m_0000 == 0)
    {
        DAT_10db6ed8[index].m_2368.x = -100000.0f - DAT_10db6ed8[index].m_2368.x * 1.0f;
        int value = DAT_1020b430[index].m_0004[1];
        DAT_1020d540[index] = 0;
        DAT_1020b430[index].m_0004[3] = value % 10;
        DAT_1020b430[index].m_0000 = 1;
        DAT_10c635a8 = 1;
        DAT_1049efb0[value / 10 % 10][value % 10] = (char)index;
        DAT_10867e18[index].y = -200000.0f;
    }
    return 0;
}
