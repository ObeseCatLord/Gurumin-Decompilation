// Engine containers (0x101ac000-0x101af000): vertex packs, index lists, key-frame tracks and banks, a motion
// list and matrix helpers. Buffers come from the engine allocator (DAT_11b0b938 = allocate(size, tag),
// DAT_11b1ba40 = free) or the CRT heap.
//
// Key frames: a KeyTrack is a growable list of (frame, Vec4 value, flag) kept sorted by frame; a KeyBank is a
// fixed 0x80-key version used while loading. Flag 4 marks a key as a motion frame key.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Fill(const float &value);
    void Copy(const Vec4 &other);
    void Transform(const struct Matrix4 &matrix);
};

struct VertexPack
{
    int m_0000;
    int m_0004;
    int m_0008;
    int m_000c;
    int m_0010;
    int m_0014;
    int m_0018;
    int vertexCount;
    Vec4 boundsMin;
    Vec4 boundsMax;
    int m_0040;
    int m_0044;
    char pad_0048[0x28];
    void *vertices;
    int SetVertexCount(int count);
    int CopyFrom(const VertexPack *source);
    int FreeVertices();
};

struct IndexList
{
    int count;
    int *indices;
    int Resize(int newCount);
    int CopyFrom(const IndexList *source);
    int Free();
};

// List of 16-byte index spans (tagged "m_indexSP" in the allocator).
struct IndexSpanList
{
    int count;
    void *spans;
    int Resize(int newCount);
};

struct KeyTrack
{
    int m_0000;
    int *frames;
    Vec4 *values;
    char *flags;
    int count;
    void Free();
    void Reserve(int capacity);
    void Init();
    int MarkAll();
    int LastFrame();
    int HasFrame(int frame);
    int AddKey(int frame, float x, float y, float z, float w);
    int Insert(int frame, float x, float y, float z, float w);
    int CopyFrom(const KeyTrack *source);
};

struct KeyBank
{
    int m_0000[0x80];
    Vec4 m_0200[0x80];
    char flags[0x80];
    int m_0a80;
    KeyBank();
    void Clear();
    int CopyTo(KeyTrack *target);
    int Insert(int frame, float x, float y, float z, float w);
};

struct Matrix4
{
    float m[16];
    Vec4 GetTranslation();
    void SetTranslation(float x, float y, float z, float w);
    void ResetRotation();
    void SetIdentity();
    void SetRotationTransposed(const Matrix4 &source);
};

extern "C" void *(*DAT_11b0b938)(int size, const char *tag);
extern "C" void (*DAT_11b1ba40)(void *);

// Reallocates the vertex buffer for `count` vertices (0x70 bytes each, see vertex.cpp). Returns 1.
int VertexPack::SetVertexCount(int count)
{
    if (vertices != 0)
        DAT_11b1ba40(vertices);
    vertices = 0;
    void *buffer = DAT_11b0b938(count * 0x70, "");
    vertexCount = count;
    vertices = buffer;
    return 1;
}

// Frees the vertex buffer. Returns 1.
int VertexPack::FreeVertices()
{
    if (vertices != 0)
        DAT_11b1ba40(vertices);
    vertices = 0;
    return 1;
}

// Frees the index buffer and sets the count to 0. Returns 1.
int IndexList::Free()
{
    if (indices != 0)
        DAT_11b1ba40(indices);
    indices = 0;
    count = 0;
    return 1;
}

// Reallocates the index buffer for `newCount` indexes (contents are not kept). Returns 1.
int IndexList::Resize(int newCount)
{
    if (indices != 0)
        DAT_11b1ba40(indices);
    indices = 0;
    int *buffer = (int *)DAT_11b0b938(newCount * 4, "");
    count = newCount;
    indices = buffer;
    return 1;
}

// Makes this list a copy of `source`.
int IndexList::CopyFrom(const IndexList *source)
{
    count = 0;
    if (source == 0)
        return 0;
    if (source->count == 0)
        return 0;
    Resize(source->count);
    memcpy(indices, source->indices, source->count * 4);
    return 1;
}

// Empties the bank, leaving one zero key at frame 0 as the default value.
void KeyBank::Clear()
{
    m_0a80 = 0;
    m_0000[0] = 0;
    float zero = 0;
    m_0200[0].Fill(zero);
}

// Frees the three key arrays (only when they were allocated) and empties the track.
void KeyTrack::Free()
{
    if (m_0000 != 0)
    {
        free(frames);
        frames = 0;
        free(values);
        values = 0;
        free(flags);
        flags = 0;
    }
    m_0000 = 0;
    count = 0;
}

// Allocates room for 16 keys and empties the track, with a zero default key at frame 0.
void KeyTrack::Init()
{
    m_0000 = 0;
    Reserve(16);
    count = 0;
    frames[0] = 0;
    float zero = 0;
    values->Fill(zero);
}

// Marks every key as a motion frame key (flag 4).
int KeyTrack::MarkAll()
{
    for (int i = 0; i < count; i++)
        flags[i] = 4;
    return 0;
}

// Returns the highest key frame (0 for an empty track).
int KeyTrack::LastFrame()
{
    int last = 0;
    int n = count;
    for (int i = 0; i < n; i++)
    {
        if (frames[i] > last)
            last = frames[i];
    }
    return last;
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

// Returns the translation row as a vector (w copied from m[15]). Not byte-identical yet.
Vec4 Matrix4::GetTranslation()
{
    Vec4 result;
    result.x = m[12];
    result.y = m[13];
    result.z = m[14];
    return result;
}

// Sets the translation row.
void Matrix4::SetTranslation(float x, float y, float z, float w)
{
    m[12] = x;
    m[13] = y;
    m[14] = z;
}

// Sets the upper-left 3x3 block to identity (translation kept).
void Matrix4::ResetRotation()
{
    m[10] = 1.0f;
    m[5] = 1.0f;
    m[0] = 1.0f;
    m[9] = 0;
    m[8] = 0;
    m[6] = 0;
    m[4] = 0;
    m[2] = 0;
    m[1] = 0;
}

// Sets the identity matrix.
void Matrix4::SetIdentity()
{
    m[12] = m[13] = m[14] = m[8] = m[9] = m[11] = m[4] = m[6] = m[7] = m[1] = m[2] = m[3] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// Copies the transposed rotation part of `source` (the inverse rotation for a pure rotation matrix).
void Matrix4::SetRotationTransposed(const Matrix4 &source)
{
    m[0] = source.m[0];
    m[4] = source.m[1];
    m[8] = source.m[2];
    m[1] = source.m[4];
    m[5] = source.m[5];
    m[9] = source.m[6];
    m[2] = source.m[8];
    m[6] = source.m[9];
    m[10] = source.m[10];
}

// 0x101af420. Constructor: an empty bank (see the IPRA note in open.tsv).
KeyBank::KeyBank()
{
    Clear();
}

// Returns `point` transformed by `matrix`.
Vec4 TransformPoint(const Vec4 &point, const Matrix4 &matrix)
{
    Vec4 result;
    result.Copy(point);
    result.Transform(matrix);
    return result;
}
// Makes this track a copy of `source` (keys, values and flags). Returns 1.
int KeyTrack::CopyFrom(const KeyTrack *source)
{
    Free();
    Init();
    for (int i = 0; i < source->count; i++)
    {
        AddKey(source->frames[i], source->values[i].x, source->values[i].y, source->values[i].z, source->values[i].w);
        flags[i] = source->flags[i];
    }
    return 1;
}

// Copies the bank's keys into `target` (a growable track). Returns 0.
int KeyBank::CopyTo(KeyTrack *target)
{
    target->Free();
    target->Init();
    for (int i = 0; i < m_0a80; i++)
    {
        target->AddKey(m_0000[i], m_0200[i].x, m_0200[i].y, m_0200[i].z, m_0200[i].w);
        target->flags[i] = flags[i];
    }
    return 0;
}
// List of up to 0x7f motion ids plus up to 64 named entries (id, value, extra).
struct MotionList
{
    int m_0000;
    int m_0004;
    int count;
    int items[0x7f];
    int m_0208;
    int entryCount;
    char names[0x40][0x20];
    int ids[0x40];
    int values[0x40];
    int extras[0x40];
    int Insert(int position, int value, int tag);
    int AddEntry(int id, int value, const char *name, int extra);
};

// Inserts `value` at `position` (-1 = append) and records `tag`. Returns 0 when the position is past the end or
// the list is full.
int MotionList::Insert(int position, int value, int tag)
{
    m_0208 = tag;
    if (count < position)
        return 0;
    if (count > 0x7f)
        return 0;
    if (position == -1)
    {
        items[count] = value;
        count++;
        return 1;
    }
    for (int i = count; i >= position; i--)
        items[i + 1] = items[i];
    items[position] = value;
    count++;
    return 1;
}

// Adds a named entry (`name`, id, value, extra). Returns 0 when all 64 entries are used.
int MotionList::AddEntry(int id, int value, const char *name, int extra)
{
    if (entryCount > 0x3f)
        return 0;
    ids[entryCount] = id;
    values[entryCount] = value;
    strncpy(names[entryCount], name, 0x1f);
    names[entryCount][0x1f] = 0;
    extras[entryCount] = extra;
    entryCount++;
    return 1;
}
extern "C" void *(*DAT_11b07530)(void *dst, const void *src, int size);

// Makes this pack a copy of `source` (header fields and vertices).
int VertexPack::CopyFrom(const VertexPack *source)
{
    vertexCount = 0;
    if (source == 0)
        return 0;
    if (source->vertexCount == 0)
        return 0;
    SetVertexCount(source->vertexCount);
    boundsMin.Copy(source->boundsMin);
    boundsMax.Copy(source->boundsMax);
    m_0000 = source->m_0000;
    m_0004 = source->m_0004;
    m_0008 = source->m_0008;
    m_000c = source->m_000c;
    vertexCount = source->vertexCount;
    m_0010 = source->m_0010;
    m_0018 = source->m_0018;
    m_0014 = source->m_0014;
    m_0040 = source->m_0040;
    m_0044 = source->m_0044;
    DAT_11b07530(vertices, source->vertices, vertexCount * 0x70);
    return 1;
}
// Inserts key (x, y, z, w) at `frame` in frame order, or replaces the value of an existing key at that frame.
// Returns its index. Not byte-identical yet (see open.tsv).
int KeyBank::Insert(int frame, float x, float y, float z, float w)
{
    int i;
    for (i = 0; i < m_0a80; i++)
    {
        if (m_0000[i] == frame)
            break;
        if (m_0000[i] > frame)
            break;
    }
    for (int j = m_0a80 - 1; j >= i; j--)
    {
        m_0000[j + 1] = m_0000[j];
        m_0200[j + 1].Copy(m_0200[j]);
        flags[j + 1] = flags[j];
    }
    m_0200[i].x = x;
    m_0200[i].y = y;
    m_0200[i].z = z;
    m_0200[i].w = w;
    m_0000[i] = frame;
    flags[i] = 0;
    m_0a80++;
    return i;
}
// Same as KeyBank::Insert for a growable track (grows the arrays when needed).
int KeyTrack::Insert(int frame, float x, float y, float z, float w)
{
    int last = count;
    int i;
    for (i = 0; i < count; i++)
    {
        if (frames[i] == frame)
            break;
        if (frames[i] > frame)
            break;
    }
    for (int j = last - 1; j >= i; j--)
    {
        frames[j + 1] = frames[j];
        values[j + 1].Copy(values[j]);
        flags[j + 1] = flags[j];
    }
    if (count + 16 > m_0000)
        Reserve(16);
    frames[i] = frame;
    values[i].x = x;
    values[i].y = y;
    values[i].z = z;
    values[i].w = w;
    flags[i] = 0;
    count++;
    if (count + 16 > m_0000)
        Reserve(16);
    return i;
}
// Reallocates the span buffer for `newCount` 16-byte spans (contents are not kept). The allocation is tagged
// "m_indexSP(count)" for the engine's memory report. Returns 1.
int IndexSpanList::Resize(int newCount)
{
    char tag[64];
    if (spans != 0)
        DAT_11b1ba40(spans);
    spans = 0;
    sprintf(tag, "m_indexSP(%d)", newCount);
    void *buffer = DAT_11b0b938(newCount * 16, tag);
    count = newCount;
    spans = buffer;
    return 1;
}

// Cubic Hermite interpolation between `b` (t = 0) and `c` (t = 1) with neighbours `a` and `d`. The tangents are
// (b - a) / 2 + (c - b) at b and (c - b) / 2 + (d - c) at c (not the usual Catmull-Rom halves of both differences).
// Not byte-identical yet: cl 13.00 rewrites t3 * -2 as -(t3 + t3) and shares it with the b term (see open.tsv).
float __stdcall Spline(float t, float a, float b, float c, float d)
{
    float bc = c - b;
    float tangentB = (b - a) * 0.5f + bc;
    float tangentC = (d - c) + bc * 0.5f;
    float t2 = t * t;
    float t3 = t2 * t;
    float t2x3 = t2 * 3.0f;
    return (t3 - (t2 + t2) + t) * tangentB + (t3 + t3 - t2x3 + 1.0f) * b + (t3 * -2.0f + t2x3) * c +
           (t3 - t2) * tangentC;
}
