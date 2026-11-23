// HD movies for Gurumin (JP PC, bin\APP.dll): .mmv movies made at 2x or 4x their original size play at the
// original on-screen size with the extra detail.
//
// What the game does (see docs/MOVIES_HD.md):
//   - MmvPlayer::Open 0x10160a90 loads a whole .mmv and takes its size from the header (player +0x40/+0x42).
//   - MmvPlayer::M_10160960 copies the decoded 8-bit frame into the texture slot "videotex" (created 1024x512),
//     clipped to the slot's width/height (+0x08/+0x0c), through the palette.
//   - The drawing code takes the movie size from player +0x40/+0x42 for the on-screen size and divides it by 1024 and
//     512 for the texture coordinates.
//
// An HD movie carries "HD" and its scale in the unused fourth bytes of palette entries 0, 1 and 2 (the game ignores
// them). For such a movie this file
//   - reports the original size (header size / scale) at +0x40/+0x42, so every screen keeps its layout;
//   - puts a texture `scale` times larger (2048x1024 for 2x) into the videotex slot while the movie plays, so the
//     game's texture coordinates (original size / 1024) cover exactly the HD frame;
//   - uploads the frame at the full HD size.
// Normal movies and the DirectShow video (3ddata\op.avi) get the original texture back before they are uploaded.
#include <windows.h>
#include <d3d9.h>
#include <string.h>
#include "display.h"

#define VA_MMV_OPEN 0x10160a90u
#define VA_MMV_UPLOAD 0x10160960u
#define VA_MMV_CLOSE 0x101609b0u
#define VA_DSHOW_UPLOAD 0x1015c6a0u

#define VIDEOTEX_WIDTH 1024
#define VIDEOTEX_HEIGHT 512

// The parts of MmvPlayer (decomp/src/mmv_texreg.cpp) used here.
#define PLAYER_WIDTH(p) (*(short *)((BYTE *)(p) + 0x40))
#define PLAYER_HEIGHT(p) (*(short *)((BYTE *)(p) + 0x42))
#define PLAYER_MEMORY(p) (*(BYTE **)((BYTE *)(p) + 0x250))

// The parts of a texture slot (an entry of the 0x38-byte table at 0x118d0ba8) used here.
typedef struct
{
    IDirect3DTexture9 *texture;
    int unknown_04;
    int width;
    int height;
} TextureSlot;

typedef void(__thiscall *open_fn)(void *player, const char *path);
typedef int(__thiscall *upload_fn)(void *player, TextureSlot *slot);
typedef void(__thiscall *close_fn)(void *player);

static open_fn real_open;
static upload_fn real_upload;
static close_fn real_close;
static void *real_dshow_upload;

static BYTE *app_base;
static void (*log_line)(const char *format, ...);

// One HD movie at a time (the game has one movie texture).
static struct
{
    void *player;
    int width, height, scale;
} hd_movie;

// The videotex slot while an HD texture is in it.
static struct
{
    TextureSlot *slot;
    IDirect3DTexture9 *original, *hd;
    int width, height;
} swapped;

static IDirect3DTexture9 *hd_textures[5];   // by scale (2 and 4), kept for the rest of the game

static BYTE *app_address(DWORD va)
{
    return app_base + (va - 0x10000000u);
}

// `expect` lists the original bytes at `va` (0x100 = any byte: relocated addresses).
static int code_matches(DWORD va, const short *expect, int length)
{
    const BYTE *target = app_address(va);
    for (int i = 0; i < length; i++)
        if (expect[i] < 0x100 && target[i] != expect[i])
        {
            log_line("movies: unexpected code at %08lx, HD movies off", va);
            return 0;
        }
    return 1;
}

// Jumps from the first `length` bytes of `va` to `hook`; returns a trampoline that runs those bytes and goes on.
static void *detour(DWORD va, int length, void *hook)
{
    BYTE *target = app_address(va);
    BYTE *trampoline = VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!trampoline)
        return NULL;
    memcpy(trampoline, target, length);
    trampoline[length] = 0xe9;
    *(INT32 *)(trampoline + length + 1) = (INT32)(target + length - (trampoline + length + 5));
    DWORD old;
    VirtualProtect(target, length, PAGE_EXECUTE_READWRITE, &old);
    target[0] = 0xe9;
    *(INT32 *)(target + 1) = (INT32)((BYTE *)hook - (target + 5));
    for (int i = 5; i < length; i++)
        target[i] = 0x90;
    VirtualProtect(target, length, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, length);
    return trampoline;
}

// Scale of an HD movie from its palette's spare bytes, or 1.
static int movie_scale(const BYTE *movie)
{
    if (!movie || movie[9] != 'H' || movie[13] != 'D')
        return 1;
    int scale = movie[17];
    return scale == 2 || scale == 4 ? scale : 1;
}

static void restore_videotex(void)
{
    if (!swapped.slot)
        return;
    // The game may have replaced the texture itself (device reset): then keep its new one.
    if (swapped.slot->texture == swapped.hd)
    {
        swapped.slot->texture = swapped.original;
        swapped.slot->width = swapped.width;
        swapped.slot->height = swapped.height;
    }
    swapped.slot = NULL;
}

static int swap_in_hd_texture(TextureSlot *slot, int scale)
{
    if (swapped.slot == slot && slot->texture == swapped.hd)
        return 1;
    restore_videotex();
    if (!slot->texture || slot->width != VIDEOTEX_WIDTH || slot->height != VIDEOTEX_HEIGHT)
        return 0;
    if (!hd_textures[scale])
    {
        D3DSURFACE_DESC desc;
        IDirect3DDevice9 *device = NULL;
        if (FAILED(IDirect3DTexture9_GetLevelDesc(slot->texture, 0, &desc)) ||
            FAILED(IDirect3DTexture9_GetDevice(slot->texture, &device)))
            return 0;
        // Same pool as the game's own (system memory): the renderer copies it into a video-memory texture of the
        // slot's size with UpdateTexture (FUN_10148590), which only takes a system-memory source.
        HRESULT result = IDirect3DDevice9_CreateTexture(device, VIDEOTEX_WIDTH * scale, VIDEOTEX_HEIGHT * scale, 1,
                                                        desc.Usage, desc.Format, desc.Pool, &hd_textures[scale], NULL);
        IDirect3DDevice9_Release(device);
        log_line("movies: HD texture %dx%d format %d pool %d -> %08lx", VIDEOTEX_WIDTH * scale,
                 VIDEOTEX_HEIGHT * scale, desc.Format, desc.Pool, result);
        if (FAILED(result))
        {
            hd_textures[scale] = NULL;
            return 0;
        }
    }
    swapped.slot = slot;
    swapped.original = slot->texture;
    swapped.hd = hd_textures[scale];
    swapped.width = slot->width;
    swapped.height = slot->height;
    slot->texture = swapped.hd;
    slot->width = VIDEOTEX_WIDTH * scale;
    slot->height = VIDEOTEX_HEIGHT * scale;
    return 1;
}

static void __fastcall hook_open(void *player, void *unused, const char *path)
{
    (void)unused;
    real_open(player, path);
    if (hd_movie.player == player)
        hd_movie.player = NULL;
    int scale = movie_scale(PLAYER_MEMORY(player));
    if (scale == 1 || PLAYER_WIDTH(player) % scale || PLAYER_HEIGHT(player) % scale)
        return;
    hd_movie.player = player;
    hd_movie.scale = scale;
    hd_movie.width = PLAYER_WIDTH(player);
    hd_movie.height = PLAYER_HEIGHT(player);
    PLAYER_WIDTH(player) = (short)(hd_movie.width / scale);
    PLAYER_HEIGHT(player) = (short)(hd_movie.height / scale);
    log_line("movies: %s is HD x%d (%dx%d)", path ? path : "?", scale, hd_movie.width, hd_movie.height);
}

static int __fastcall hook_upload(void *player, void *unused, TextureSlot *slot)
{
    (void)unused;
    if (hd_movie.player != player || !swap_in_hd_texture(slot, hd_movie.scale))
    {
        restore_videotex();
        return real_upload(player, slot);
    }
    short width = PLAYER_WIDTH(player), height = PLAYER_HEIGHT(player);
    PLAYER_WIDTH(player) = (short)hd_movie.width;
    PLAYER_HEIGHT(player) = (short)hd_movie.height;
    int result = real_upload(player, slot);
    PLAYER_WIDTH(player) = width;
    PLAYER_HEIGHT(player) = height;
    return result;
}

static void __fastcall hook_close(void *player, void *unused)
{
    (void)unused;
    if (hd_movie.player == player)
    {
        hd_movie.player = NULL;
        restore_videotex();
    }
    real_close(player);
}

// The DirectShow upload uses registers the compiler does not see: save them all around restore_videotex.
__attribute__((naked)) static void hook_dshow_upload(void)
{
    __asm__ volatile("pushal\n\t"
                     "pushfl\n\t"
                     "call %P0\n\t"
                     "popfl\n\t"
                     "popal\n\t"
                     "jmp *%1"
                     :
                     : "i"(restore_videotex), "m"(real_dshow_upload));
}

void movies_install(BYTE *app, void (*log)(const char *format, ...))
{
    static const short open_code[] = {0x6a, 0xff, 0x68, 0x100, 0x100, 0x100, 0x100};    // push -1; push handler
    static const short upload_code[] = {0x8b, 0xc1, 0x53, 0x8b, 0x98, 0x54, 0x02, 0x00, 0x00};
    static const short close_code[] = {0x53, 0x56, 0x8b, 0xf1, 0x8a, 0x46, 0x4b};
    static const short dshow_code[] = {0x83, 0xec, 0x6c, 0x56, 0x8b, 0xf1};
    app_base = app;
    log_line = log;
    if (!code_matches(VA_MMV_OPEN, open_code, 7) || !code_matches(VA_MMV_UPLOAD, upload_code, 9) ||
        !code_matches(VA_MMV_CLOSE, close_code, 7) || !code_matches(VA_DSHOW_UPLOAD, dshow_code, 6))
        return;
    real_open = (open_fn)detour(VA_MMV_OPEN, 7, (void *)hook_open);
    real_upload = (upload_fn)detour(VA_MMV_UPLOAD, 9, (void *)hook_upload);
    real_close = (close_fn)detour(VA_MMV_CLOSE, 7, (void *)hook_close);
    real_dshow_upload = detour(VA_DSHOW_UPLOAD, 6, (void *)hook_dshow_upload);
    log_line("movies: HD movie support on");
}
