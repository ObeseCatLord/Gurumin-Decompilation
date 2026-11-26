// Startup check of off-screen rendering (EN build). Ported from the JP source decomp/src/render_target_probe.cpp
// (APP.dll 0x10017d30, unmatched) and the EN bytes.
#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "map_view.h"
#include "model_bank.h"
#include "render.h"
#include "texture_mgr.h"

// Side of the probe texture and the bytes of its black start image (the first 0x60 read-back bytes are what is tested).
const int PROBE_TEXTURE_SIDE = 0x100;
const int PROBE_IMAGE_BYTES = 0xc00;
// Bytes of read-back pixels that are tested (32 pixels of three bytes) and the bytes per pixel.
const int PROBE_TESTED_BYTES = 0x60;
const int PROBE_BYTES_PER_PIXEL = 3;
// Blue byte a pixel of the blue square must exceed.
const int PROBE_BRIGHT_BLUE = 0xf0;

// Puts the normal render target back after the probe. Runs at every exit of RenderTarget_Probe.
static void RestoreRenderTarget()
{
    if (g_renderToBackBuffer != 0)
        Render_ResetTarget();
    else
        Render_ResetSwapTarget();
}

// Startup check of off-screen rendering: renders a clear and the background square into a 256x256 texture, reads it
// back, then draws the blue square and reads it back again. Cards that cannot do this turn the effects that need it
// off. Returns 1 when rendering to textures works. Called once at startup. EN 0x5c2960.
extern "C" int RenderTarget_Probe(void)
{
    unsigned char pixels[PROBE_IMAGE_BYTES];
    g_renderTargetsWork = 1;
    unsigned char *black = (unsigned char *)operator new(PROBE_IMAGE_BYTES);
    g_blueSquareSeen = 0;
    memset(black, 0, PROBE_IMAGE_BYTES);
    int slot = Texture_CreateFromPixels("crtcc", PROBE_TEXTURE_SIDE, PROBE_TEXTURE_SIDE, black, 1, 0);
    free(black);
    if (slot < 0)
    {
        g_renderTargetsWork = 0;
        g_renderToTextureWorks = 0;
        return 0;
    }
    g_textures[slot].UpdateImage(0);
    Render_DrawBackgroundSquare();
    g_textures[slot].CaptureRgbFirst(pixels);
    int bright;
    for (int i = 0; i < PROBE_TESTED_BYTES; i += PROBE_BYTES_PER_PIXEL)
    {
        if (pixels[i + 2] <= PROBE_BRIGHT_BLUE)
        {
            bright = 0;
            break;
        }
        bright = 1;
    }
    g_blueSquareSeen = bright;
    if (bright == 0)
    {
        g_shadowsEnabled = 0;
        g_shadowsSwitchedOn = 0;
        g_alwaysPlaceOnGround = 1;
    }
    Render_Command(RENDER_FLUSH_SCENE, 0, 0);
    g_renderTargetsWork = 1;
    if (g_textures[slot].UpdateImage(0) == 0)
    {
        g_renderTargetsWork = 0;
        g_renderToTextureWorks = 0;
        RestoreRenderTarget();
        Texture_Release(slot, 1);
        return 0;
    }
    Render_DrawBlueSquare();
    g_textures[slot].CaptureRgbRows(pixels);
    for (int j = 0; j < PROBE_TESTED_BYTES; j += PROBE_BYTES_PER_PIXEL)
    {
        if (pixels[j + 2] <= PROBE_BRIGHT_BLUE)
        {
            Render_Command(RENDER_FLUSH_SCENE, 0, 0);
            RestoreRenderTarget();
            g_renderTargetsWork = 0;
            g_renderToTextureWorks = 0;
            Texture_Release(slot, 1);
            return 0;
        }
    }
    Render_Command(RENDER_FLUSH_SCENE, 0, 0);
    RestoreRenderTarget();
    g_renderTargetsWork = 1;
    g_renderToTextureWorks = 1;
    Texture_Release(slot, 1);
    return 1;
}
