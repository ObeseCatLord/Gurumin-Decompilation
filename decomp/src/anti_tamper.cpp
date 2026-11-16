#include <stdlib.h>

struct PlayerRecord
{
    int m_0000;
    int hp;
    int maxHp;
    char pad_000c[0x10];
    int m_001c;
    int m_0020;
    char pad_0024[0x94];
    int money;
};

extern "C" PlayerRecord DAT_10daaae8;
extern "C" int DAT_10c635f8;
extern "C" int DAT_10c64ad4;
extern "C" int DAT_10c64ad8;
extern "C" int DAT_10c655f0;
extern "C" int DAT_10c6670c;
extern "C" int DAT_104684b0[115];

#define GUARD_VALUE(index, value) (~(value) + (index) * 0x41)

extern "C" int FUN_1000e750(void)
{
    DAT_104684b0[0] = GUARD_VALUE(0, DAT_10daaae8.m_001c);
    DAT_104684b0[1] = GUARD_VALUE(1, DAT_10daaae8.m_0020);
    DAT_104684b0[2] = GUARD_VALUE(2, DAT_10daaae8.hp);
    DAT_104684b0[3] = GUARD_VALUE(3, DAT_10daaae8.maxHp);
    DAT_104684b0[4] = GUARD_VALUE(4, DAT_10daaae8.money);
    DAT_104684b0[5] = GUARD_VALUE(5, DAT_10c6670c);
    DAT_104684b0[6] = GUARD_VALUE(6, DAT_10c64ad4);
    DAT_104684b0[7] = GUARD_VALUE(7, DAT_10c64ad8);
    DAT_104684b0[8] = GUARD_VALUE(8, DAT_10c635f8);
    DAT_104684b0[9] = GUARD_VALUE(9, DAT_10c655f0);
    return 0;
}

extern "C" char DAT_103150c0[0x50];
extern "C" int DAT_10c66144[15];

extern "C" int FUN_1000e810(void)
{
    int i;
    for (i = 0; i < 0x50; i++)
        DAT_104684b0[10 + i] = ~DAT_103150c0[i] + 0x14 + i * 0x41;
    for (i = 0; i < 15; i++)
        DAT_104684b0[100 + i] = ~DAT_10c66144[i] + 6500 + i * 2;
    return 0;
}

extern "C" int DAT_104c0270[16];
extern "C" int DAT_104c0344;
extern "C" short DAT_104c034c;
extern "C" int DAT_11b044f0;
extern "C" unsigned int DAT_104bf890;

inline void TAMPER()
{
    int noise = rand();
    DAT_104c0270[rand() % 16] = noise;
}

extern "C" int FUN_1000e870(void)
{
    DAT_104c034c++;
    if (DAT_11b044f0 != 0)
        TAMPER();
    if (DAT_104684b0[0] != GUARD_VALUE(0, DAT_10daaae8.m_001c))
        TAMPER();
    if (DAT_104684b0[1] != GUARD_VALUE(1, DAT_10daaae8.m_0020))
        TAMPER();
    if (DAT_104684b0[2] != GUARD_VALUE(2, DAT_10daaae8.hp))
    {
        TAMPER();
        DAT_104c0344++;
    }
    if (DAT_104684b0[3] != GUARD_VALUE(3, DAT_10daaae8.maxHp))
    {
        TAMPER();
        DAT_104c0344++;
    }
    if (DAT_104684b0[4] != GUARD_VALUE(4, DAT_10daaae8.money))
        TAMPER();
    if (DAT_104684b0[5] != GUARD_VALUE(5, DAT_10c6670c))
        TAMPER();
    if (DAT_104684b0[6] != GUARD_VALUE(6, DAT_10c64ad4))
        TAMPER();
    if (DAT_104684b0[7] != GUARD_VALUE(7, DAT_10c64ad8))
        TAMPER();
    if (DAT_104684b0[8] != GUARD_VALUE(8, DAT_10c635f8))
        TAMPER();
    if (DAT_104684b0[9] != GUARD_VALUE(9, DAT_10c655f0))
        TAMPER();
    if (DAT_104bf890 % 30 == 0)
    {
        for (int i = 0; i < 0x50; i++)
        {
            if (DAT_104684b0[10 + i] != ~DAT_103150c0[i] + 0x14 + i * 0x41)
                TAMPER();
        }
    }
    if (DAT_104bf890 % 30 == 14)
    {
        for (int i = 0; i < 15; i++)
        {
            if (DAT_104684b0[100 + i] != ~DAT_10c66144[i] + 6500 + i * 2)
                TAMPER();
        }
    }
    DAT_104c034c--;
    return 0;
}
