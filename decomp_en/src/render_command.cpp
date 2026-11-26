// The renderer's state switch (EN build). Ported from the matched JP source decomp/src/render_command.cpp; the EN
// build adds anisotropic texture filtering to RENDER_TEXTURE_FILTER.
#include "map_view.h"
#include "render.h"
#include "render_command.h"
#include "vec4.h"

// Sets one group of Direct3D states selected by `command`: blend mode, texture addressing and filtering, alpha test,
// culling, depth test and write, fog, clears and scene brackets. RENDER_BLEND_MODE returns the previous blend mode;
// every other command returns 1; 0 when there is no device. Called by the drawing code through the whole frame.
extern "C" int Render_Command(int command, int value, int extra)
{
    if (g_d3dDevice == 0)
        return 0;
    switch (command)
    {
    case RENDER_ALPHA_TEST_OFF:
        if (value != 0)
            Render_SetRenderState(D3DRS_ALPHATESTENABLE, 0);
        else
            Render_SetRenderState(D3DRS_ALPHATESTENABLE, 1);
        break;
    case RENDER_DEPTH_FUNC:
        if (value == DEPTH_FUNC_LESS_EQUAL)
        {
            Render_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
            return 1;
        }
        if (value == DEPTH_FUNC_GREATER_EQUAL)
        {
            Render_SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
            return 1;
        }
        break;
    case RENDER_BLEND_MODE:
    {
        int previousMode = g_blendMode;
        g_blendMode = value;
        if (value == BLEND_MULTIPLY)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
        }
        else if (value == BLEND_ALPHA)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
        else if (value == BLEND_ADD)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        }
        else if (value == BLEND_DARKEN)
        {
            Vec4 unused;
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
        }
        else if (value == BLEND_ADD_TARGET)
        {
            Vec4 unused;
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        }
        else if (value == BLEND_INVERT)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
        }
        else if (value == BLEND_TARGET_ALPHA)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
        else if (value == BLEND_BLACK)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        }
        else if (value == BLEND_OPAQUE)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        }
        else if (value == BLEND_KEEP_TARGET)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        }
        else if (value == BLEND_COLOUR_MIX)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
        }
        else if (value == BLEND_MULTIPLY_COPY)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
        }
        else if (value == BLEND_DARKEN_COPY)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
        }
        else if (value == BLEND_ADD_COLOUR)
        {
            Render_SetRenderState(D3DRS_SPECULARENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            Render_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
            Render_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        }
        return previousMode;
    }
    case RENDER_CULL_MODE:
        if (value == CULL_NONE)
        {
            Render_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            return 1;
        }
        if (value == CULL_DEFAULT)
        {
            Render_SetRenderState(D3DRS_CULLMODE, g_mapPassRunning != 0 ? D3DCULL_CW : D3DCULL_CCW);
            return 1;
        }
        if (value == CULL_REVERSED)
        {
            // value is D3DCULL_CW here
            Render_SetRenderState(D3DRS_CULLMODE, (g_mapPassRunning != 0) + value);
            return 1;
        }
        break;
    case RENDER_TEXTURE_ADDRESS:
        if (value == TEXTURE_ADDRESS_CLAMP)
        {
            Render_SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            Render_SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            return 1;
        }
        if (value == TEXTURE_ADDRESS_MIRROR)
        {
            Render_SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
            Render_SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
            return 1;
        }
        Render_SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        Render_SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        return 1;
    case RENDER_ALPHA_TEST:
        if (value != 0)
        {
            Render_SetRenderState(D3DRS_ALPHATESTENABLE, 1);
            Render_SetRenderState(D3DRS_ALPHAREF, extra);
            Render_SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        }
        else
            Render_SetRenderState(D3DRS_ALPHATESTENABLE, 0);
        break;
    case RENDER_CLEAR:
        g_d3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, value, 1.0f, 0);
        return 1;
    case RENDER_CLEAR_DEPTH:
        g_d3dDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, (float)value / DEPTH_FIXED_ONE, 0);
        return 1;
    case RENDER_CLEAR_RECT_128:
    {
        D3DRECT rect = {0, 0, CLEAR_SQUARE_128, CLEAR_SQUARE_128};
        g_d3dDevice->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, value, 1.0f, 0);
        return 1;
    }
    case RENDER_CLEAR_STRIP_48:
    {
        D3DRECT rect = {0, 0, CLEAR_STRIP_WIDTH, CLEAR_STRIP_HEIGHT_48};
        g_d3dDevice->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, value, 1.0f, 0);
        return 1;
    }
    case RENDER_CLEAR_STRIP_64:
    {
        D3DRECT rect = {0, 0, CLEAR_STRIP_WIDTH, CLEAR_STRIP_HEIGHT_64};
        g_d3dDevice->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, value, 1.0f, 0);
        return 1;
    }
    case RENDER_CLEAR_STRIP_128:
    {
        D3DRECT rect = {0, 0, CLEAR_STRIP_WIDTH, CLEAR_STRIP_HEIGHT_128};
        g_d3dDevice->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, value, 1.0f, 0);
        return 1;
    }
    case RENDER_CLEAR_STRIP_256:
    {
        D3DRECT rect = {0, 0, CLEAR_STRIP_WIDTH, CLEAR_STRIP_HEIGHT_256};
        g_d3dDevice->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, value, 1.0f, 0);
        return 1;
    }
    case RENDER_CLEAR_RECT_3:
    {
        D3DRECT rect = {0, 0, CLEAR_SQUARE_SMALL, CLEAR_SQUARE_SMALL};
        g_d3dDevice->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, value, 1.0f, 0);
        return 1;
    }
    case RENDER_CLEAR_RECT_512:
    {
        D3DRECT rect = {0, 0, CLEAR_SQUARE_512, CLEAR_SQUARE_512};
        g_d3dDevice->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, value, 1.0f, 0);
        return 1;
    }
    case RENDER_FLUSH_SCENE:
        Render_EndScene();
    case RENDER_BEGIN_SCENE:
        Render_BeginScene();
        return 1;
    case RENDER_END_SCENE:
        Render_EndScene();
        return 1;
    case RENDER_TEXTURE_FILTER:
        if (value != 0)
        {
            if (g_textureFilterSetting < ANISOTROPY_SETTING_FIRST)
            {
                Render_SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                Render_SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                Render_SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                Render_SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                return 1;
            }
            Render_SetSamplerState(0, D3DSAMP_MAXANISOTROPY, g_textureFilterSetting - ANISOTROPY_SETTING_BASE);
            if (Render_SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC) < 0)
                Render_SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            if (Render_SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC) < 0)
                Render_SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            if (Render_SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC) < 0)
                Render_SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            if (Render_SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC) < 0)
            {
                Render_SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                return 1;
            }
            break;
        }
        Render_SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        Render_SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        Render_SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        Render_SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        return 1;
    case RENDER_DEPTH_FUNC_ALWAYS:
        if (value != 0)
            Render_SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        else
            Render_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        break;
    case RENDER_DEPTH_WRITE:
        if (value != 0)
        {
            Render_SetRenderState(D3DRS_ZWRITEENABLE, 1);
            return 1;
        }
        Render_SetRenderState(D3DRS_ZWRITEENABLE, 0);
        return 1;
    case RENDER_DEPTH_TEST:
        if (value != 0)
        {
            Render_SetRenderState(D3DRS_ZENABLE, 1);
            return 1;
        }
        Render_SetRenderState(D3DRS_ZENABLE, 0);
        return 1;
    case RENDER_FOG:
        Render_SetRenderState(D3DRS_FOGENABLE, value);
        if (value != 0)
            Render_SetRenderState(D3DRS_FOGCOLOR, extra);
        Render_SetRenderState(D3DRS_FOGVERTEXMODE, 0);
        Render_SetRenderState(D3DRS_FOGTABLEMODE, 0);
        break;
    }
    return 1;
}
