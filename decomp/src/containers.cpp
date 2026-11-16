#include <stdlib.h>
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
    char pad_0000[0x1c];
    int vertexCount;
    char pad_0020[0x50];
    void *vertices;
    int SetVertexCount(int count);
};

struct IndexList
{
    int count;
    int *indices;
    int Resize(int newCount);
    int CopyFrom(const IndexList *source);
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
};

struct KeyBank
{
    int m_0000;
    char pad_0004[0x1fc];
    Vec4 m_0200[0x80];
    char pad_0a00[0x80];
    int m_0a80;
    KeyBank();
    void Clear();
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

void KeyBank::Clear()
{
    m_0a80 = 0;
    m_0000 = 0;
    float zero = 0;
    m_0200[0].Fill(zero);
}

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

void KeyTrack::Init()
{
    m_0000 = 0;
    Reserve(16);
    count = 0;
    frames[0] = 0;
    float zero = 0;
    values->Fill(zero);
}

int KeyTrack::MarkAll()
{
    for (int i = 0; i < count; i++)
        flags[i] = 4;
    return 0;
}

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

Vec4 Matrix4::GetTranslation()
{
    Vec4 result;
    result.x = m[12];
    result.y = m[13];
    result.z = m[14];
    return result;
}

void Matrix4::SetTranslation(float x, float y, float z, float w)
{
    m[12] = x;
    m[13] = y;
    m[14] = z;
}

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

void Matrix4::SetIdentity()
{
    m[12] = m[13] = m[14] = m[8] = m[9] = m[11] = m[4] = m[6] = m[7] = m[1] = m[2] = m[3] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

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

KeyBank::KeyBank()
{
    Clear();
}

Vec4 TransformPoint(const Vec4 &point, const Matrix4 &matrix)
{
    Vec4 result;
    result.Copy(point);
    result.Transform(matrix);
    return result;
}