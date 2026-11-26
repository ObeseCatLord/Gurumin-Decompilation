// EN key tracks, key banks, vertex packs and index lists (JP twins: containers.cpp).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "containers.h"
#include "key_bank.h"
#include "index_list.h"
#include "motion_list.h"
#include "engine_memory.h"

// Size of one sphere entry of a sphere list (centre and radius as a Vec4).
const int SPHERE_SIZE = sizeof(Vec4);

// Marks every key as a motion frame key. Called by the animation loaders.
int KeyTrack::MarkAll()
{
    for (int i = 0; i < count; i++)
        flags[i] = KEY_FLAG_MOTION;
    return 0;
}

// 1 when the track has a key exactly at `frame` (the keys are sorted, so the search stops early).
int KeyTrack::HasFrame(int frame)
{
    for (int i = 0; i < count; i++)
    {
        if (frames[i] == frame)
            return 1;
        if (frames[i] > frame)
            return 0;
    }
    return 0;
}

// Frees the three arrays and empties the track (the arrays come from the C heap, unlike the engine allocator's).
void KeyTrack::Free()
{
    if (capacity != 0)
    {
        free(frames);
        frames = 0;
        free(values);
        values = 0;
        free(flags);
        flags = 0;
    }
    capacity = 0;
    count = 0;
}

// Frees the vertex buffer; the count stays. Called while a node releases its packs. Returns 1.
int VertexPack::FreeVertices()
{
    if (vertices != 0)
        g_engineFree(vertices);
    vertices = 0;
    return 1;
}

// Frees the index buffer and empties the list. Called while a node releases its index lists. Returns 1.
int IndexList::FreeIndices()
{
    if (indices != 0)
        g_engineFree(indices);
    indices = 0;
    count = 0;
    return 1;
}

// Frees the sphere buffer and empties the list. Called while a node releases its triangle spheres. Returns 1.
int IndexList::FreeSpheres()
{
    if (spheres != 0)
        g_engineFree(spheres);
    spheres = 0;
    count = 0;
    return 1;
}

// Reallocates the vertex buffer for `count` vertices. Returns 1.
int VertexPack::SetVertexCount(int count)
{
    if (vertices != 0)
        g_engineFree(vertices);
    vertices = 0;
    void *buffer = g_engineAlloc(count * VERTEX_SIZE, "");
    vertices = (Vertex *)buffer;
    vertexCount = count;
    return 1;
}

// Reallocates the index buffer for `newCount` indexes (contents are not kept). Returns 1.
int IndexList::Resize(int newCount)
{
    if (indices != 0)
        g_engineFree(indices);
    indices = 0;
    int *buffer = (int *)g_engineAlloc(newCount * sizeof(int), "");
    indices = buffer;
    count = newCount;
    return 1;
}

// Reallocates the span buffer for `newCount` spheres (contents are not kept). The allocation is tagged
// "m_indexSP(count)" for the engine's memory report. Returns 1.
int IndexList::ResizeSpans(int newCount)
{
    char tag[64];
    if (spheres != 0)
        g_engineFree(spheres);
    spheres = 0;
    sprintf(tag, "m_indexSP(%d)", newCount);
    void *buffer = g_engineAlloc(newCount * SPHERE_SIZE, tag);
    spheres = (Vec4 *)buffer;
    count = newCount;
    return 1;
}

// Clears the fields of a pack that lives in raw memory (no constructor runs for it). Called by the node copy code.
void VertexPack::Init()
{
    vertices = 0;
    center[0] = 0;
    center[1] = 0;
    center[2] = 0;
    radius = 0;
    vertexCount = 0;
    m_0040 = 0;
    m_0044 = 0;
}

// Copies the vertices, bounds and the other pack fields of `source` into this pack. Returns 0 (leaving the pack empty)
// for a missing or empty source, else 1.
int VertexPack::CopyFrom(const VertexPack *source)
{
    vertexCount = 0;
    if (source == 0)
        return 0;
    if (source->vertexCount == 0)
        return 0;
    SetVertexCount(source->vertexCount);
    boundsMin.x = source->boundsMin.x;
    boundsMin.y = source->boundsMin.y;
    boundsMin.z = source->boundsMin.z;
    boundsMin.w = source->boundsMin.w;
    boundsMax.x = source->boundsMax.x;
    boundsMax.y = source->boundsMax.y;
    boundsMax.z = source->boundsMax.z;
    boundsMax.w = source->boundsMax.w;
    center[0] = source->center[0];
    center[1] = source->center[1];
    center[2] = source->center[2];
    radius = source->radius;
    vertexCount = source->vertexCount;
    m_0010 = source->m_0010;
    m_0018 = source->m_0018;
    material = source->material;
    m_0040 = source->m_0040;
    m_0044 = source->m_0044;
    g_engineCopy(vertices, source->vertices, vertexCount * VERTEX_SIZE);
    return 1;
}

// Clears the buffer pointer of a list that lives in raw memory (no constructor runs for it).
void IndexList::Init()
{
    indices = 0;
}

// Copies the indices of `source` into this list. Returns 0 (leaving the list empty) for a missing or empty source,
// else 1.
int IndexList::CopyFrom(const IndexList *source)
{
    count = 0;
    if (source == 0)
        return 0;
    if (source->count == 0)
        return 0;
    Resize(source->count);
    memcpy(indices, source->indices, source->count * sizeof(int));
    return 1;
}

// Keys a track grows by when it runs out of room (also its initial room).
const int KEY_TRACK_GROWTH = 16;

// Empties the track and gives it room for KEY_TRACK_GROWTH keys, one of them the zero key at frame 0. Called before a
// track is filled from another one.
void KeyTrack::Init()
{
    capacity = 0;
    Reserve(KEY_TRACK_GROWTH);
    count = 0;
    frames[0] = 0;
    Vec4 *firstValue = values;
    firstValue->x = 0;
    firstValue->y = 0;
    firstValue->z = 0;
    firstValue->w = 0;
}

// Inserts a key (x, y, z, w) at `frame` in frame order, in front of any key already at that frame, growing the arrays
// when needed. Returns the index of the new key.
int KeyTrack::Insert(int frame, float x, float y, float z, float w)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (frames[i] == frame)
            break;
        if (frames[i] > frame)
            break;
    }
    for (int j = count - 1; j >= i; j--)
    {
        frames[j + 1] = frames[j];
        Vec4 *keyValues = values;
        keyValues[j + 1].x = keyValues[j].x;
        keyValues[j + 1].y = keyValues[j].y;
        keyValues[j + 1].z = keyValues[j].z;
        keyValues[j + 1].w = keyValues[j].w;
        flags[j + 1] = flags[j];
    }
    if (count + KEY_TRACK_GROWTH > capacity)
        Reserve(KEY_TRACK_GROWTH);
    frames[i] = frame;
    values[i].x = x;
    values[i].y = y;
    values[i].z = z;
    values[i].w = w;
    flags[i] = 0;
    count++;
    if (count + KEY_TRACK_GROWTH > capacity)
        Reserve(KEY_TRACK_GROWTH);
    return i;
}

// Sets the key at `frame` to (x, y, z, w): an existing key at that frame gets the new value (returns 1); otherwise a
// new key is inserted in frame order, growing the arrays when needed, and its index is returned.
int KeyTrack::Set(int frame, float x, float y, float z, float w)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (frames[i] == frame)
        {
            values[i].x = x;
            values[i].y = y;
            values[i].z = z;
            values[i].w = w;
            return 1;
        }
        if (frames[i] > frame)
            break;
    }
    for (int j = count - 1; j >= i; j--)
    {
        frames[j + 1] = frames[j];
        values[j + 1].x = values[j].x;
        values[j + 1].y = values[j].y;
        values[j + 1].z = values[j].z;
        values[j + 1].w = values[j].w;
        flags[j + 1] = flags[j];
    }
    if (count + KEY_TRACK_GROWTH > capacity)
        Reserve(KEY_TRACK_GROWTH);
    frames[i] = frame;
    values[i].x = x;
    values[i].y = y;
    values[i].z = z;
    values[i].w = w;
    flags[i] = 0;
    count++;
    if (count + KEY_TRACK_GROWTH > capacity)
        Reserve(KEY_TRACK_GROWTH);
    return i;
}

// Makes this track a copy of `source`: its keys, values and flags. Returns 1.
int KeyTrack::CopyFrom(const KeyTrack *source)
{
    Free();
    Init();
    for (int i = 0; i < source->count; i++)
    {
        Set(source->frames[i], source->values[i].x, source->values[i].y, source->values[i].z, source->values[i].w);
        flags[i] = source->flags[i];
    }
    return 1;
}

// Copies the bank's keys into `target` (a growable track). Returns 0.
int KeyBank::CopyTo(KeyTrack *target)
{
    target->Free();
    target->Init();
    for (int i = 0; i < count; i++)
    {
        target->Set(frames[i], values[i].x, values[i].y, values[i].z, values[i].w);
        target->flags[i] = flags[i];
    }
    return 0;
}

// Largest entry count of a MotionList: the highest index AddEntry accepts.
const int MOTION_LAST_ENTRY = MOTION_ENTRY_COUNT - 1;

// Adds an entry (`id`, `value`, `name` cut to the entry size, `extra`) to the list. Returns 0 when all entries are
// used, else 1.
int MotionList::AddEntry(int id, int value, const char *name, int extra)
{
    if (entryCount > MOTION_LAST_ENTRY)
        return 0;
    ids[entryCount] = id;
    values[entryCount] = value;
    strncpy(names[entryCount], name, MOTION_NAME_SIZE - 1);
    names[entryCount][MOTION_NAME_SIZE - 1] = 0;
    extras[entryCount] = extra;
    entryCount++;
    return 1;
}

// Sets the identity matrix.
void Matrix4::SetIdentity()
{
    m[12] = m[13] = m[14] = m[8] = m[9] = m[11] = m[4] = m[6] = m[7] = m[1] = m[2] = m[3] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// MotionList::FrameAt: the frame shown `time` frames after m_0208 (never below 0). Without MOTION_LIST_PLAYS_ITEMS in
// m_0004 that is just the elapsed time. Otherwise the list plays its items in turn: each item is a named entry whose
// ids..values range is its first and end frame and whose extra bit MOTION_EXTRA_BACKWARDS plays it backwards. Stores
// the item number in *outItem and the entry in *outEntry (when given). Past the end the last item repeats, or with
// MOTION_EXTRA_HOLD_LAST holds its last frame (the backward case tests the extra of entry count - 1 instead of the
// last item's entry). Called by the node animation code.
int MotionList::FrameAt(int time, int *outItem, int *outEntry)
{
    time -= m_0208;
    if (!(m_0004 & MOTION_LIST_PLAYS_ITEMS))
        return time > 0 ? time : 0;
    int start = 0;
    int length;
    for (int i = 0; i < count; i++)
    {
        length = values[items[i]] - ids[items[i]];
        int end = length + start;
        if (time < end)
        {
            if (!(extras[items[i]] & MOTION_EXTRA_BACKWARDS))
            {
                if (outItem != 0)
                    *outItem = i;
                if (outEntry != 0)
                    *outEntry = items[i];
                int frame = ids[items[i]] - start + time;
                return frame > 0 ? frame : 0;
            }
            int back = start + start - time + length;
            if (outItem != 0)
                *outItem = i;
            if (outEntry != 0)
                *outEntry = items[i];
            back += ids[items[i]] - start;
            return back > 0 ? back : 0;
        }
        start = end;
    }
    if (length <= 0)
        return 0;
    if (!(extras[items[count - 1]] & MOTION_EXTRA_BACKWARDS))
    {
        if (outItem != 0)
            *outItem = count - 1;
        if (outEntry != 0)
            *outEntry = items[count - 1];
        if (extras[items[count - 1]] & MOTION_EXTRA_HOLD_LAST)
        {
            int frame = ids[items[count - 1]] + (length - 1) % length;
            return frame > 0 ? frame : 0;
        }
        int frame = ids[items[count - 1]] + (time - start) % length;
        return frame > 0 ? frame : 0;
    }
    if (outItem != 0)
        *outItem = count - 1;
    if (outEntry != 0)
        *outEntry = items[count - 1];
    if (extras[count - 1] & MOTION_EXTRA_HOLD_LAST)
    {
        int frame = ids[items[count - 1]];
        return frame > 0 ? frame : 0;
    }
    int frame = ids[items[count - 1]] - (time - start) % length + length - 1;
    return frame > 0 ? frame : 0;
}
