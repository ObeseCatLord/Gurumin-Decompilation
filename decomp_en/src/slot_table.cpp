// Slot table materials and texture unbinding (EN build). Ported from the matched JP source
// decomp/src/engine_10143.cpp.
#include "slot_table.h"
#include "render.h"
#include "buffer_pool.h"
#include "engine_memory.h"

// Limits later operations to entries first..last-1. Called when a model part is selected.
int SlotTable::SetRange(int newFirst, int newLast)
{
    first = newFirst;
    last = newLast;
    return 1;
}

// Sets the material of entry `index` (when in range).
int SlotTable::SetMaterial(int index, int value)
{
    if (index < count && index < last)
        material[index] = value;
    return 0;
}

// Replaces the material of every entry in range whose current or original material is `key` (or of all entries
// when key is MATERIAL_ANY) with `value`. Node::ReplaceMaterial runs it over a whole model (texture swaps, e.g.
// costume colours).
int SlotTable::ReplaceMaterial(int key, int value)
{
    for (int i = first; i < count && i < last; i++)
    {
        if (key == MATERIAL_ANY || material[i] == key || originalMaterial[i] == key)
            material[i] = value;
    }
    return 0;
}

// Frees the mesh's texture slot and every table it owns, then clears the pointers. Called by Node::ReleaseMesh once
// the last node using the mesh lets go of it.
void SlotTable::Release()
{
    if (textureSlot >= 0)
        Texture_ReleaseSlot(textureSlot);
    textureSlot = -1;
    if (materialIds != 0)
        g_engineFree(materialIds);
    if (material != 0)
        g_engineFree(material);
    if (originalMaterial != 0)
        g_engineFree(originalMaterial);
    if (secondaryTextures != 0)
        g_engineFree(secondaryTextures);
    if (doubleSide != 0)
        g_engineFree(doubleSide);
    if (vertexStarts != 0)
        g_engineFree(vertexStarts);
    if (vertexEnds != 0)
        g_engineFree(vertexEnds);
    if (indexStarts != 0)
        g_engineFree(indexStarts);
    if (indexEnds != 0)
        g_engineFree(indexEnds);
    if (pointFlags != 0)
        g_engineFree(pointFlags);
    if (points != 0)
        g_engineFree(points);
    if (boxes != 0)
        g_engineFree(boxes);
    count = 0;
    secondaryTextures = 0;
    materialIds = 0;
    material = 0;
    originalMaterial = 0;
    doubleSide = 0;
    vertexStarts = 0;
    vertexEnds = 0;
    indexStarts = 0;
    indexEnds = 0;
    pointFlags = 0;
    points = 0;
    boxes = 0;
}

// Render_UnbindTexture0: unbinds texture stage 0 (only when something is bound).
extern "C" void Render_UnbindTexture0(void)
{
    if (g_stage0Texture != TEXTURE_BINDING_UNKNOWN)
    {
        g_d3dDevice->SetTexture(0, 0);
        g_stage0Texture = TEXTURE_BINDING_UNKNOWN;
    }
}

// Render_UnbindTexture1: unbinds texture stage 1 (only when something is bound).
extern "C" void Render_UnbindTexture1(void)
{
    Stage1_Unbind();
}

// Render_SetTextureStageState: sets a texture stage state unless the cache says it already has that value.
// Returns 1 when the state was (re)written.
extern "C" int Render_SetTextureStageState(unsigned long stage, unsigned long type, unsigned long value)
{
    return StageState_Set(stage, type, value);
}

// SlotTable::Reset: puts a freshly allocated table in its empty state: no texture slot (-1), no entries and every
// table pointer cleared. Returns 0. Called by the node mesh code after it allocates a table.
int SlotTable::Reset()
{
    textureSlot = -1;
    count = 0;
    materialIds = 0;
    material = 0;
    originalMaterial = 0;
    secondaryTextures = 0;
    doubleSide = 0;
    vertexStarts = 0;
    vertexEnds = 0;
    indexStarts = 0;
    indexEnds = 0;
    pointFlags = 0;
    points = 0;
    boxes = 0;
    built = 0;
    return 0;
}
