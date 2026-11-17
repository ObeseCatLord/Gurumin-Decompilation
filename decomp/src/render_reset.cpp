// Direct3D state reset.

#include <string.h>
#include "d3d9_min.h"

extern "C" IDirect3DDevice9 *DAT_10f1fcf0;
extern "C" int DAT_10f6548c;
extern "C" int DAT_10f6550c;
extern "C" int DAT_10f65510;
extern "C" int DAT_10f654cc;
extern "C" void *DAT_10f60d30;
extern "C" void *DAT_10f0d540;
extern "C" D3DVIEWPORT9_ DAT_10f1fd30;
extern "C" D3DVIEWPORT9_ DAT_10f0ece0;
extern "C" int DAT_10f65470;
extern "C" int DAT_10f65474;
extern "C" int DAT_10202b34;
extern "C" int DAT_10202b38;
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
