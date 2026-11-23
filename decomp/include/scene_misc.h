// ColorTables, Widget, Placement: types of scene_misc.cpp.
#ifndef GURUMIN_SCENE_MISC_H
#define GURUMIN_SCENE_MISC_H

#include "vec4.h"
#include "matrix4.h"

// Four tables of 256 colours (constructor 0x10172780 in scene_misc.cpp; only the Vec4 constructors run).
struct ColorTables
{
    char pad_0000[0x14];
    Vec4 a[0x100];
    Vec4 b[0x100];
    Vec4 c[0x100];
    Vec4 d[0x100];
    ColorTables();
};

// An object holding three matrices at 0xc4; its destructor only runs theirs.
struct Widget
{
    char pad_0000[0xc4];
    Matrix4 m_00c4;
    Matrix4 m_0104;
    Matrix4 m_0144;
    ~Widget();
};

// An effect node placement (0x3d8 bytes): the kind and the model file loaded for it.
struct Placement
{
    int m_0000;               // cleared for entry 0 by the camera reset (camera_reset.cpp)
    char pad_0004[0x1fc];
    int kind;                 // 0x200
    char pad_0204[0x37];
    char name[0x40];          // 0x23b: model file name
    char pad_027b[0x15d];
};

extern "C" Placement DAT_1101dde0[];   // the effect node placements

#endif
