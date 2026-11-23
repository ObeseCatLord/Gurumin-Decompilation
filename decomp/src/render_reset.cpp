// Direct3D state reset.

#include <string.h>
#include "d3d9_min.h"

#include "render_vars.h"

extern "C" int DAT_10f6550c;
extern "C" int DAT_10f65510;
extern "C" void FUN_10143b50(void);

// Render_ResetState: unbinds the textures of stages 0..3, resets the cached stage states, and when an off-screen
// target was active (DAT_10f654cc) restores the back buffer, depth buffer and full-screen viewport.
// Not byte-identical yet (see open.tsv).
extern "C" void FUN_1014b390(void)
{
    DAT_10f6548c = 0;
    DAT_10f1fcf0->SetTexture(0, 0);
    DAT_10f6550c = 1;
    DAT_10f1fcf0->SetTexture(1, 0);
    DAT_10f65510 = 1;
    DAT_10f1fcf0->SetTexture(2, 0);
    DAT_10f1fcf0->SetTexture(3, 0);
    FUN_10143b50();
    if (DAT_10f654cc != 0)
    {
        DAT_10f1fcf0->SetRenderTarget(0, DAT_10f60d30);
        DAT_10f1fcf0->SetDepthStencilSurface(DAT_10f0d540);
        memset(&DAT_10f1fd30, 0, sizeof(DAT_10f1fd30));
        DAT_10f1fd30 = DAT_10f0ece0;
        DAT_10202b34 = DAT_10f65470;
        DAT_10202b38 = DAT_10f65474;
        DAT_10f1fcf0->SetViewport(&DAT_10f0ece0);
    }
}

// Default-pool Direct3D resources that do not survive a device reset (Render_OnResetDevice recreates them).
extern "C" IUnknownLike *DAT_10f141dc;
extern "C" IUnknownLike *DAT_10f1fd28;            // created with the device
extern "C" IUnknownLike *DAT_10f08d38;
extern "C" void FUN_10147760(void);  // BufferPool_ReleaseDynamic
extern "C" int FUN_10148350(void);   // TexCache_ReleaseAll

// Render_OnLostDevice (0x1014b2a0). Releases every default-pool resource before IDirect3DDevice9::Reset: the draw
// list buffers, the render targets, the 2D vertex buffer, the dynamic buffer pool, the texture cache and the
// single objects below. Some pointers are cleared afterwards and some are left dangling until they are recreated,
// as in the original. Returns 1.
extern "C" int FUN_1014b2a0(void)
{
    int list;
    for (list = 0; list < 16; list++)
    {
        for (int i = 0; i < 0x46; i++)
        {
            if (DAT_10f0ee00[list][i] != 0)
                DAT_10f0ee00[list][i]->Release();
        }
    }
    for (int target = 0; target < 8; target++)
    {
        if (DAT_10f1fd04[target] != 0)
            DAT_10f1fd04[target]->Release();
        DAT_10f1fd04[target] = 0;
    }
    DAT_10f654ac->Release();
    FUN_10147760();
    FUN_10148350();
    if (DAT_10f141dc != 0)
        DAT_10f141dc->Release();
    DAT_10f141dc = 0;
    if (DAT_10f1a864 != 0)
        DAT_10f1a864->Release();
    if (DAT_10f1fcfc[0] != 0)
        DAT_10f1fcfc[0]->Release();
    if (DAT_10f1fd00 != 0)
        DAT_10f1fd00->Release();
    if (DAT_10f1fd28 != 0)
        DAT_10f1fd28->Release();
    if (DAT_10f1fd2c != 0)
        DAT_10f1fd2c->Release();
    DAT_10f1a864 = 0;
    if (DAT_10f08d38 != 0)
        DAT_10f08d38->Release();
    DAT_10f08d38 = 0;
    return 1;
}


// 0x101439e0 (Render_ResetTarget). Unbinds the textures of stages 0..3, makes the back buffer and depth buffer the
// render targets again and sets the viewport to the whole back buffer or one half of it (split screen). Unless an
// off-screen target is set up, this is also saved as the state Render_ResetState returns to.
extern "C" void FUN_101439e0(void)
{
    DAT_10f6548c = 0;
    DAT_10f1fcf0->SetTexture(0, 0);
    DAT_10f6550c = 1;
    DAT_10f1fcf0->SetTexture(1, 0);
    DAT_10f65510 = 1;
    DAT_10f1fcf0->SetTexture(2, 0);
    DAT_10f1fcf0->SetTexture(3, 0);
    DAT_10f1fcf0->SetRenderTarget(0, DAT_10f141dc);
    DAT_10f1fcf0->SetDepthStencilSurface(DAT_10f1a864);
    int width = DAT_10202b24;
    int height = DAT_10202b28;
    memset(&DAT_10f1fd30, 0, sizeof(DAT_10f1fd30));
    DAT_10f1fd30.X = 0;
    DAT_10f1fd30.Y = 0;
    DAT_10f1fd30.Width = width;
    DAT_10f1fd30.Height = height;
    if (DAT_10f65480 == 0)
    {
        DAT_10f1fd30.X = 0;
        DAT_10f1fd30.Width = width;
    }
    else if (DAT_10f65480 == 1)
    {
        DAT_10f1fd30.X = 0;
        DAT_10f1fd30.Width = width / 2;
    }
    else if (DAT_10f65480 == 2)
    {
        DAT_10f1fd30.X = width / 2;
        DAT_10f1fd30.Width = width / 2;
    }
    DAT_10f1fd30.MinZ = 0;
    DAT_10f1fd30.MaxZ = 1.0f;
    DAT_10202b34 = width;
    DAT_10202b38 = height;
    if (DAT_10f65478 == 0)
    {
        DAT_10f0d540 = DAT_10f1a864;
        DAT_10f60d30 = DAT_10f141dc;
        DAT_10f0ece0 = DAT_10f1fd30;
        DAT_10f65470 = width;
        DAT_10f65474 = height;
    }
    DAT_10f1fcf0->SetViewport(&DAT_10f1fd30);
}

// windows.h-style min (vec4.cpp): each argument is evaluated twice.
#define MIN(a, b) ((a) > (b) ? (b) : (a))


// 0x1014b480 (Render_ResetSwapTarget). Like Render_ResetTarget, but the render target is the swap target in use and
// the depth buffer the second one (DAT_10f08d38), and the viewport is at most DAT_10202b18 wide and high.
extern "C" void FUN_1014b480(void)
{
    DAT_10f6548c = 0;
    DAT_10f1fcf0->SetTexture(0, 0);
    DAT_10f6550c = 1;
    DAT_10f1fcf0->SetTexture(1, 0);
    DAT_10f65510 = 1;
    DAT_10f1fcf0->SetTexture(2, 0);
    DAT_10f1fcf0->SetTexture(3, 0);
    FUN_10143b50();
    DAT_10f1fcf0->SetRenderTarget(0, DAT_10f1fcfc[DAT_10f65490]);
    DAT_10f1fcf0->SetDepthStencilSurface(DAT_10f08d38);
    memset(&DAT_10f1fd30, 0, sizeof(DAT_10f1fd30));
    DAT_10f1fd30.Y = 0;
    int width = (int)MIN(DAT_10202b18, (float)DAT_10202b24);
    int height = (int)MIN(DAT_10202b18, (float)DAT_10202b28);
    if (DAT_10f65480 == 0)
    {
        DAT_10f1fd30.X = 0;
        DAT_10f1fd30.Width = width;
    }
    else if (DAT_10f65480 == 1)
    {
        DAT_10f1fd30.X = 0;
        DAT_10f1fd30.Width = width / 2;
    }
    else if (DAT_10f65480 == 2)
    {
        DAT_10f1fd30.X = width / 2;
        DAT_10f1fd30.Width = width / 2;
    }
    DAT_10f1fd30.Height = height;
    DAT_10f1fd30.MinZ = 0;
    DAT_10f1fd30.MaxZ = 1.0f;
    if (DAT_10f65478 == 0)
    {
        DAT_10f60d30 = DAT_10f1fcfc[DAT_10f65490];
        DAT_10f0d540 = DAT_10f08d38;
        DAT_10f0ece0 = DAT_10f1fd30;
        DAT_10f65470 = width;
        DAT_10f65474 = height;
    }
    DAT_10202b34 = width;
    DAT_10202b38 = height;
    DAT_10f1fcf0->SetViewport(&DAT_10f1fd30);
}
