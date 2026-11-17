// MMV movie player (docs/MODDING_MAP.md, tools/mmv_tool.py) and the texture name registry.
// Texture slots: DAT_10ffe5f8[1600][0x40] holds each slot's file name (empty = free).

#include <string.h>

struct ClipRect
{
    int left;
    int top;
    int right;
    int bottom;
};

struct MovieFile
{
    void *handle;
    char name[0x20];
    MovieFile();
    ~MovieFile();
    void Close();
};

struct MmvPlayer
{
    char pad_0000[0x10];
    int m_0010;
    char isOpen;
    char pad_0015[3];
    MovieFile file;
    void *m_003c;
    short width;
    short height;
    char pad_0044[6];
    char m_004a;
    char m_004b;
    char m_004c;
    char pad_004d;
    char m_004e[0x202];
    int m_0250;
    void *frame;           // 0x254: decoded 8-bit frame (palette indexes), updated in place
    unsigned char *data;   // 0x258: compressed data of the current frame

    MmvPlayer();
    ~MmvPlayer();
    void Close();
    int M_10160960(const ClipRect *clip);
    int M_10160df0();
    int M_10160f00();
    int M_10161050();
    void DecodeFrameRle(int count);
};

struct TextureSlot
{
    char data[0x38];
    void M_1014b670();
    void M_1014b6e0();
};

extern "C" void (*DAT_11b1ba40)(void *);
extern "C" void __stdcall FUN_1014f120(int, int, int width, int height, int pitch, void *pixels, void *palette);
extern "C" void FUN_10143b50(void);
extern "C" void FUN_10143b80(void);
extern "C" char DAT_10ffe5f8[1600][0x40];
extern "C" char DAT_10f8dbb8[1600][0x40];
extern "C" char DAT_10fa6db8[1600];
extern "C" TextureSlot DAT_118d0ba8[1600];

// Constructor: nothing open yet.
MmvPlayer::MmvPlayer()
{
    m_0010 = 0;
    m_0250 = 0;
    m_004b = 0;
    frame = 0;
    m_004a = 0;
    m_004c = 0;
}

// Destructor: closes the movie.
MmvPlayer::~MmvPlayer()
{
    if (isOpen && m_003c != 0)
        file.Close();
    isOpen = 0;
    m_004b = 0;
    if (m_0250 != 0)
        DAT_11b1ba40((void *)m_0250);
    if (frame != 0 && m_004a)
        DAT_11b1ba40(frame);
}

// Closes the movie file and frees its buffers.
void MmvPlayer::Close()
{
    if (m_004b)
    {
        if (isOpen && m_003c != 0)
            file.Close();
        isOpen = 0;
        if (m_0250 != 0)
            DAT_11b1ba40((void *)m_0250);
        if (frame != 0 && m_004a)
            DAT_11b1ba40(frame);
        m_0250 = 0;
        frame = 0;
        m_004b = 0;
    }
}

// Copies the current frame (clipped to `clip`) into the movie texture using the frame's palette.
// Not byte-identical yet (see open.tsv).
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

// Decodes the next frame (looping or one-shot variant depending on m_0010).
int MmvPlayer::M_10161050()
{
    if (m_0010 != 0)
        return M_10160df0();
    return M_10160f00();
}

// Binds texture slot `index` to stage 0 (a negative index unbinds). Returns the index or -1.
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

// Binds texture slot `index` to stage 1 (a negative index unbinds). Returns the index or -1.
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

// Clears both texture name tables (all slots free). Returns 1.
extern "C" int FUN_101610f0(void)
{
    memset(DAT_10ffe5f8, 0, sizeof(DAT_10ffe5f8));
    memset(DAT_10f8dbb8, 0, sizeof(DAT_10f8dbb8));
    return 1;
}

// Marks every loaded texture slot as kept (DAT_10fa6db8) before a map change.
extern "C" int FUN_101618f0(void)
{
    for (int i = 0; i < 1600; i++)
    {
        if (DAT_10ffe5f8[i][0] != 0)
            DAT_10fa6db8[i] = 1;
    }
    return 0;
}

// Texture_FindByName: returns the slot holding file `name` (case-insensitive), or -1.
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

// Mmv_DecodeFrameRle: applies one frame's RLE data (`count` bytes at `data`) to the previous frame in `frame`.
// Codes (docs/MODDING_MAP.md, tools/mmv_tool.py):
//   0xFC n     skip n pixels (they keep the previous frame's colour)
//   0xFD n v   n pixels of palette index v
//   other b    one pixel of palette index b
// Written in inline assembly in the original (frame pointer, `or cl,cl` tests), so it is here too.
void MmvPlayer::DecodeFrameRle(int count)
{
    MmvPlayer *self = this;
    unsigned char *source = data;
    unsigned char *dest = (unsigned char *)frame;
    __asm
    {
        mov edi, source
        mov esi, dest
        mov edx, count
        xor ecx, ecx
        xor ebx, ebx
    next_code:
        mov cl, [edi]
        cmp cl, 0xfc
        je skip_run
        cmp cl, 0xfd
        jne single_pixel
        sub edx, 3
        mov cl, [edi + 1]
        mov bl, [edi + 2]
        add edi, 2
    fill_loop:
        mov [esi], bl
        dec cl
        inc esi
        or cl, cl
        jne fill_loop
        inc edi
        jmp done_code
    skip_run:
        mov cl, [edi + 1]
        add edi, 2
        sub edx, 2
        add esi, ecx
        jmp done_code
    single_pixel:
        mov [esi], cl
        dec edx
        inc edi
        inc esi
    done_code:
        or edx, edx
        jne next_code
    }
}
