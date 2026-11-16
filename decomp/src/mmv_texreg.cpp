#include <string.h>

struct ClipRect
{
    int left;
    int top;
    int right;
    int bottom;
};

struct MmvDecoder
{
    char data[0x28];
    MmvDecoder();
};

struct MmvPlayer
{
    char pad_0000[0x10];
    int m_0010;
    char pad_0014[4];
    MmvDecoder decoder;
    short width;
    short height;
    char pad_0044[6];
    char m_004a;
    char m_004b;
    char m_004c;
    char pad_004d;
    char m_004e[0x202];
    int m_0250;
    void *frame;

    MmvPlayer();
    int M_10160960(const ClipRect *clip);
    int M_10160df0();
    int M_10160f00();
    int M_10161050();
};

struct TextureSlot
{
    char data[0x38];
    void M_1014b670();
    void M_1014b6e0();
};

extern "C" void __stdcall FUN_1014f120(int, int, int width, int height, int pitch, void *pixels, void *palette);
extern "C" void FUN_10143b50(void);
extern "C" void FUN_10143b80(void);
extern "C" char DAT_10ffe5f8[1600][0x40];
extern "C" char DAT_10f8dbb8[1600][0x40];
extern "C" char DAT_10fa6db8[1600];
extern "C" TextureSlot DAT_118d0ba8[1600];

MmvPlayer::MmvPlayer()
{
    m_0010 = 0;
    m_0250 = 0;
    m_004b = 0;
    frame = 0;
    m_004a = 0;
    m_004c = 0;
}

int MmvPlayer::M_10160960(const ClipRect *clip)
{
    if (frame != 0)
    {
        int h = height > clip->bottom ? clip->bottom : height;
        int w = width > clip->right ? clip->right : width;
        FUN_1014f120(0, 0, w, h, width, frame, m_004e);
    }
    return 1;
}

int MmvPlayer::M_10161050()
{
    if (m_0010 != 0)
        return M_10160df0();
    return M_10160f00();
}

extern "C" int FUN_10161070(int index)
{
    if (index < 0)
    {
        FUN_10143b50();
        return -1;
    }
    if (DAT_10ffe5f8[index][0] != 0)
        DAT_118d0ba8[index].M_1014b670();
    return index;
}

extern "C" int FUN_101610b0(int index)
{
    if (index < 0)
    {
        FUN_10143b80();
        return -1;
    }
    if (DAT_10ffe5f8[index][0] != 0)
        DAT_118d0ba8[index].M_1014b6e0();
    return index;
}

extern "C" int FUN_101610f0(void)
{
    memset(DAT_10ffe5f8, 0, sizeof(DAT_10ffe5f8));
    memset(DAT_10f8dbb8, 0, sizeof(DAT_10f8dbb8));
    return 1;
}

extern "C" int FUN_101618f0(void)
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0)
            DAT_10fa6db8[i] = 1;
    }
    return 0;
}

extern "C" int FUN_10161920(const char *name)
{
    if (*name == 0)
        return -1;
    for (int i = 0; i < 1600; i++)
    {
        if (_stricmp(DAT_10ffe5f8[i], name) == 0)
            return i;
    }
    return -1;
}
