// RegionVertex, RegionPoint, MapRegion: declared here once (first used by src/region_view.cpp).
#ifndef GURUMIN_REGION_VIEW_H
#define GURUMIN_REGION_VIEW_H

#include "matrix4.h"

// A vertex of a region's water surface, 0x70 bytes.
struct RegionVertex
{
    float x;
    float y;
    float z;
    char pad_000c[0x14];
    Vec4 tint;                // 0x20: blended between a point's two tints
    char pad_0030[0x10];
    Vec4 offset;              // 0x40: blended between a point's two offsets (w unused)
    Vec4 baseTint;            // 0x50: the tint as built (the wave shading scales `tint` from it)
    float baseX;              // 0x60: the vertex position as built
    float baseY;
    float baseZ;
    float nearness;           // 0x6c: distance to the closest lit vertex of another region over 900 (1 = none near)
};
// A control point of a region, 0x7c bytes.
struct RegionPoint
{
    Vec4 positionFrom;        // 0x00
    Vec4 positionTo;          // 0x10
    Vec4 tintFrom;            // 0x20: the w of the two tints decides which of the two blends runs faster
    Vec4 tintTo;              // 0x30
    Vec4 offsetFrom;          // 0x40
    Vec4 offsetTo;            // 0x50
    float m_0060;             // 0x60: its hundreds (odd or even) choose the wave's phase together with m_0064
    float m_0064;
    float m_0068;
    char pad_006c[4];
    int morphStart;           // 0x70: frame counter value when the morph began
    int morphing;             // 0x74: non-zero while it blends towards the vertex
    char pad_0078[4];
};
// Map region, 0x44 bytes.
struct MapRegion
{
    RegionVertex *vertices;   // 0x00
    void *buffer0004;         // 0x04: a third buffer of the region, freed with the others (map_unload.cpp)
    RegionPoint *points;      // 0x08
    int pointCount;           // 0x0c
    int indexCount;           // 0x10: number of 16-bit indices in buffer0004
    Vec4 min;                 // 0x14: box corner with the smallest coordinates
    Vec4 max;                 // 0x24: box corner with the largest coordinates
    int visible;              // 0x34: set by the update each frame
    int hidden;               // 0x38: non-zero = never visible
    int settled;              // 0x3c: cleared when no point is morphing
    unsigned int morphStart;  // 0x40
};

// The current map's regions (DAT_102564a0 of them), allocated by the map loader; 0 when no map is loaded.
extern "C" MapRegion *DAT_104c03c0;
extern "C" int DAT_104c03c4;               // non-zero once the regions are in use (0 = all invisible)
extern "C" int DAT_102028e0;               // texture slot of the "@moya" node (the haze of the water regions)

#endif
