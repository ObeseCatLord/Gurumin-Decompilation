// Shared texture files, model files and playback slot offsets (EN build). Ported from the matched JP source
// decomp/src/mesh_cache.cpp.
#include "anim_model.h"
#include "mesh_cache.h"

// TextureFile_Release: releases one reference to a shared texture file (freeing the name at 0). Returns the remaining
// count. Called when a node's texture reference is dropped.
extern "C" int TextureFile_Release(const TextureRef *texture)
{
    int index = texture->slot;
    if (index < 0)
        return 0;
    if (--g_textureFileRefCount[index] <= 0)
    {
        g_textureFileNames[index][0] = 0;
        g_textureFileRefCount[index] = 0;
    }
    return g_textureFileRefCount[index];
}

// Playback::SetSlotOffset: sets the (x, y) offset of slot `slot` in layer `layer`. Returns 0 for a layer or slot out
// of range.
int Playback::SetSlotOffset(int layer, int slot, int x, int y)
{
    if (layer > PLAYBACK_LAYER_COUNT - 1)
        return 0;
    if (slot > PLAYBACK_SLOT_COUNT - 1)
        return 0;
    slotOffsetX[layer][slot] = x;
    slotOffsetY[layer][slot] = y;
    return 1;
}

// MeshCache_Release: releases one reference to the shared model file a mesh was loaded from; the name entry is freed
// when the count reaches 0. Returns the remaining count. Called when a node's mesh is dropped.
extern "C" int MeshCache_Release(const SlotTable *mesh)
{
    int index = mesh->bufferSlot;
    if (index < 0)
        return 0;
    if (--g_modelFileRefCount[index] <= 0)
    {
        g_modelFileNames[index][0] = 0;
        g_modelFileRefCount[index] = 0;
    }
    return g_modelFileRefCount[index];
}
