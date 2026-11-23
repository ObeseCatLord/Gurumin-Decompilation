// RegionVertex, RegionPoint, MapRegion: declared here once (first used by src/region_view.cpp).
#ifndef GURUMIN_REGION_VIEW_H
#define GURUMIN_REGION_VIEW_H

#include "matrix4.h"

// A vertex of a region's water surface, 0x70 bytes (only its position is used here).
struct RegionVertex
{
    float x;
    float y;
    float z;
    char pad_000c[0x64];
};
// A control point of a region, 0x7c bytes.
struct RegionPoint
{
    char pad_0000[0x60];
    float m_0060;             // 0x60: its hundreds (odd or even) choose the wave's phase together with m_0064
    float m_0064;
    char pad_0068[0x14];
};
// Map region, 0x44 bytes.
struct MapRegion
{
    RegionVertex *vertices;   // 0x00
    void *buffer0004;         // 0x04: a third buffer of the region, freed with the others (map_unload.cpp)
    RegionPoint *points;      // 0x08
    char pad_000c[8];
    Vec4 min;                 // 0x14: box corner with the smallest coordinates
    Vec4 max;                 // 0x24: box corner with the largest coordinates
    int pad_0034;
    int hidden;               // 0x38: non-zero = never visible
    char pad_003c[8];
};

// The current map's regions (DAT_102564a0 of them), allocated by the map loader; 0 when no map is loaded.
extern "C" MapRegion *DAT_104c03c0;

#endif
