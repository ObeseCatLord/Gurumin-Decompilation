// A list of mesh indices or triangle spheres (EN build): a count and a buffer from the engine allocator.
// This is the one declaration of IndexList in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_INDEX_LIST_H
#define GURUMIN_EN_INDEX_LIST_H

#include "vec4.h"

struct IndexList
{
    int count;       // 0x00: entries in the buffer
    union
    {
        void *entries;   // 0x04: the buffer (0 = none)
        int *indices;    // 0x04: index list: the vertex indices
        Vec4 *spheres;   // 0x04: sphere list: centre and radius (w) of each triangle
    };

    int Resize(int newCount);       // EN 0x7721a0: reallocates the index buffer, contents not kept
    int ResizeSpans(int newCount);  // EN 0x7721f0: reallocates the sphere buffer (16 bytes per entry), contents not kept
    int Free();      // EN 0x778500 (JP 0x101ad0e0): frees the buffer and sets count and buffer to 0
};

#endif
