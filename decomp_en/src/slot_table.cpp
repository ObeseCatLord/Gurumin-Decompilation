// Slot table materials and texture unbinding (EN build). Ported from the matched JP source
// decomp/src/engine_10143.cpp.
#include "slot_table.h"
#include "render.h"

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
    if (g_stage1Texture != TEXTURE_BINDING_UNKNOWN)
    {
        g_d3dDevice->SetTexture(1, 0);
        g_stage1Texture = TEXTURE_BINDING_UNKNOWN;
    }
}

// Render_SetTextureStageState: sets a texture stage state unless the cache says it already has that value.
// Returns 1 when the state was (re)written.
extern "C" int Render_SetTextureStageState(unsigned long stage, unsigned long type, unsigned long value)
{
    if (g_textureStageStateCache[stage][type] == value)
        return 0;
    unsigned long current;
    g_d3dDevice->GetTextureStageState(stage, (D3DTEXTURESTAGESTATETYPE)type, &current);
    if (current != value)
        g_d3dDevice->SetTextureStageState(stage, (D3DTEXTURESTAGESTATETYPE)type, value);
    g_textureStageStateCache[stage][type] = value;
    return 1;
}
