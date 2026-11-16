#include "d3d9_min.h"

struct SlotTable
{
    int m_0000;
    int pad_0004[2];
    int m_000c;
    int *m_0010;
    int *m_0014;
    int m_0018;
    int m_001c;
    int m_0020;
    int m_0024;
    int m_0028;
    int m_002c;
    int m_0030;
    int m_0034;
    int m_0038;
    int m_003c;
    int m_0040;
    int m_0044;
    int m_0048;

    void M_10145a10();
    int M_10146d90(int first, int last);
    int M_10147690(int key, int value);
    int M_101476e0(int index, int value);
};

struct IntPair
{
    int first;
    int second;
    IntPair();
};

extern "C" IDirect3DDevice9 *DAT_10f1fcf0;
extern "C" int DAT_10f6550c;
extern "C" int DAT_10f65510;
extern "C" int DAT_10f181e0[0x5a0];

extern "C" void FUN_10143b50(void)
{
    if (DAT_10f6550c != 1)
    {
        DAT_10f1fcf0->SetTexture(0, 0);
        DAT_10f6550c = 1;
    }
}

extern "C" void FUN_10143b80(void)
{
    if (DAT_10f65510 != 1)
    {
        DAT_10f1fcf0->SetTexture(1, 0);
        DAT_10f65510 = 1;
    }
}

IntPair::IntPair()
{
    first = 0;
    second = 0;
}

extern "C" int FUN_101446b0(void)
{
    FUN_10143b50();
    FUN_10143b80();
    DAT_10f1fcf0->SetTexture(2, 0);
    DAT_10f1fcf0->SetTexture(3, 0);
    return 0;
}

void SlotTable::M_10145a10()
{
    m_0000 = -1;
    m_0044 = 0;
    m_000c = 0;
    m_0014 = 0;
    m_0010 = 0;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
    m_0024 = 0;
    m_0028 = 0;
    m_002c = 0;
    m_0030 = 0;
    m_0034 = 0;
    m_0038 = 0;
    m_0048 = 0;
}

int SlotTable::M_10146d90(int first, int last)
{
    m_003c = first;
    m_0040 = last;
    return 1;
}

int SlotTable::M_10147690(int key, int value)
{
    for (int i = m_003c; i < m_0044 && i < m_0040; i++)
    {
        if (key == -1 || m_0014[i] == key || m_0010[i] == key)
            m_0014[i] = value;
    }
    return 0;
}

int SlotTable::M_101476e0(int index, int value)
{
    if (index < m_0044 && index < m_0040)
        m_0014[index] = value;
    return 0;
}

extern "C" int FUN_10147a00(void)
{
    for (int i = 0; i < 0x5a0; i++)
        DAT_10f181e0[i]++;
    return 1;
}
