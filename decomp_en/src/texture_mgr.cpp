// EN texture slots, the 2D triangle batch and renderer shutdown (JP twins: texture_mgr.cpp, tri_batch.cpp).

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
    texture = 0;
    blendMode = BLEND_MODE_UNSET;
    return 1;
}

// Switches the batch to blend mode `mode`, flushing the queued triangles when the mode changes.
int TriangleBatch::SetBlend(int mode, int unused)
{
    if (blendMode != BLEND_MODE_UNSET)
    {
        if (blendMode != mode)
            Flush();
        blendMode = mode;
        return 0;
    }
    blendMode = mode;
    return 0;
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
    if (slot == -1 || g_textureNames[slot][0] == 0)
        return -1;
    char refs = --g_textureRefCount[slot];
    if (force != 0 || refs == 0)
    {
        g_textureRefCount[slot] = 0;
        g_textures[slot].Unload();
        g_textureNames[slot][0] = 0;
    }
    return g_textureRefCount[slot];
}
