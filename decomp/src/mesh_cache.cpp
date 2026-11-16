struct IntPair
{
    int first;
    int second;
    IntPair();
};

struct MeshInstance : IntPair
{
    char pad_0008[0x18];
    int m_0020;
    int m_0024;
    int pad_0028;
    int m_002c;
    int m_0030;
    int m_0034;

    MeshInstance();
    int M_10169df0();
    int M_10169e10();
    int M_10169f70(int value);
};

struct MeshRef
{
    char pad_0000[0x4c];
    int index;
};

struct TextureRef
{
    int index;
};

struct LayerTable
{
    char pad_0000[0x2c];
    int m_002c[16];
    int m_006c[16];
    char pad_00ac[0xfc4];
    int m_1070[0x90][16];
    int m_3470[0x90][16];
    int m_5870[0x90][16];

    int M_1016a220(int a, int slot, int value);
    int M_1016a260(int layer, int slot, int x, int y);
    int M_1016a2a0(int layer, int slot, int value);
};

extern "C" int DAT_10feb3e0[0xc80];
extern "C" char DAT_10fada90[0xc80][0x20];
extern "C" int DAT_10faae88[0x400];
extern "C" char DAT_10ff65f8[0x400][0x20];
extern "C" int DAT_1101d884;
extern "C" int DAT_1101d88c;
extern "C" int FUN_1014f520(void);
extern "C" int __stdcall FUN_1014f1f0(int, int, int, int);
extern "C" int __stdcall FUN_10143870(int, int, int);

extern "C" int FUN_10169990(const MeshRef *mesh)
{
    int index = mesh->index;
    if (index < 0)
        return 0;
    if (--DAT_10feb3e0[index] <= 0)
    {
        DAT_10fada90[index][0] = 0;
        DAT_10feb3e0[index] = 0;
    }
    return DAT_10feb3e0[index];
}

extern "C" int FUN_101699d0(void)
{
    for (int i = 0; i < 0x400; i++)
    {
        if (DAT_10ff65f8[i][0] == 0)
            return 0;
    }
    return -1;
}

extern "C" int FUN_10169be0(const TextureRef *texture)
{
    int index = texture->index;
    return ++DAT_10faae88[index];
}

extern "C" int FUN_10169c00(const TextureRef *texture)
{
    int index = texture->index;
    if (index < 0)
        return 0;
    if (--DAT_10faae88[index] <= 0)
    {
        DAT_10ff65f8[index][0] = 0;
        DAT_10faae88[index] = 0;
    }
    return DAT_10faae88[index];
}

MeshInstance::MeshInstance()
{
    m_0024 = 0;
    m_0020 = 0;
}

int MeshInstance::M_10169df0()
{
    if (m_0034 != 0)
        return FUN_1014f520();
    return 1;
}

int MeshInstance::M_10169e10()
{
    if (m_0034 != 0)
        return FUN_1014f1f0(m_002c, m_0030, m_0024, m_0020);
    return 1;
}

int MeshInstance::M_10169f70(int value)
{
    return FUN_10143870(m_002c, m_0030, value);
}

int LayerTable::M_1016a220(int a, int slot, int value)
{
    if (value > 0x8f)
        return 0;
    if (slot > 15)
        return 0;
    if (a == 0)
        return 0;
    m_006c[slot] = value;
    m_002c[slot] = a;
    return 1;
}

int LayerTable::M_1016a260(int layer, int slot, int x, int y)
{
    if (layer > 0x8f)
        return 0;
    if (slot > 15)
        return 0;
    m_1070[layer][slot] = x;
    m_3470[layer][slot] = y;
    return 1;
}

int LayerTable::M_1016a2a0(int layer, int slot, int value)
{
    if (layer > 0x8f)
        return 0;
    if (slot > 7)
        return 0;
    m_5870[layer][slot] = value;
    return 1;
}

extern "C" int FUN_1016a8a0(int value)
{
    DAT_1101d884 = value;
    DAT_1101d88c = 0;
    return 0;
}
