// Key tracks and vertex packs of the EN build (JP: decomp/include/key_track.h, mesh.h). KeyBank is in key_bank.h and
// IndexList in index_list.h. This is the one declaration of KeyTrack, Vertex and VertexPack in decomp_en.
#ifndef GURUMIN_EN_CONTAINERS_H
#define GURUMIN_EN_CONTAINERS_H

#include "vec4.h"

// A growable list of keys (frame number, value, flags), sorted by frame.
struct KeyTrack
{
    int capacity;     // 0x00: allocated length of the three arrays
    int *frames;      // 0x04: frame number of each key, ascending
    Vec4 *values;     // 0x08: value of each key (a position, a quaternion or a scale)
    char *flags;      // 0x0c: KEY_FLAG_* bits of each key
    int count;        // 0x10: keys in use

    int MarkAll();                 // EN 0x7765b0 (JP 0x101adb80): sets KEY_FLAG_MOTION on every key
    void Reserve(int extra);       // EN 0x7754b0 (JP 0x101ada30): grows the three arrays by `extra` entries, keeping their contents
    int HasFrame(int frame);       // EN 0x7752e0 (JP 0x101adcb0): 1 when a key sits at `frame`
    void Init();                   // EN: inline in the callers (JP 0x101ad960): one zero key at frame 0, room for 16 keys
    int Insert(int frame, float x, float y, float z, float w);  // EN 0x76f810: inserts a key before the first key at or after `frame` (an existing key at `frame` is kept), returns its index
    int Sample(int frame, float *x, float *y, float *z, float *w, int flags);  // EN 0x76fd80: the value at `frame`, interpolated between the neighbouring keys (flags 1 = as a rotation)
    int Set(int frame, float x, float y, float z, float w);  // EN 0x779540 (JP 0x101adcf0): sets or inserts the key at `frame`
    int CopyFrom(const KeyTrack *source);  // EN 0x771de0 (JP 0x101ade10): makes this track a copy of `source`
    void Free();                   // EN 0x778450 (JP 0x101ad7b0): frees the three arrays and empties the track
    int NextFrame(int frame);      // EN 0x7772e0: frame of the first key after `frame` (the last key when none is later, 0 for an empty track)
    int PreviousFrame(int frame);  // EN 0x777670: frame of the last key before `frame` (the first key when none is earlier, 0 for an empty track)
};

// Size of one mesh vertex in bytes.
const int VERTEX_SIZE = 0x70;

// One mesh vertex.
struct Vertex
{
    float position[4];                    // 0x00: x, y, z, 1
    float normal[4];                      // 0x10: normal x, y, z, 1
    float colour[4];                      // 0x20: colour red, green, blue, alpha (0..1)
    float uv[2];                          // 0x30: first texture coordinates u, v
    int sourceMaterial;                   // 0x38: material the vertex was read with
    int material;                         // 0x3c: material index
    float emissive[4];                    // 0x40: emissive colour
    float m_0050[4];                      // 0x50: not recovered (cleared by Set)
    float m_0060;                         // 0x60: not recovered (1 after Set)
    float m_0064[3];                      // 0x64: not recovered (cleared by Set)

    void SetPosition(float x, float y, float z);   // EN 0x779b20 (JP 0x101acbb0): sets the position, everything else to defaults
    void Set(float x, float y, float z, float nx, float ny, float nz, float u, float v, float r, float g, float b, float a);  // EN 0x779ec0: position, normal, uv and colour in one call
};

static_assert(sizeof(Vertex) == VERTEX_SIZE, "Vertex must be 0x70 bytes");

// One drawable pack of a mesh: bounds, material and the vertex buffer.
struct VertexPack
{
    float center[3];         // 0x00: centre of the pack's vertices
    float radius;            // 0x0c: bounding radius (0 = not needed)
    int m_0010;              // 0x10: not recovered
    int material;            // 0x14: index into the node's materials
    int m_0018;              // 0x18: not recovered
    int vertexCount;         // 0x1c: vertices in the buffer
    Vec4 boundsMin;          // 0x20: smallest x, y, z of the vertices (w = 1)
    Vec4 boundsMax;          // 0x30: largest x, y, z (w = 1)
    int m_0040;              // 0x40: not recovered (cleared by Init)
    int m_0044;              // 0x44: not recovered (cleared by Init)
    char m_0048[0x28];       // 0x48: not recovered
    Vertex *vertices;        // 0x70: the vertex buffer (0 = none)

    int SetVertexCount(int count);   // EN 0x772150: reallocates the vertex buffer, contents not kept
    void Init();                     // EN 0x7787e0 (JP 0x101acfa0): empties a pack allocated raw
    int CopyFrom(const VertexPack *source);  // EN 0x771eb0 (JP 0x101ad040): copies the bounds and the vertices; returns 0 for an empty source
    int FreeVertices();              // EN 0x7784a0 (JP 0x101acfc0): frees the vertex buffer (the count is kept); returns 1
};

#endif
