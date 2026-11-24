// EN texture slots, the 2D triangle batch and renderer shutdown (JP twins: texture_mgr.cpp, tri_batch.cpp).

#include <d3d9.h>

// Keep out-of-line copies of TriangleBatch::SetBlend, AddQuad and TextureObject::UpdateImage here (see texture_mgr.h).
#define TEXTURE_MGR_OUT_OF_LINE
#include "render.h"
#include "texture_mgr.h"

// Unloads all textures and shuts the renderer down. Called when the game exits.
void TextureManager::UnloadAndShutdown()
{
    UnloadAllTextures();
    Shutdown();
}

// Empties the batch and forgets its texture.
int TriangleBatch::Reset()
{
    count = 0;
    blendMode = BLEND_MODE_UNSET;
    texture = 0;
    return 1;
}

// Queues `count` vertices for `texture`, flushing first when the texture changes or the buffer is full.
int TriangleBatch::AddVertices(TextureObject *texture, void *source, int count)
{
    g_pendingBatches++;
    if (this->texture != 0 && this->texture != texture)
        Flush();
    this->texture = texture;
    if (this->count + count >= capacity)
        Flush();
    AppendVertices((const ScreenVertex *)source, count);
    return 0;
}

// Draws the queued triangles with the batch's texture and blend mode, then empties the batch. The render state is
// switched to the batch's mode for the draw and back to 0 afterwards.
int TriangleBatch::Flush()
{
    if (count >= TRIANGLE_VERTICES)
    {
        if (texture != 0)
        {
            g_pendingBatches--;
            Render_Command(RENDER_BLEND_MODE, blendMode, 0);
            texture->DrawTriangles(vertices, count / TRIANGLE_VERTICES);
            Render_Command(RENDER_BLEND_MODE, 0, 0);
        }
        count = 0;
    }
    return 0;
}

// Draws the queued triangles with `texture` (instead of the batch's own); nothing happens with fewer than three
// vertices or without a texture. Returns the draw call's result, or 0.
int TriangleBatch::DrawWith(TextureObject *texture)
{
    if (count < TRIANGLE_VERTICES)
        return 0;
    if (texture != 0)
        return texture->DrawTriangles(vertices, count / TRIANGLE_VERTICES);
    return 0;
}

// After a map change: unloads every texture that was not marked as kept and has no generated alpha.
extern "C" int Texture_ReloadMissing(void)
{
    for (int i = 0; i < TEXTURE_SLOT_COUNT; i++)
    {
        if (g_textureNames[i][0] != 0 && g_textureKept[i] == 0 && g_textures[i].generatedAlpha == 0)
            Texture_Release(i, 1);
    }
    return 0;
}

// Releases one reference to texture slot `slot`. The texture is unloaded (and its name cleared, freeing the slot)
// when the count reaches zero or when `force` is set. Returns the remaining count, or -1 for an unused slot.
extern "C" int Texture_Release(int slot, int force)
{
    return TextureSlot_Release(slot, force);
}

// Binds this texture to stage 0 unless it is bound there already. Pooled textures fetch their frame from the buffer
// pool first. Returns 1.
int TextureObject::BindStage0()
{
    if (g_stage0Texture != this)
    {
        g_textureBindCount++;
        if (poolSlot != 0)
        {
            IDirect3DBaseTexture9 *pooled = TexturePool_Get(poolIndex, texture, width, height, poolSlot);
            g_d3dDevice->SetTexture(0, pooled);
            g_stage0Texture = this;
            return 1;
        }
        g_d3dDevice->SetTexture(0, texture);
        g_stage0Texture = this;
    }
    return 1;
}

// Binds this texture to stage 1 (multi-texturing) unless it is bound there already. Returns 1.
int TextureObject::BindStage1()
{
    if (g_stage1Texture != this)
    {
        g_textureBindCount++;
        if (poolSlot != 0)
        {
            IDirect3DBaseTexture9 *pooled = TexturePool_Get(poolIndex, texture, width, height, poolSlot);
            g_d3dDevice->SetTexture(1, pooled);
            g_stage1Texture = this;
            return 1;
        }
        g_d3dDevice->SetTexture(1, texture);
        g_stage1Texture = this;
    }
    return 1;
}

// Texture_BindStage1: binds texture slot `index` to stage 1 (a negative index unbinds). Returns the index or -1.
extern "C" int Texture_BindStage1(int index)
{
    if (index < 0)
    {
        Render_UnbindTexture1();
        return -1;
    }
    if (g_textureNames[index][0] != 0)
        g_textures[index].BindStage1();
    return index;
}

// Texture_BindStage0: binds texture slot `index` to stage 0 (a negative index unbinds). Returns the index or -1.
extern "C" int Texture_BindStage0(int index)
{
    if (index < 0)
    {
        Render_UnbindTexture0();
        return -1;
    }
    if (g_textureNames[index][0] != 0)
        g_textures[index].BindStage0();
    return index;
}

// Textures_ReleaseGenerated: releases the Direct3D data of every texture whose alpha is generated, so
// the device can be reset (they are rebuilt afterwards). Returns 1. Called while recovering from a lost device.
extern "C" int Textures_ReleaseGenerated(void)
{
    for (int i = 0; i < TEXTURE_SLOT_COUNT; i++)
    {
        if (g_textureNames[i][0] != 0 && g_textures[i].generatedAlpha != 0)
            g_textures[i].Release();
    }
    return 1;
}
