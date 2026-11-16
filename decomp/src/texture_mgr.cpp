struct TextureObject
{
    char pad_0000[0x34];
    void *texture;
    void M_10169df0();
    void M_10169e10();
    void M_10169e40();
};

struct TextureManager
{
    int M_10173480();
    int M_101734d0();
    int M_10173510();
    void M_1015c730();
    void M_10175e70();
};

struct ZeroVec
{
    float x;
    float y;
    float z;
    float w;
    ZeroVec();
};

struct Matrix4
{
    float m[16];
    Matrix4();
};

struct Transform
{
    int m_0000;
    ZeroVec position;
    ZeroVec rotation;
    ZeroVec scale;
    Matrix4 matrix;
    Transform();
};

struct Selection
{
    int m_0000;
    int m_0004;
    int m_0008;
    int pad_000c;
    int m_0010;
    int current;
    int M_10172a80(int a, int b);
    int M_10172aa0();
    void M_101733f0();
    int M_10173450(int id, int unused);
};

struct Scene
{
    char pad_0000[0x1c];
    int m_001c;
    void M_10169f90();
    void M_1016a020(void *owner);
};

struct SceneOwner
{
    char pad_0000[0x298];
    Scene *scene;
    int M_1017b1c0();
};

struct Animator
{
    void M_101687e0();
    int M_101692c0();
    int M_10184160();
};

struct PackHeader
{
    char data[0x74];
    void M_101acfc0();
};

extern "C" char DAT_10ffe5f8[1600][0x40];
extern "C" char DAT_10f8dbb8[1600][0x40];
extern "C" char DAT_10fa6db8[1600];
extern "C" TextureObject DAT_118d0ba8[1600];
extern "C" void *(*DAT_11b0b938)(int size, const char *tag);
extern "C" void FUN_10174c70(int index, int flag);

Transform::Transform()
{
}

int Selection::M_10172a80(int a, int b)
{
    m_0000 = a;
    m_0004 = b;
    return 1;
}

int Selection::M_10172aa0()
{
    m_0008 = 0;
    m_0010 = 0;
    current = -1;
    return 1;
}

int Selection::M_10173450(int id, int unused)
{
    if (current != -1)
    {
        if (current != id)
            M_101733f0();
        current = id;
        return 0;
    }
    current = id;
    return 0;
}

int TextureManager::M_10173480()
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0)
        {
            DAT_118d0ba8[i].M_10169e40();
            DAT_10ffe5f8[i][0] = 0;
            DAT_10f8dbb8[i][0] = 0;
        }
    }
    return 1;
}

int TextureManager::M_101734d0()
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0)
            DAT_118d0ba8[i].M_10169df0();
    }
    return 1;
}

int TextureManager::M_10173510()
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0)
            DAT_118d0ba8[i].M_10169e10();
    }
    return 1;
}

void TextureManager::M_10175e70()
{
    M_10173480();
    M_1015c730();
}

int SceneOwner::M_1017b1c0()
{
    if (scene == 0)
    {
        scene = (Scene *)DAT_11b0b938(0x7c84, "");
        scene->M_10169f90();
        scene->M_1016a020(this);
        return scene->m_001c;
    }
    return scene->m_001c;
}

int Animator::M_10184160()
{
    M_101687e0();
    return M_101692c0();
}

extern "C" int FUN_10188fb0(void)
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0 && DAT_10fa6db8[i] == 0 && DAT_118d0ba8[i].texture == 0)
            FUN_10174c70(i, 1);
    }
    return 0;
}

extern "C" void __stdcall FUN_101911b0(PackHeader *packs, int count)
{
    if (packs != 0)
    {
        for (; count > 0; count--)
        {
            packs->M_101acfc0();
            packs++;
        }
    }
}
