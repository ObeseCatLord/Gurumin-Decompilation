// Shared texture files, model files and playback slot offsets (EN build). Ported from the matched JP source
// decomp/src/mesh_cache.cpp.
#include "anim_model.h"
#include "mesh_cache.h"
#include <stdio.h>
#include <string.h>

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

// ModelNames_FindFree: the first free entry of the model file name table (an empty name), or -1. Called when a node
// takes a new model file slot.
extern "C" int ModelNames_FindFree(void)
{
    for (int i = 0; i < MODEL_FILE_COUNT; i++)
    {
        if (g_modelFileNames[i][0] == 0)
            return i;
    }
    return -1;
}

// ModelNames_SetJoined: stores folder + name (cut to FILE_NAME_SIZE - 1 characters) as entry `index` of the model file
// name table. Returns 0. Called when a node takes a new model file slot.
extern "C" int ModelNames_SetJoined(int index, const char *folder, const char *name)
{
    char joined[MODEL_NAME_SCRATCH_SIZE];
    strcpy(joined, folder);
    strcat(joined, name);
    joined[FILE_NAME_SIZE - 1] = 0;
    strcpy(g_modelFileNames[index], joined);
    return 0;
}

// ModelNames_Find: looks up folder + name (cut to FILE_NAME_SIZE - 1 characters) in the model file name table. Returns
// its index when it is there. Otherwise the name is stored in the first free entry (its reference count reset), that
// index goes to *freeSlot and -1 is returned. A full table is fatal: the original calls fread with null arguments on
// purpose to crash. Called when a node copies a mesh.
extern "C" int ModelNames_Find(const char *folder, const char *name, int *freeSlot)
{
    char joined[MODEL_NAME_SCRATCH_SIZE];
    strcpy(joined, folder);
    strcat(joined, name);
    joined[FILE_NAME_SIZE - 1] = 0;
    *freeSlot = -1;
    for (int i = 0; i < MODEL_FILE_COUNT; i++)
    {
        if (g_modelFileNames[i][0] != 0)
        {
            if (strcmp(joined, g_modelFileNames[i]) == 0)
                return i;
        }
        else if (*freeSlot == -1)
        {
            g_modelFileRefCount[i] = 0;
            *freeSlot = i;
        }
    }
    if (*freeSlot == -1)
        fread(0, 1, 1, 0);
    strcpy(g_modelFileNames[*freeSlot], joined);
    return -1;
}

// MeshCache_AddRef: adds a reference to the shared model file of `mesh` and returns the new count. Called when a node
// shares a cached mesh.
extern "C" int MeshCache_AddRef(const SlotTable *mesh)
{
    int index = mesh->bufferSlot;
    g_modelFileRefCount[index]++;
    return g_modelFileRefCount[index];
}
