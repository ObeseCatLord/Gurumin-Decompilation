// Copying a mesh table (EN build, EN-only: no JP twin found).
#include "slot_table.h"

// Makes this table share the buffers and tables of `source`: every field is copied except the double-sided word table,
// the built flag and the buffer slot. Returns 0 (copying nothing) when `source` is null, else 1. No direct caller.
int SlotTable::CopyFrom(const SlotTable *source)
{
    if (source == 0)
        return 0;
    vertexStarts = source->vertexStarts;
    vertexEnds = source->vertexEnds;
    indexStarts = source->indexStarts;
    indexEnds = source->indexEnds;
    pointFlags = source->pointFlags;
    points = source->points;
    boxes = source->boxes;
    first = source->first;
    textureSlot = source->textureSlot;
    vertexCount = source->vertexCount;
    indexCount = source->indexCount;
    materialIds = source->materialIds;
    originalMaterial = source->originalMaterial;
    secondaryTextures = source->secondaryTextures;
    material = source->material;
    last = source->last;
    count = source->count;
    return 1;
}
