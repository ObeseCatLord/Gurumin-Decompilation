// EN texture binding, writing and drawing (JP twin: texture_bind.cpp).

#include <d3d9.h>

#include "engine_memory.h"
#include "vec4.h"
#include "render.h"
#include "texture_mgr.h"
#include "texture_pool.h"

// Flag for LockRect: do not take the system lock (D3DLOCK_NOSYSLOCK).
const DWORD LOCK_NO_SYSTEM_LOCK = 0x800;

// Forgets any stage binding that still points at this texture (the next bind then calls SetTexture again).
// Inline in the original; used by the write functions after they changed the pixels.
static void ForgetStageBindings(TextureObject *object)
{
    if (g_stage0Texture == object)
        g_stage0Texture = TEXTURE_BINDING_UNKNOWN;
    if (g_stage1Texture == object)
        g_stage1Texture = TEXTURE_BINDING_UNKNOWN;
}

// Copies 16-bit pixels into the texture: rows y..rows-1 of `rowPixels` pixels each from `bits` (`pitch` bytes per
// source row; `x` is not used). Drops the pooled copy and forgets stage bindings of this texture so the new pixels
// are used. Returns 0 when the texture cannot be locked, else 1. Called by the 2D image loaders.
int TextureObject::Write16(int x, int y, int rowPixels, int rows, int pitch, const void *bits)
{
    D3DLOCKED_RECT locked;
    if (texture == 0)
        return 0;
    if (texture->LockRect(0, &locked, 0, LOCK_NO_SYSTEM_LOCK) != 0)
        return 0;
    char *destination = (char *)locked.pBits;
    const char *source = (const char *)bits;
    for (int row = y; row < rows; row++)
    {
        g_engineCopy(destination, source, rowPixels * 2);
        destination += locked.Pitch;
        source += pitch;
    }
    texture->UnlockRect(0);
    TexturePool_DropSurface(poolIndex);
    ForgetStageBindings(this);
    return 1;
}

// Bytes of one source pixel of WriteRgb32 (B, G, R, unused).
const int RGB32_PIXEL_BYTES = 4;
// Source pixels converted per 32-bit store of WriteRgb32.
const int RGB32_PIXELS_PER_STORE = 2;

// Writes 32-bit pixels (B, G, R, unused bytes) into the 16-bit (R5G6B5) texture, rows top..bottom-1, two pixels per
// 32-bit store: the row of `pitch` source bytes becomes pitch / 8 pairs (`left` and `right` are not used). Like
// Write16, it drops the pooled copy and forgets the stage bindings afterwards. Returns 0 when the texture cannot be
// locked, else 1.
int TextureObject::WriteRgb32(int left, int top, int right, int bottom, int pitch, const unsigned char *bits)
{
    D3DLOCKED_RECT locked;
    if (texture == 0)
        return 0;
    if (texture->LockRect(0, &locked, 0, LOCK_NO_SYSTEM_LOCK) != 0)
        return 0;
    const unsigned char *source = bits;
    int pairs = pitch / (RGB32_PIXEL_BYTES * RGB32_PIXELS_PER_STORE);
    char *destination = (char *)locked.pBits;
    for (int y = top; y < bottom; y++)
    {
        for (int i = 0; i < pairs; i++)
        {
            // second pixel in the high half, first pixel in the low half
            *(unsigned int *)destination = ((((((source[6] & 0xf8) << 5 | (source[5] & 0xfc)) << 6 | (source[4] & 0xf8)) << 5 |
                                             (source[2] & 0xf8)) << 5 | (source[1] & 0xfc)) << 3) | (source[0] >> 3);
            destination += 4;
            source += 8;
        }
        destination += locked.Pitch - pairs * 4;
        source += pitch - pairs * 8;
    }
    texture->UnlockRect(0);
    TexturePool_DropSurface(poolIndex);
    ForgetStageBindings(this);
    return 1;
}

// Palette entry value that is replaced by 0 (transparent) when a paletted image is written.
const unsigned short PALETTE_COLOUR_KEY = 0x20;

// Writes 8-bit palette indexes into the 16-bit texture: the rectangle left..right-1, top..bottom-1 of the texture gets
// palette[index] for each source byte (`bits` is the rectangle's first row, `pitch` bytes per row); the palette value
// 0x20 becomes 0. Like Write16, it drops the pooled copy and forgets the stage bindings afterwards. Returns 0 when
// the texture cannot be locked, else 1.
int TextureObject::WritePaletted(int left, int top, int right, int bottom, int pitch, const unsigned char *bits,
                                 const unsigned short *palette)
{
    D3DLOCKED_RECT locked;
    if (texture == 0)
        return 0;
    if (texture->LockRect(0, &locked, 0, LOCK_NO_SYSTEM_LOCK) != 0)
        return 0;
    char *destination = (char *)locked.pBits;
    const unsigned char *source = bits;
    for (int y = top; y < bottom; y++)
    {
        for (int x = left; x < right; x++)
        {
            unsigned short colour = palette[source[x - left]];
            if (colour == PALETTE_COLOUR_KEY)
                colour = 0;
            ((unsigned short *)destination)[x] = colour;
        }
        destination += locked.Pitch;
        source += pitch;
    }
    texture->UnlockRect(0);
    TexturePool_DropSurface(poolIndex);
    ForgetStageBindings(this);
    return 1;
}

// Vertex format of the 2D triangle lists: screen position, diffuse and specular colour, one texture coordinate.
const DWORD SCREEN_VERTEX_FORMAT = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
// Bytes of one screen vertex.
const UINT SCREEN_VERTEX_STRIDE = 0x20;

// Draws `triangles` triangles from `vertices` (screen vertices of 0x20 bytes) with this texture on stage 0: texture
// times vertex colour for both colour and alpha, stages 1 to 3 off. Returns 0 when there is nothing to draw, else 1.
// Called by the triangle batches (TriangleBatch::Flush, DrawWith).
int TextureObject::DrawTriangles(const void *vertices, int triangles)
{
    if (triangles < 1)
        return 0;
    BindStage0();
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(SCREEN_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    StageState_Set(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    Stage1_Unbind();
    g_d3dDevice->SetTexture(2, 0);
    g_d3dDevice->SetTexture(3, 0);
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, triangles, vertices, SCREEN_VERTEX_STRIDE);
    return 1;
}

// Width and height of the 640x480 space the 2D vertices are given in.
const float SCREEN_SPACE_WIDTH = 640.0f;
const float SCREEN_SPACE_HEIGHT = 480.0f;

// Draws a triangle strip of `count` vertices given in 640x480 screen coordinates: the positions are scaled to the
// back-buffer size in place, then drawn like DrawTriangles. Returns 0 when there are fewer than 3 vertices, else 1.
int TextureObject::DrawStrip(ScreenVertex *vertices, int count)
{
    if (count < 3)
        return 0;
    BindStage0();
    Stage1_Unbind();
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(SCREEN_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    StageState_Set(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    float scaleX = (float)g_backBufferWidth / SCREEN_SPACE_WIDTH;
    float scaleY = (float)g_backBufferHeight / SCREEN_SPACE_HEIGHT;
    Stage1_Unbind();
    g_d3dDevice->SetTexture(2, 0);
    g_d3dDevice->SetTexture(3, 0);
    for (int i = 0; i < count; i++)
    {
        vertices[i].x = scaleX * vertices[i].x;
        vertices[i].y = scaleY * vertices[i].y;
    }
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, count - 2, vertices, SCREEN_VERTEX_STRIDE);
    return 1;
}

// Largest value of one colour channel.
const int COLOUR_CHANNEL_MAX = 255;
// Vertex colour of a white vertex without alpha (alpha goes in the top byte).
const unsigned int COLOUR_WHITE_RGB = 0xffffff;
// Specular colour the 2D vertices carry: opaque black.
const unsigned int SPECULAR_OPAQUE_BLACK = 0xff000000;
// Shift of the alpha byte in a vertex colour.
const int COLOUR_ALPHA_SHIFT = 24;
// Half, for centring a rectangle.
const float RECT_HALF = 0.5f;

// Draws the cell u0..u1, v0..v1 of this texture into the screen rectangle left..right, top..bottom (back-buffer pixels,
// no 640x480 scaling) as a two-triangle strip, turned by `angle` radians about the rectangle's centre unless it is 0.
// White at `alpha` (0..1), texture times vertex colour. Returns 1. Called by the menu and HUD drawing.
int TextureObject::DrawCell(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, float z,
                            float alpha, float angle)
{
    ScreenVertex vertices[4];
    BindStage0();
    if (angle != 0.0f)
    {
        Vec4 topLeft;
        Vec4 bottomRight;
        Vec4 topRight;
        Vec4 bottomLeft;
        Vec4 centre;
        centre.x = (float)(left + right) * RECT_HALF;
        centre.y = (float)(top + bottom) * RECT_HALF;
        topLeft.x = (float)left - centre.x;
        topLeft.y = (float)top - centre.y;
        bottomRight.x = (float)right - centre.x;
        bottomRight.y = (float)bottom - centre.y;
        topRight.x = (float)right - centre.x;
        topRight.y = (float)top - centre.y;
        bottomLeft.x = (float)left - centre.x;
        bottomLeft.y = (float)bottom - centre.y;
        topLeft.RotateZ(angle);
        bottomRight.RotateZ(angle);
        topRight.RotateZ(angle);
        bottomLeft.RotateZ(angle);
        topLeft.x += centre.x;
        topLeft.y += centre.y;
        bottomRight.x += centre.x;
        bottomRight.y += centre.y;
        topRight.x += centre.x;
        topRight.y += centre.y;
        bottomLeft.x += centre.x;
        bottomLeft.y += centre.y;
        vertices[0].x = topLeft.x;
        vertices[0].y = topLeft.y;
        vertices[1].x = topRight.x;
        vertices[1].y = topRight.y;
        vertices[2].x = bottomLeft.x;
        vertices[2].y = bottomLeft.y;
        vertices[3].x = bottomRight.x;
        vertices[3].y = bottomRight.y;
    }
    else
    {
        vertices[0].x = (float)left;
        vertices[0].y = (float)top;
        vertices[1].x = (float)right;
        vertices[1].y = (float)top;
        vertices[2].x = (float)left;
        vertices[2].y = (float)bottom;
        vertices[3].x = (float)right;
        vertices[3].y = (float)bottom;
    }
    vertices[0].u = u0;
    vertices[0].v = v0;
    vertices[1].u = u1;
    vertices[1].v = v0;
    vertices[2].u = u0;
    vertices[2].v = v1;
    vertices[3].u = u1;
    vertices[3].v = v1;
    int a = (int)(alpha * (float)COLOUR_CHANNEL_MAX);
    if (a > COLOUR_CHANNEL_MAX)
        a = COLOUR_CHANNEL_MAX;
    else if (a < 0)
        a = 0;
    vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = vertices[3].diffuse =
        a << COLOUR_ALPHA_SHIFT | COLOUR_WHITE_RGB;
    vertices[0].specular = vertices[1].specular = vertices[2].specular = vertices[3].specular = SPECULAR_OPAQUE_BLACK;
    vertices[0].z = vertices[1].z = vertices[2].z = vertices[3].z = z;
    vertices[0].rhw = vertices[1].rhw = vertices[2].rhw = vertices[3].rhw = 1.0f;
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(SCREEN_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    StageState_Set(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, SCREEN_VERTEX_STRIDE);
    return 1;
}

// First texture coordinate of a screen copy: just inside the texture, so the border texel is not blended in.
const float SCREEN_COPY_UV_MIN = 0.001f;

// Draws this whole texture into the rectangle left..right, top..bottom (640x480 screen coordinates, scaled to the back
// buffer) as a two-triangle strip, with counter-clockwise culling. When the texture is wider or taller than the back
// buffer, only the part the size of the back buffer is used (a copy of the screen in a larger texture). White at
// `alpha` (0..1), texture times vertex colour; stage 1 off. Returns 1. Called by the screen fade and blur effects.
int TextureObject::DrawScreenCopy(int left, int top, int right, int bottom, float z, float alpha)
{
    ScreenVertex vertices[4];
    BindStage0();
    float scaleX = (float)g_backBufferWidth / SCREEN_SPACE_WIDTH;
    float scaleY = (float)g_backBufferHeight / SCREEN_SPACE_HEIGHT;
    float uMax = 1.0f;
    float vMax = 1.0f;
    if (g_deviceBackBufferWidth < width)
        uMax = (float)g_deviceBackBufferWidth / (float)width;
    if (g_deviceBackBufferHeight < height)
        vMax = (float)g_deviceBackBufferHeight / (float)height;
    vertices[0].x = (float)left * scaleX;
    vertices[0].y = (float)top * scaleY;
    vertices[1].x = (float)right * scaleX;
    vertices[1].y = (float)top * scaleY;
    vertices[2].x = (float)left * scaleX;
    vertices[2].y = (float)bottom * scaleY;
    vertices[3].x = (float)right * scaleX;
    vertices[3].y = (float)bottom * scaleY;
    vertices[0].u = SCREEN_COPY_UV_MIN;
    vertices[0].v = SCREEN_COPY_UV_MIN;
    vertices[1].u = uMax;
    vertices[1].v = SCREEN_COPY_UV_MIN;
    vertices[2].u = SCREEN_COPY_UV_MIN;
    vertices[2].v = vMax;
    vertices[3].u = uMax;
    vertices[3].v = vMax;
    int a = (int)(alpha * (float)COLOUR_CHANNEL_MAX);
    if (a > COLOUR_CHANNEL_MAX)
        a = COLOUR_CHANNEL_MAX;
    else if (a < 0)
        a = 0;
    vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = vertices[3].diffuse =
        a << COLOUR_ALPHA_SHIFT | COLOUR_WHITE_RGB;
    vertices[0].specular = vertices[1].specular = vertices[2].specular = vertices[3].specular = SPECULAR_OPAQUE_BLACK;
    vertices[0].z = vertices[1].z = vertices[2].z = vertices[3].z = z;
    vertices[0].rhw = vertices[1].rhw = vertices[2].rhw = vertices[3].rhw = 1.0f;
    RenderState_Set(D3DRS_CULLMODE, D3DCULL_CCW);
    RenderState_Set(D3DRS_COLORVERTEX, 1);
    g_d3dDevice->SetFVF(SCREEN_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    StageState_Set(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    StageState_Set(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    StageState_Set(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    StageState_Set(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    StageState_Set(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, SCREEN_VERTEX_STRIDE);
    return 1;
}

// Like DrawCell, but the rectangle is given in 640x480 screen coordinates
// (scaled to the back buffer) and the diffuse colour is `alpha`, `red`, `green`, `blue` (each 0..1); no specular.
// Returns 1.
int TextureObject::DrawColoured(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1,
                                float z, float alpha, float angle, float red, float green, float blue)
{
    ScreenVertex vertices[4];
    BindStage0();
    float scaleX = (float)g_backBufferWidth / SCREEN_SPACE_WIDTH;
    float scaleY = (float)g_backBufferHeight / SCREEN_SPACE_HEIGHT;
    if (angle != 0.0f)
    {
        Vec4 topLeft;
        Vec4 bottomRight;
        Vec4 topRight;
        Vec4 bottomLeft;
        Vec4 centre;
        centre.x = (float)(left + right) * RECT_HALF;
        centre.y = (float)(top + bottom) * RECT_HALF;
        topLeft.x = (float)left - centre.x;
        topLeft.y = (float)top - centre.y;
        bottomRight.x = (float)right - centre.x;
        bottomRight.y = (float)bottom - centre.y;
        topRight.x = (float)right - centre.x;
        topRight.y = (float)top - centre.y;
        bottomLeft.x = (float)left - centre.x;
        bottomLeft.y = (float)bottom - centre.y;
        topLeft.RotateZ(angle);
        bottomRight.RotateZ(angle);
        topRight.RotateZ(angle);
        bottomLeft.RotateZ(angle);
        topLeft.x += centre.x;
        topLeft.y += centre.y;
        bottomRight.x += centre.x;
        bottomRight.y += centre.y;
        topRight.x += centre.x;
        topRight.y += centre.y;
        bottomLeft.x += centre.x;
        bottomLeft.y += centre.y;
        vertices[0].x = topLeft.x * scaleX;
        vertices[0].y = topLeft.y * scaleY;
        vertices[1].x = topRight.x * scaleX;
        vertices[1].y = topRight.y * scaleY;
        vertices[2].x = bottomLeft.x * scaleX;
        vertices[2].y = bottomLeft.y * scaleY;
        vertices[3].x = bottomRight.x * scaleX;
        vertices[3].y = bottomRight.y * scaleY;
    }
    else
    {
        vertices[0].x = (float)left * scaleX;
        vertices[0].y = (float)top * scaleY;
        vertices[1].x = (float)right * scaleX;
        vertices[1].y = (float)top * scaleY;
        vertices[2].x = (float)left * scaleX;
        vertices[2].y = (float)bottom * scaleY;
        vertices[3].x = (float)right * scaleX;
        vertices[3].y = (float)bottom * scaleY;
    }
    vertices[0].u = u0;
    vertices[0].v = v0;
    vertices[1].u = u1;
    vertices[1].v = v0;
    vertices[2].u = u0;
    vertices[2].v = v1;
    vertices[3].u = u1;
    vertices[3].v = v1;
    int a = (int)(alpha * (float)COLOUR_CHANNEL_MAX);
    if (a > COLOUR_CHANNEL_MAX)
        a = COLOUR_CHANNEL_MAX;
    else if (a < 0)
        a = 0;
    // packed as alpha, red, green, blue from the top byte down
    unsigned int colour = (((unsigned int)(red * (float)COLOUR_CHANNEL_MAX) | a << 8) << 8 |
                           (unsigned int)(green * (float)COLOUR_CHANNEL_MAX)) << 8 |
                          (unsigned int)(blue * (float)COLOUR_CHANNEL_MAX);
    vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = vertices[3].diffuse = colour;
    vertices[0].specular = vertices[1].specular = vertices[2].specular = vertices[3].specular = SPECULAR_OPAQUE_BLACK;
    vertices[0].z = vertices[1].z = vertices[2].z = vertices[3].z = z;
    vertices[0].rhw = vertices[1].rhw = vertices[2].rhw = vertices[3].rhw = 1.0f;
    RenderState_Set(D3DRS_COLORVERTEX, 1);                  // D3DRS_COLORVERTEX
    g_d3dDevice->SetFVF(SCREEN_VERTEX_FORMAT);
    g_d3dDevice->SetVertexShader(0);
    RenderState_Set(D3DRS_DIFFUSEMATERIALSOURCE, 1);                  // D3DRS_DIFFUSEMATERIALSOURCE = D3DMCS_COLOR1
    StageState_Set(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);                  // alpha: modulate
    StageState_Set(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);                  //   texture
    StageState_Set(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);                  //   diffuse
    g_drawCallCount++;
    g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, SCREEN_VERTEX_STRIDE);
    return 1;
}

// Releases the Direct3D texture (returning pooled frames to the buffer pool first) and the render-target surface,
// forgets any stage binding that still points at this object, and clears the size and pointers. Returns 0.
// Called by the texture unload functions.
int TextureObject::Release()
{
    if (texture != 0)
    {
        if (poolSlot != 0)
            TexCache_Free(poolIndex, width, height);
        texture->Release();
    }
    if (surface != 0)
        surface->Release();
    ForgetStageBindings(this);
    width = 0;
    height = 0;
    poolSlot = 0;
    texture = 0;
    surface = 0;
    return 0;
}
