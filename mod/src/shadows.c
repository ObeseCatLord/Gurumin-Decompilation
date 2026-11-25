// Larger dynamic-shadow textures for Gurumin (JP PC, bin\APP.dll). UNTESTED on the Japanese game: off by default.
//
// Ported from Gurumin Enhanced Rendering (MIT, ObeseCatLord), src/shadows.hpp, for the English Steam game.exe. The
// idea is the same; the hooks follow the JP decompilation (see docs/CAMERA_SHADOWS_JP.md for the evidence):
//
//   - ModelBank::CreateTextures 0x1017c160 (decomp/src/model_bank.cpp, matched) creates the ten shadow targets
//     "shadow0".."shadow9" with Texture_CreateFromPixels(name, 256, 256, pixels, 1, 0) (0x10175c90). Its calls to
//     that function are redirected; for the shadow names the wrapper asks for `size` x `size` instead and falls
//     back to 256 when that fails. Alpha 1 makes Create (0x10169c60) build a render-target texture of that size
//     and keep the size in sourceWidth/sourceHeight (+0x2c/+0x30), which is what the game draws into.
//   - Node::RenderShadowTexture 0x101a24e0 (decomp/src/shadow_texture.cpp, matched) and the clear loop in
//     CreateTextures make a shadow texture the render target with TextureObject::UpdateImage 0x10169f70, which calls
//     BeginRenderTo(sourceWidth, sourceHeight, 0). BeginRenderTo (decomp/src/texture_bind.cpp) clips the viewport
//     to the back-buffer size 0x10202b24/0x10202b28, so a 2048 target on a 1280x960 screen would only be drawn in
//     part. Those two UpdateImage calls are redirected; for a large shadow texture the wrapper raises the two
//     globals to the texture size around the call and puts them back.
//   - BeginRenderTo pairs the target with the off-screen depth buffer DAT_10f08d38, which Render_OnResetDevice
//     (decomp/src/render_on_reset.cpp) creates DAT_10202b18 squared. A depth buffer smaller than the target is not
//     allowed, so IDirect3DDevice9::CreateDepthStencilSurface is hooked and, when its output pointer is
//     &DAT_10f08d38, the buffer is made at least `size` squared.
//
// No texel or projection change is needed: the shadow camera (RenderShadowTexture) and the projector that lays the
// texture on the ground (Node::DrawShadow) work in clip space and 0..1 texture space, not in pixels.
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stddef.h>
#include <string.h>
#include "display.h"
#include "shadows.h"

#define VA_CREATE_TEXTURES 0x1017c160u          // ModelBank::CreateTextures, 1037 bytes, matched
#define SIZE_CREATE_TEXTURES 1037u
#define VA_RENDER_SHADOW_TEXTURE 0x101a24e0u    // Node::RenderShadowTexture, 1267 bytes, matched
#define SIZE_RENDER_SHADOW_TEXTURE 1267u
#define VA_TEXTURE_FROM_PIXELS 0x10175c90u      // Texture_CreateFromPixels(name, width, height, pixels, alpha, flags)
#define VA_UPDATE_IMAGE 0x10169f70u             // TextureObject::UpdateImage(value), thiscall
#define VA_TEXTURE_SLOTS 0x118d0ba8u            // TextureObject DAT_118d0ba8[1600], 0x38 bytes each
#define VA_SCREEN_WIDTH 0x10202b24u
#define VA_SCREEN_HEIGHT 0x10202b28u
#define VA_OFFSCREEN_DEPTH 0x10f08d38u          // IDirect3DSurface9 *: depth buffer for off-screen targets

#define TEXTURE_SLOT_COUNT 1600
#define TEXTURE_MAX_SIDE 2048                   // TextureObject::Create refuses larger textures
#define NATIVE_EDGE 256
// Calls in the matched functions (decomp/src/model_bank.cpp, decomp/src/shadow_texture.cpp).
#define CREATE_CALLS 10                         // 10 shadow (one call in a loop), 3 crossfade, 6 mirror/save/minimap
#define CLEAR_CALLS 1                           // the shadow clear loop
#define RENDER_CALLS 1

// The parts of TextureObject (decomp/include/texture_object.h) used here.
typedef struct
{
    IDirect3DTexture9 *texture;   // 0x00
    IDirect3DSurface9 *surface;   // 0x04: render-target surface
    int width, height;            // 0x08, 0x0c
    int target;                   // 0x10
    int unused_14[6];             // 0x14..0x28
    int source_width;             // 0x2c: size BeginRenderTo draws at
    int source_height;            // 0x30
    int generated_alpha;          // 0x34
} TextureObject;
_Static_assert(offsetof(TextureObject, source_width) == 0x2c, "TextureObject layout");
_Static_assert(sizeof(TextureObject) == 0x38, "TextureObject size");

typedef int(__cdecl *create_fn)(const char *name, int width, int height, const void *pixels, int alpha, int flags);
typedef int(__thiscall *update_image_fn)(TextureObject *texture, int value);
typedef HRESULT(WINAPI *create_depth_fn)(IDirect3DDevice9 *, UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL,
                                         IDirect3DSurface9 **, HANDLE *);

static void (*log_line)(const char *format, ...);
static int requested_edge, edge, active;
static UINT depth_edge;            // size of the current off-screen depth buffer (0 = not seen yet)
static create_fn game_create;
static update_image_fn game_update_image;
static create_depth_fn real_create_depth;
static TextureObject *tracked[10];
static unsigned long raised_targets;

static TextureObject *texture_slot(int slot)
{
    return (TextureObject *)app_address(VA_TEXTURE_SLOTS) + slot;
}

// 0..9 for "shadow0".."shadow9" (sprintf "shadow%0d" in CreateTextures), else -1.
static int shadow_index(const char *name)
{
    if (!name || strncmp(name, "shadow", 6) != 0 || name[6] < '0' || name[6] > '9' || name[7] != 0)
        return -1;
    return name[6] - '0';
}

static int is_tracked(const TextureObject *texture)
{
    for (int i = 0; i < 10; i++)
        if (tracked[i] == texture)
            return 1;
    return 0;
}

static int __cdecl create_hook(const char *name, int width, int height, const void *pixels, int alpha, int flags)
{
    int index = shadow_index(name);
    if (index < 0 || width != NATIVE_EDGE || height != NATIVE_EDGE || alpha != 1 || flags != 0 || edge <= NATIVE_EDGE)
        return game_create(name, width, height, pixels, alpha, flags);
    if (depth_edge < (UINT)edge)
    {
        log_line("shadows: %s kept at %d: the off-screen depth buffer is %u", name, width, depth_edge);
        return game_create(name, width, height, pixels, alpha, flags);
    }
    // Alpha 1 = a render target; `pixels` is not read for it (TextureObject::Create keeps no copy).
    int slot = game_create(name, edge, edge, pixels, alpha, flags);
    if (slot >= 0 && slot < TEXTURE_SLOT_COUNT)
    {
        TextureObject *texture = texture_slot(slot);
        if (texture->source_width == edge && texture->source_height == edge && texture->texture)
        {
            tracked[index] = texture;
            log_line("shadows: %s slot %d is %dx%d", name, slot, edge, edge);
        }
        else
            // The name was already loaded (Texture_CreateFromPixels returns the existing slot): keep it as it is.
            log_line("shadows: %s slot %d already exists at %dx%d", name, slot, texture->source_width,
                     texture->source_height);
        return slot;
    }
    // A failed create registers no name, so the game's own size can still be asked for.
    log_line("shadows: %s could not be made %dx%d; using %dx%d", name, edge, edge, width, height);
    return game_create(name, width, height, pixels, alpha, flags);
}

// UpdateImage on a shadow texture: BeginRenderTo clips its viewport to the screen size, so let the screen size be
// at least the texture size for this call.
static int __fastcall update_image_hook(TextureObject *texture, void *unused_edx, int value)
{
    (void)unused_edx;
    if (!is_tracked(texture))
        return game_update_image(texture, value);
    int need = texture->source_width > texture->source_height ? texture->source_width : texture->source_height;
    if (depth_edge < (UINT)need)
    {
        // The depth buffer was re-created smaller (after a device reset): fail as BeginRenderTo would, and
        // RenderShadowTexture turns the dynamic shadows off (DAT_10203434 = 0) for the round shadow image.
        static int reported;
        if (!reported++)
            log_line("shadows: depth buffer %u is smaller than the %d shadow target: dynamic shadows off", depth_edge,
                     need);
        return 0;
    }
    int *screen_width = (int *)app_address(VA_SCREEN_WIDTH), *screen_height = (int *)app_address(VA_SCREEN_HEIGHT);
    int width = *screen_width, height = *screen_height;
    if (width >= need && height >= need)
        return game_update_image(texture, value);
    *screen_width = width > need ? width : need;
    *screen_height = height > need ? height : need;
    int result = game_update_image(texture, value);
    *screen_width = width;
    *screen_height = height;
    if (!raised_targets++)
        log_line("shadows: first %d target drawn with the full viewport (screen %dx%d)", need, width, height);
    return result;
}

static HRESULT WINAPI create_depth_hook(IDirect3DDevice9 *device, UINT width, UINT height, D3DFORMAT format,
                                        D3DMULTISAMPLE_TYPE multisample, DWORD quality, BOOL discard,
                                        IDirect3DSurface9 **surface, HANDLE *shared)
{
    if (!active || surface != (IDirect3DSurface9 **)app_address(VA_OFFSCREEN_DEPTH))
        return real_create_depth(device, width, height, format, multisample, quality, discard, surface, shared);
    if (edge > NATIVE_EDGE && (width < (UINT)edge || height < (UINT)edge))
    {
        UINT w = width > (UINT)edge ? width : (UINT)edge, h = height > (UINT)edge ? height : (UINT)edge;
        HRESULT result = real_create_depth(device, w, h, format, multisample, quality, discard, surface, shared);
        log_line("shadows: off-screen depth buffer %ux%u -> %ux%u: %08lx", width, height, w, h, result);
        if (SUCCEEDED(result))
        {
            depth_edge = w < h ? w : h;
            return result;
        }
    }
    HRESULT result = real_create_depth(device, width, height, format, multisample, quality, discard, surface, shared);
    if (SUCCEEDED(result))
        depth_edge = width < height ? width : height;
    return result;
}

void shadows_device_created(void *device_pointer)
{
    IDirect3DDevice9 *device = device_pointer;
    if (!active || !device)
        return;
    D3DCAPS9 caps;
    if (SUCCEEDED(IDirect3DDevice9_GetDeviceCaps(device, &caps)))
    {
        int limit = (int)(caps.MaxTextureWidth < caps.MaxTextureHeight ? caps.MaxTextureWidth : caps.MaxTextureHeight);
        while (edge > NATIVE_EDGE && edge > limit)
            edge /= 2;
        log_line("shadows: device allows %lux%lu textures: shadow size %d", caps.MaxTextureWidth,
                 caps.MaxTextureHeight, edge);
    }
    void **vtable = *(void ***)device;
    int index = (int)(offsetof(IDirect3DDevice9Vtbl, CreateDepthStencilSurface) / sizeof(void *));
    void *original = vtable_hook(vtable, index, (void *)create_depth_hook);
    if (original)
        real_create_depth = (create_depth_fn)original;
    log_line("shadows: CreateDepthStencilSurface (vtable %d) hooked", index);
}

int shadows_active(void)
{
    return active;
}

void shadows_install(void (*log)(const char *format, ...), const char *ini)
{
    log_line = log;
    if (!GetPrivateProfileIntA("shadows", "enabled", 0, ini))
    {
        log_line("shadows: off");
        return;
    }
    requested_edge = GetPrivateProfileIntA("shadows", "size", 2048, ini);
    edge = NATIVE_EDGE;
    while (edge * 2 <= requested_edge && edge * 2 <= TEXTURE_MAX_SIDE)
        edge *= 2;
    log_line("shadows: size %d asked, %d used (power of two, %d..%d)", requested_edge, edge, NATIVE_EDGE,
             TEXTURE_MAX_SIDE);
    if (edge <= NATIVE_EDGE)
        return;

    // Find every call first; patch only when all three sets are exactly as the matched source has them.
    DWORD create_sites[CREATE_CALLS + 1], clear_sites[CLEAR_CALLS + 1], render_sites[RENDER_CALLS + 1];
    int creates = find_calls(VA_CREATE_TEXTURES, SIZE_CREATE_TEXTURES, VA_TEXTURE_FROM_PIXELS, create_sites,
                             CREATE_CALLS + 1);
    int clears = find_calls(VA_CREATE_TEXTURES, SIZE_CREATE_TEXTURES, VA_UPDATE_IMAGE, clear_sites, CLEAR_CALLS + 1);
    int renders = find_calls(VA_RENDER_SHADOW_TEXTURE, SIZE_RENDER_SHADOW_TEXTURE, VA_UPDATE_IMAGE, render_sites,
                             RENDER_CALLS + 1);
    log_line("shadows: ModelBank::CreateTextures %08lx: %d call(s) to Texture_CreateFromPixels %08lx (expected %d), "
             "%d to TextureObject::UpdateImage %08lx (expected %d)",
             (unsigned long)VA_CREATE_TEXTURES, creates, (unsigned long)VA_TEXTURE_FROM_PIXELS, CREATE_CALLS, clears,
             (unsigned long)VA_UPDATE_IMAGE, CLEAR_CALLS);
    log_line("shadows: Node::RenderShadowTexture %08lx: %d call(s) to UpdateImage (expected %d)",
             (unsigned long)VA_RENDER_SHADOW_TEXTURE, renders, RENDER_CALLS);
    if (creates != CREATE_CALLS || clears != CLEAR_CALLS || renders != RENDER_CALLS)
    {
        log_line("shadows: call sites not as in the decompiled source: shadows left at 256");
        return;
    }
    game_create = (create_fn)app_address(VA_TEXTURE_FROM_PIXELS);
    game_update_image = (update_image_fn)app_address(VA_UPDATE_IMAGE);
    redirect_calls(create_sites, creates, (void *)create_hook);
    redirect_calls(clear_sites, clears, (void *)update_image_hook);
    redirect_calls(render_sites, renders, (void *)update_image_hook);
    for (int i = 0; i < creates; i++)
        log_line("shadows: create call at %08lx redirected", create_sites[i]);
    log_line("shadows: UpdateImage calls at %08lx (clear) and %08lx (render) redirected", clear_sites[0],
             render_sites[0]);
    log_line("shadows: screen size %08lx/%08lx, texture slots %08lx, off-screen depth %08lx",
             (unsigned long)VA_SCREEN_WIDTH, (unsigned long)VA_SCREEN_HEIGHT, (unsigned long)VA_TEXTURE_SLOTS,
             (unsigned long)VA_OFFSCREEN_DEPTH);
    active = 1;
}
