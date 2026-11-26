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

    IndexList();                    // EN 0x77be70: an empty list
    void InitSpheres();             // EN 0x778830: clears the buffer pointer of a sphere list allocated raw (identical to Init)
    int Resize(int newCount);      // EN 0x7721a0: reallocates the index buffer, contents not kept
    int ResizeSpans(int newCount);  // EN 0x7721f0: reallocates the sphere buffer (16 bytes per entry), contents not kept
    void Init();                    // EN 0x778820 (JP 0x101ad1b0): clears the buffer pointer of a list allocated raw
    int CopyFrom(const IndexList *source);  // EN 0x771fa0 (JP 0x101ad160): copies an index list; returns 0 for an empty source
    void FreeEntries();  // EN 0x76f120 (index_list_free.cpp, EN-only): frees the buffer and sets count and buffer to 0 (a void twin of FreeIndices)
    int FreeIndices();// EN 0x7784d0 (JP 0x101ad0e0): frees the index buffer and sets count and buffer to 0; returns 1
    int FreeSpheres();  // EN 0x778500: the same for a sphere list (identical code, separate function)
};

// Allocates an array of `count` empty lists (each Init-ed) from the engine allocator and returns it (see pack_arrays.cpp).
// EN 0x755710.
extern "C" IndexList * __stdcall IndexLists_NewArray(int count);

#endif
