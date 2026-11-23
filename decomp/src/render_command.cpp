// The renderer's state switch.

#include "d3d9_min.h"
#include "vec4.h"

#include "render_state.h"
#include "render_vars.h"

extern "C" int FUN_1013ef30(unsigned long state, unsigned long value);                       // Render_SetRenderState
extern "C" int FUN_1013f000(unsigned long sampler, unsigned long type, unsigned long value);  // Render_SetSamplerState
extern "C" int FUN_1013ed20(void);                                                            // Render_BeginScene
extern "C" int FUN_1013ed60(void);                                                            // Render_EndScene

// Direct3D render states and sampler states as numbered by the cached setters.
#define RS_ZENABLE 7
#define RS_ZWRITEENABLE 14
#define RS_ALPHATESTENABLE 15
#define RS_SRCBLEND 19
#define RS_DESTBLEND 20
#define RS_CULLMODE 22
#define RS_ZFUNC 23
#define RS_ALPHAREF 24
#define RS_ALPHAFUNC 25
#define RS_ALPHABLENDENABLE 27
#define RS_FOGENABLE 28
#define RS_SPECULARENABLE 29
#define RS_FOGCOLOR 34
#define RS_FOGTABLEMODE 35
#define RS_FOGVERTEXMODE 140
#define SAMP_ADDRESSU 1
#define SAMP_ADDRESSV 2
#define SAMP_MAGFILTER 5
#define SAMP_MINFILTER 6

// Direct3D constants the switch uses.
#define BLEND_ZERO 1
#define BLEND_ONE 2
#define BLEND_SRCCOLOR 3
#define BLEND_INVSRCCOLOR 4
#define BLEND_SRCALPHA 5
#define BLEND_INVSRCALPHA 6
#define BLEND_DESTCOLOR 9
#define BLEND_INVDESTCOLOR 10
#define CULL_NONE_D3D 1
#define CULL_CW_D3D 2
#define CULL_CCW_D3D 3
#define CMP_LESSEQUAL 4
#define CMP_GREATEREQUAL 7
#define CMP_ALWAYS 8
#define ADDRESS_WRAP 1
#define ADDRESS_MIRROR 2
#define ADDRESS_CLAMP 3
#define FILTER_POINT 1
#define FILTER_LINEAR 2
#define CLEAR_TARGET_AND_ZBUFFER 3
#define CLEAR_ZBUFFER 2
#define CLEAR_STRIP_WIDTH 0x17c
#define CLEAR_STRIP_HEIGHT_48 0x30
#define CLEAR_STRIP_HEIGHT_64 0x40
#define CLEAR_STRIP_HEIGHT_128 0x80
#define CLEAR_STRIP_HEIGHT_256 0x100
#define CLEAR_SQUARE_SMALL 3
#define CLEAR_SQUARE_128 0x80
#define CLEAR_SQUARE_512 0x200
#define OPAQUE_DEPTH 1.0f
#define DEPTH_FIXED_ONE 65536.0f
#define CMPFUNC_GREATEREQUAL_ALPHA 5

// 0x1013f070 (Render_Command). Sets one group of Direct3D states selected by `command` (RenderCommand): blend mode,
// texture addressing and filtering, alpha test, culling, depth test and write, fog, clears and scene brackets.
// RENDER_BLEND_MODE returns the previous blend mode; every other command returns 1; 0 when there is no device.
extern "C" int FUN_1013f070(int command, int value, int extra)
{
    if (DAT_10f1fcf0 == 0)
        return 0;
    switch (command)
    {
    case RENDER_ALPHA_TEST_OFF:
        if (value != 0)
            FUN_1013ef30(RS_ALPHATESTENABLE, 0);
        else
            FUN_1013ef30(RS_ALPHATESTENABLE, 1);
        break;
    case RENDER_DEPTH_FUNC:
        if (value == 0)
        {
            FUN_1013ef30(RS_ZFUNC, CMP_LESSEQUAL);
            return 1;
        }
        if (value == 2)
        {
            FUN_1013ef30(RS_ZFUNC, CMP_GREATEREQUAL);
            return 1;
        }
        break;
    case RENDER_BLEND_MODE:
    {
        int previousMode = DAT_10202b14;
        DAT_10202b14 = value;
        if (value == BLEND_MULTIPLY)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_ZERO);
            FUN_1013ef30(RS_DESTBLEND, BLEND_SRCCOLOR);
        }
        else if (value == BLEND_ALPHA)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_SRCALPHA);
            FUN_1013ef30(RS_DESTBLEND, BLEND_INVSRCALPHA);
        }
        else if (value == BLEND_ADD)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_SRCALPHA);
            FUN_1013ef30(RS_DESTBLEND, BLEND_ONE);
        }
        else if (value == BLEND_DARKEN)
        {
            Vec4 unused;
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_ZERO);
            FUN_1013ef30(RS_DESTBLEND, BLEND_INVSRCCOLOR);
        }
        else if (value == BLEND_ADD_TARGET)
        {
            Vec4 unused;
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_DESTCOLOR);
            FUN_1013ef30(RS_DESTBLEND, BLEND_ONE);
        }
        else if (value == BLEND_INVERT)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_INVDESTCOLOR);
            FUN_1013ef30(RS_DESTBLEND, BLEND_INVSRCCOLOR);
        }
        else if (value == BLEND_TARGET_ALPHA)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_DESTCOLOR);
            FUN_1013ef30(RS_DESTBLEND, BLEND_INVSRCALPHA);
        }
        else if (value == BLEND_BLACK)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_ZERO);
            FUN_1013ef30(RS_DESTBLEND, BLEND_ZERO);
        }
        else if (value == BLEND_OPAQUE)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 0);
            FUN_1013ef30(RS_SRCBLEND, BLEND_ONE);
            FUN_1013ef30(RS_DESTBLEND, BLEND_ZERO);
        }
        else if (value == BLEND_KEEP_TARGET)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_ZERO);
            FUN_1013ef30(RS_DESTBLEND, BLEND_ONE);
        }
        else if (value == BLEND_COLOUR_MIX)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_SRCCOLOR);
            FUN_1013ef30(RS_DESTBLEND, BLEND_INVSRCCOLOR);
        }
        else if (value == BLEND_MULTIPLY_COPY)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_ZERO);
            FUN_1013ef30(RS_DESTBLEND, BLEND_SRCCOLOR);
        }
        else if (value == BLEND_DARKEN_COPY)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_ZERO);
            FUN_1013ef30(RS_DESTBLEND, BLEND_INVSRCCOLOR);
        }
        else if (value == BLEND_ADD_COLOUR)
        {
            FUN_1013ef30(RS_SPECULARENABLE, 1);
            FUN_1013ef30(RS_ALPHABLENDENABLE, 1);
            FUN_1013ef30(RS_SRCBLEND, BLEND_ONE);
            FUN_1013ef30(RS_DESTBLEND, BLEND_ONE);
        }
        return previousMode;
    }
    case RENDER_CULL_MODE:
        if (value == CULL_NONE)
        {
            FUN_1013ef30(RS_CULLMODE, CULL_NONE_D3D);
            return 1;
        }
        if (value == CULL_DEFAULT)
        {
            FUN_1013ef30(RS_CULLMODE, DAT_10f65520 != 0 ? CULL_CW_D3D : CULL_CCW_D3D);
            return 1;
        }
        if (value == CULL_REVERSED)
        {
            // value is D3DCULL_CW here
            FUN_1013ef30(RS_CULLMODE, (DAT_10f65520 != 0) + value);
            return 1;
        }
        break;
    case RENDER_TEXTURE_ADDRESS:
        if (value == 1)
        {
            FUN_1013f000(0, SAMP_ADDRESSU, ADDRESS_CLAMP);
            FUN_1013f000(0, SAMP_ADDRESSV, ADDRESS_CLAMP);
            return 1;
        }
        if (value == 2)
        {
            FUN_1013f000(0, SAMP_ADDRESSU, ADDRESS_MIRROR);
            FUN_1013f000(0, SAMP_ADDRESSV, ADDRESS_MIRROR);
            return 1;
        }
        FUN_1013f000(0, SAMP_ADDRESSU, ADDRESS_WRAP);
        FUN_1013f000(0, SAMP_ADDRESSV, ADDRESS_WRAP);
        return 1;
    case RENDER_ALPHA_TEST:
        if (value != 0)
        {
            FUN_1013ef30(RS_ALPHATESTENABLE, 1);
            FUN_1013ef30(RS_ALPHAREF, extra);
            FUN_1013ef30(RS_ALPHAFUNC, CMPFUNC_GREATEREQUAL_ALPHA);
        }
        else
            FUN_1013ef30(RS_ALPHATESTENABLE, 0);
        break;
    case RENDER_CLEAR:
        DAT_10f1fcf0->Clear(0, 0, CLEAR_TARGET_AND_ZBUFFER, value, OPAQUE_DEPTH, 0);
        return 1;
    case RENDER_CLEAR_DEPTH:
        DAT_10f1fcf0->Clear(0, 0, CLEAR_ZBUFFER, 0, (float)value / DEPTH_FIXED_ONE, 0);
        return 1;
    case RENDER_CLEAR_RECT_128:
        {
            D3DRECT_ rect;
            rect.x1 = 0;
            rect.y1 = 0;
            rect.x2 = CLEAR_SQUARE_128;
            rect.y2 = CLEAR_SQUARE_128;
            DAT_10f1fcf0->Clear(1, &rect, CLEAR_TARGET_AND_ZBUFFER, value, OPAQUE_DEPTH, 0);
            return 1;
        }
    case RENDER_CLEAR_STRIP_48:
        {
            D3DRECT_ rect;
            rect.x1 = 0;
            rect.y1 = 0;
            rect.x2 = CLEAR_STRIP_WIDTH;
            rect.y2 = CLEAR_STRIP_HEIGHT_48;
            DAT_10f1fcf0->Clear(1, &rect, CLEAR_TARGET_AND_ZBUFFER, value, OPAQUE_DEPTH, 0);
            return 1;
        }
    case RENDER_CLEAR_STRIP_64:
        {
            D3DRECT_ rect;
            rect.x1 = 0;
            rect.y1 = 0;
            rect.x2 = CLEAR_STRIP_WIDTH;
            rect.y2 = CLEAR_STRIP_HEIGHT_64;
            DAT_10f1fcf0->Clear(1, &rect, CLEAR_TARGET_AND_ZBUFFER, value, OPAQUE_DEPTH, 0);
            return 1;
        }
    case RENDER_CLEAR_STRIP_128:
        {
            D3DRECT_ rect;
            rect.x1 = 0;
            rect.y1 = 0;
            rect.x2 = CLEAR_STRIP_WIDTH;
            rect.y2 = CLEAR_STRIP_HEIGHT_128;
            DAT_10f1fcf0->Clear(1, &rect, CLEAR_TARGET_AND_ZBUFFER, value, OPAQUE_DEPTH, 0);
            return 1;
        }
    case RENDER_CLEAR_STRIP_256:
        {
            D3DRECT_ rect;
            rect.x1 = 0;
            rect.y1 = 0;
            rect.x2 = CLEAR_STRIP_WIDTH;
            rect.y2 = CLEAR_STRIP_HEIGHT_256;
            DAT_10f1fcf0->Clear(1, &rect, CLEAR_TARGET_AND_ZBUFFER, value, OPAQUE_DEPTH, 0);
            return 1;
        }
    case RENDER_CLEAR_RECT_3:
        {
            D3DRECT_ rect;
            rect.x1 = 0;
            rect.y1 = 0;
            rect.x2 = CLEAR_SQUARE_SMALL;
            rect.y2 = CLEAR_SQUARE_SMALL;
            DAT_10f1fcf0->Clear(1, &rect, CLEAR_TARGET_AND_ZBUFFER, value, OPAQUE_DEPTH, 0);
            return 1;
        }
    case RENDER_CLEAR_RECT_512:
        {
            D3DRECT_ rect;
            rect.x1 = 0;
            rect.y1 = 0;
            rect.x2 = CLEAR_SQUARE_512;
            rect.y2 = CLEAR_SQUARE_512;
            DAT_10f1fcf0->Clear(1, &rect, CLEAR_TARGET_AND_ZBUFFER, value, OPAQUE_DEPTH, 0);
            return 1;
        }
    case RENDER_FLUSH_SCENE:
        FUN_1013ed60();
    case RENDER_BEGIN_SCENE:
        FUN_1013ed20();
        return 1;
    case RENDER_END_SCENE:
        FUN_1013ed60();
        return 1;
    case RENDER_TEXTURE_FILTER:
        if (value != 0)
        {
            FUN_1013f000(0, SAMP_MAGFILTER, FILTER_LINEAR);
            FUN_1013f000(0, SAMP_MINFILTER, FILTER_LINEAR);
            FUN_1013f000(1, SAMP_MAGFILTER, FILTER_LINEAR);
            FUN_1013f000(1, SAMP_MINFILTER, FILTER_LINEAR);
            return 1;
        }
        FUN_1013f000(0, SAMP_MAGFILTER, FILTER_POINT);
        FUN_1013f000(0, SAMP_MINFILTER, FILTER_POINT);
        FUN_1013f000(1, SAMP_MAGFILTER, FILTER_POINT);
        FUN_1013f000(1, SAMP_MINFILTER, FILTER_POINT);
        return 1;
    case RENDER_DEPTH_FUNC_ALWAYS:
        if (value != 0)
            FUN_1013ef30(RS_ZFUNC, CMP_ALWAYS);
        else
            FUN_1013ef30(RS_ZFUNC, CMP_LESSEQUAL);
        break;
    case RENDER_DEPTH_WRITE:
        if (value != 0)
        {
            FUN_1013ef30(RS_ZWRITEENABLE, 1);
            return 1;
        }
        FUN_1013ef30(RS_ZWRITEENABLE, 0);
        return 1;
    case RENDER_DEPTH_TEST:
        if (value != 0)
        {
            FUN_1013ef30(RS_ZENABLE, 1);
            return 1;
        }
        FUN_1013ef30(RS_ZENABLE, 0);
        return 1;
    case RENDER_FOG:
        FUN_1013ef30(RS_FOGENABLE, value);
        if (value != 0)
            FUN_1013ef30(RS_FOGCOLOR, extra);
        FUN_1013ef30(RS_FOGVERTEXMODE, 0);
        FUN_1013ef30(RS_FOGTABLEMODE, 0);
        break;
    }
    return 1;
}
