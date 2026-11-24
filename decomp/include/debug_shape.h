// The debug shape drawing function (src/debug_shape.cpp): wire and solid shapes drawn with the plain coloured
// vertex shader. One declaration of the shape kinds and of the function: include this header.
#ifndef GURUMIN_DEBUG_SHAPE_H
#define GURUMIN_DEBUG_SHAPE_H

#include "vec4.h"
#include "matrix4.h"

// `kind` of Debug_DrawShape.
enum DebugShape
{
    DEBUG_SHAPE_ELLIPSOID = 0,   // three rings (planes yz, xz, xy) around the origin with radii a.x, a.y, a.z
    DEBUG_SHAPE_BOX = 1,         // the twelve edges of the box with corners a (lower) and b (upper)
    DEBUG_SHAPE_CYLINDER = 2,    // outline of a tube from a to b with radius a.w and flat end caps
    DEBUG_SHAPE_RINGS = 3,       // three horizontal rings (at a.z, b.z and halfway) around the box a..b
    DEBUG_SHAPE_CONE = 5,        // outline from the base at a (radius b.w) to the apex b; with w = 0 a plain line a-b
    DEBUG_SHAPE_SOLID_BOX = 6,   // the box a..b as twelve translucent blue triangles
    DEBUG_SHAPE_LINE = 10        // one line from a to b
};

// The frame of a tube or cone: [0] the unit direction from a to b, [1] a unit vector across it (scaled to the
// radius), [2] that vector turned about the direction for each section, [3] the direction scaled to the radius.
struct TubeFrame
{
    Vec4 axis[4];
    __forceinline TubeFrame() {}
};

// 0x10145ba0. Draws a debug shape of `kind` (DebugShape) from `a` and `b`, placed by `matrix` and seen through
// `view`. Returns 1.
int Debug_DrawShape(int kind, const Vec4 *a, const Vec4 *b, Matrix4 *matrix, const Matrix4 *view);

extern "C" void FUN_10143b50(void);   // Render_UnbindTexture0
extern "C" void FUN_10143b80(void);   // Render_UnbindTexture1

#endif
