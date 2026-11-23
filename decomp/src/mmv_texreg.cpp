// MMV movie player (docs/MODDING_MAP.md, tools/mmv_tool.py) and the texture name registry.
// Texture slots: DAT_10ffe5f8[1600][0x40] holds each slot's file name (empty = free).

#include <stdio.h>
#include <string.h>

#include "mmv_player.h"


#include "engine_memory.h"
#include "render_vars.h"

// Not byte-identical yet: the original keeps 0 in ebx for its compares and stores (see open.tsv).
// Like ReadFrame for a movie that plays once: after the last frame the in-memory copy is freed or the file closed,
// then the movie either holds its last frame (state MMV_HOLD_LAST_FRAME) or stops and frees the frame buffer when it
// owns it (m_004a).
void MmvPlayer::ReadFrameOnce()
{
    if (m_004b == 0)
        return;
    if (m_0048 == 6 && frameIndex >= 7)
        return;
    if (isOpen == 0)
    {
        if (state == MMV_HOLD_LAST_FRAME)
            return;
        int length = *(int *)(start + memory + offset);
        data = (unsigned char *)(start + memory + offset + 4);
        DecodeFrameRle(length);
        offset += length + 4;
        frameIndex++;
        if (start + offset < size)
            return;
        DAT_11b1ba40(memory);
        memory = 0;
    }
    else
    {
        if (state == MMV_HOLD_LAST_FRAME)
            return;
        int length;
        file.Read(&length, 1, 4);
        if (length <= 0x25800 && length > 0)
        {
            file.Read(DAT_10f68138, 1, length);
            data = DAT_10f68138;
            DecodeFrameRle(length);
            offset += length + 4;
            frameIndex++;
        }
        else
            offset += size;
        if (offset < size)
            return;
        file.Close();
    }
    if (state != MMV_STOPPED)
    {
        state = MMV_HOLD_LAST_FRAME;
        return;
    }
    m_004b = 0;
    if (frame != 0 && m_004a != 0)
        DAT_11b1ba40(frame);
    frame = 0;
}

// Reads the next frame record (a 4-byte length and RLE data) from memory or from the file, decodes it into the frame
// and advances; after the last frame playback starts again from the first one (the file is rewound to 0x406, just
// after the header). Movies of type 6 stop after 7 frames. Does nothing while the player is not playing (m_004b).
void MmvPlayer::ReadFrame()
{
    if (m_004b != 0 && (m_0048 != 6 || frameIndex < 7))
    {
        if (isOpen == 0)
        {
            int length = *(int *)(start + memory + offset);
            data = (unsigned char *)(start + memory + offset + 4);
            DecodeFrameRle(length);
            offset += length + 4;
            frameIndex++;
            if (start + offset >= size)
            {
                offset = 0;
                frameIndex = 0;
            }
        }
        else
        {
            int length;
            file.Read(&length, 1, 4);
            if (length <= 0x25800 && length > 0)
            {
                file.Read(DAT_10f68138, 1, length);
                data = DAT_10f68138;
                DecodeFrameRle(length);
                offset += length + 4;
                frameIndex++;
            }
            else
                offset += size;
            if (offset >= size)
            {
                offset = 0;
                frameIndex = 0;
                file.Seek(0x406, 0);
            }
        }
    }
}

#include "texture_object.h"

extern "C" void __stdcall FUN_1014f120(int, int, int width, int height, int pitch, void *pixels, void *palette);
extern "C" void FUN_10143b50(void);
extern "C" void FUN_10143b80(void);

// Constructor: nothing open yet.
MmvPlayer::MmvPlayer()
{
    loop = 0;
    memory = 0;
    m_004b = 0;
    frame = 0;
    m_004a = 0;
    state = MMV_STOPPED;
}

// Destructor: closes the movie.
MmvPlayer::~MmvPlayer()
{
    if (isOpen && m_003c != 0)
        file.Close();
    isOpen = 0;
    m_004b = 0;
    if (memory != 0)
        DAT_11b1ba40(memory);
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
        if (memory != 0)
            DAT_11b1ba40(memory);
        if (frame != 0 && m_004a)
            DAT_11b1ba40(frame);
        memory = 0;
        frame = 0;
        m_004b = 0;
    }
}

// Copies the current frame (clipped to the size of `target`) into the movie texture using the frame's palette.
// Not byte-identical yet (see open.tsv).
int MmvPlayer::DrawToTexture(const TextureObject *target)
{
    if (frame != 0)
    {
        int h = height > target->height ? target->height : height;
        int w = width > target->width ? target->width : width;
        FUN_1014f120(0, 0, w, h, width, frame, palette);
    }
    return 1;
}

// Decodes the next frame (looping or one-shot variant depending on loop).
void MmvPlayer::M_10161050()
{
    if (loop != 0)
        ReadFrame();
    else
        ReadFrameOnce();
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
        DAT_118d0ba8[index].BindStage0();
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
        DAT_118d0ba8[index].BindStage1();
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

extern "C" int FUN_10148b00(void);               // Render_CreateDevice

// 0x10161970. Creates the Direct3D device and empties the texture name tables; returns the device result. The
// compiler keeps the result in edx across the call because it sees that FUN_101610f0 above leaves edx alone.
extern "C" int FUN_10161970(void)
{
    int result = FUN_10148b00();
    FUN_101610f0();
    return result;
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

#include "game_file.h"
extern "C" int FUN_10161120(char *out, const char *path);       // Path_GetBaseName

// 0x10160a90 (MmvPlayer::Open). Loads movie `path` (looked for as .in\<base name>.MMV first) whole into memory:
// the size, the 255-colour palette (as RGB565, colour 255 set to 0xfbff), a frame buffer of width x height and the
// first frame decoded. When the frame buffer cannot be allocated the movie is dropped again.
void MmvPlayer::Open(const char *path)
{
    char base[0x100];
    char name[0x100];
    loop = 0;
    state = MMV_STOPPED;
    isOpen = 0;
    GameFile movie;
    FUN_10161120(base, path);
    sprintf(name, ".\\bin\\%s.MMV", base);
    void *opened = movie.Open(name, "rb");
    if (opened == 0)
    {
        opened = movie.Open(path, "rb");
        if (opened == 0)
            return;
    }
    movie.Seek(0, 2);
    int length = movie.Size();
    movie.Seek(0, 0);
    if (memory != 0)
        DAT_11b1ba40(memory);
    memory = (char *)DAT_11b0b938(length, "MMV_ALL");
    if (memory == 0)
        movie.Close();
    else
    {
        movie.Read(memory, 1, length);
        movie.Close();
        width = *(short *)(memory + 2);
        height = *(short *)(memory + 4);
        int i;
        for (i = 0; i < 0xff; i++)
            palette[i] = (((unsigned short)(unsigned char)memory[i * 4 + 8] & ~7) << 5 |
                          ((unsigned short)(unsigned char)memory[i * 4 + 7] & ~3)) << 3 |
                         (unsigned short)((unsigned char)memory[i * 4 + 6] >> 3);
        palette[i] = 0xfbff;
        if (frame != 0 && m_004a != 0)
            DAT_11b1ba40(frame);
        frame = DAT_11b0b938(width * height, "MMV_DIB");
        if (frame == 0)
        {
            if (memory != 0)
                DAT_11b1ba40(memory);
            memory = 0;
        }
        else
        {
            m_004a = 1;
            int first = *(int *)(memory + 0x406);
            data = (unsigned char *)memory + 0x40a;
            DecodeFrameRle(first);
            start = first + 0x40a;
            offset = 0;
            size = length;
            frameIndex = 0;
            m_004b = 1;
        }
    }
}
