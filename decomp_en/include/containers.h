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
};

// Size of one mesh vertex in bytes.
const int VERTEX_SIZE = 0x70;

// One mesh vertex; the layout is not recovered yet.
struct Vertex
{
    char m_0000[VERTEX_SIZE];  // 0x00: position, normal, uv, weights (not split up yet)
};

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
    char m_0040[0x30];       // 0x40: not recovered
    Vertex *vertices;        // 0x70: the vertex buffer (0 = none)

    int SetVertexCount(int count);   // EN 0x772150: reallocates the vertex buffer, contents not kept
};

#endif
