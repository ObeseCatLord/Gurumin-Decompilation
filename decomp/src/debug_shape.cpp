// The debug shape drawing function: boxes, rings, cones and lines drawn with the untextured coloured vertex shader,
// used by the node tree's debug views (skeletons and collision shapes, node_queue.cpp).

#include <math.h>

#include "d3d9_min.h"

#include "vec4.h"

#include "matrix4.h"

#include "mesh.h"

#include "render_vars.h"

#include "render_state.h"

#include "debug_draw_vars.h"

#include "mesh_draw_vars.h"

#include "debug_shape.h"

#define D3DPT_LINELIST_ 2
#define D3DPT_LINESTRIP_ 3
#define D3DPT_TRIANGLELIST_ 4
#define D3DFMT_INDEX16_ 101
#define SHADER_CONSTANT_TRANSFORM 0
#define SHADER_CONSTANT_TINT 0x5f
#define VERTEX_BUFFER_CAPACITY 32
#define RING_SEGMENTS 16
#define RING_VERTEX_COUNT 17
#define FULL_TURN 6.283f
#define QUARTER_TURN 1.57075f
#define EIGHTH_TURN 0.785375f
#define VERTICAL_AXIS_LIMIT 0.99
#define RING_RADIUS_SCALE 0.5f
#define RING_RADIUS_DIVISOR 1.414f
#define CONE_APEX_SCALE 0.0f
#define TUBE_SECTIONS 4
#define TUBE_VERTICES_PER_SECTION 6
#define TUBE_LINE_COUNT 24
#define BOX_VERTEX_COUNT 8
#define BOX_EDGE_COUNT 12
#define BOX_TRIANGLE_COUNT 12
#define SOLID_BOX_ALPHA 0.3f

// 0x10145ba0 (Debug_DrawShape). Draws a debug shape of `kind` (DebugShape) from `a` and `b`, placed by `matrix`
// and seen through `view`: sets the texture stages to plain vertex colour, unbinds both textures, loads the
// transposed matrix * view and a white tint into the vertex shader constants and selects the debug vertex shader.
// Rings and tubes are line strips of 17 white vertices built by turning a template vertex about an axis; the box
// edges and the translucent solid box are indexed lists. The depth test is switched on again at the end. Callers:
// the node tree debug views. Returns 1.
int Debug_DrawShape(int kind, const Vec4 *a, const Vec4 *b, Matrix4 *matrix, const Matrix4 *view)
{
    FUN_1013ef90(0, 4, 3);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(0, 1, 3);
    FUN_1013ef90(0, 2, 2);
    FUN_1013ef90(0, 3, 0);
    FUN_10143b50();
    FUN_10143b80();
    Matrix4 transform;
    transform = Transpose(*matrix * *view);
    DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_TRANSFORM, (const float *)&transform, 4);
    float tint[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_TINT, tint, 1);
    DAT_10f1fcf0->SetVertexShader(DAT_10f0ee14);
    DAT_10f1fcf0->SetVertexDeclaration(DAT_10f654ac);

    Vertex vertices[VERTEX_BUFFER_CAPACITY];
    int i;
    if (kind == DEBUG_SHAPE_LINE)
    {
        FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
        Vec4 unused;
        Vertex vertex;
        vertex.Set(1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        Vertex line[2];
        line[0] = vertex;
        line[1] = vertex;
        line[0].position[0] = a->x;
        line[0].position[1] = a->y;
        line[0].position[2] = a->z;
        line[1].position[0] = b->x;
        line[1].position[1] = b->y;
        line[1].position[2] = b->z;
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_LINELIST_, 1, line, sizeof(Vertex));
    }
    else if (kind == DEBUG_SHAPE_RINGS)
    {
        float dx, dy;
        dy = b->y - a->y;
        dx = b->x - a->x;
        float length = (float)sqrt(dx * dx + dy * dy);
        float radius = length * RING_RADIUS_SCALE / RING_RADIUS_DIVISOR;
        Vec4 point;
        Vertex vertex;
        vertex.Set(1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        for (i = 0; i < RING_VERTEX_COUNT; i++)
        {
            vertices[i] = vertex;
            point.x = vertex.position[0];
            point.y = vertex.position[1];
            point.z = vertex.position[2];
            point.RotateZ((float)i * FULL_TURN / RING_SEGMENTS);
            vertices[i].position[0] = point.x * radius;
            vertices[i].position[1] = point.y * radius;
            vertices[i].position[2] = a->z;
        }
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_LINESTRIP_, RING_SEGMENTS, vertices, sizeof(Vertex));
        vertex.Set(1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        for (i = 0; i < RING_VERTEX_COUNT; i++)
        {
            vertices[i] = vertex;
            point.x = vertex.position[0];
            point.y = vertex.position[1];
            point.z = vertex.position[2];
            point.RotateZ((float)i * FULL_TURN / RING_SEGMENTS);
            vertices[i].position[0] = point.x * radius;
            vertices[i].position[1] = point.y * radius;
            vertices[i].position[2] = b->z;
        }
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_LINESTRIP_, RING_SEGMENTS, vertices, sizeof(Vertex));
        vertex.Set(1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        for (i = 0; i < RING_VERTEX_COUNT; i++)
        {
            vertices[i] = vertex;
            point.x = vertex.position[0];
            point.y = vertex.position[1];
            point.z = vertex.position[2];
            point.RotateZ((float)i * FULL_TURN / RING_SEGMENTS);
            vertices[i].position[0] = point.x * radius;
            vertices[i].position[1] = point.y * radius;
            vertices[i].position[2] = (a->z + b->z) / 2.0f;
        }
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_LINESTRIP_, RING_SEGMENTS, vertices, sizeof(Vertex));
    }
    else if (kind == DEBUG_SHAPE_CONE)
    {
        int section;
        TubeFrame frame;
        Vec4 (&axis)[4] = frame.axis;
        axis[0].Copy(*b - *a);
        axis[0].Normalized3();
        axis[1].Copy(axis[0]);
        if (axis[0].z > VERTICAL_AXIS_LIMIT)
            axis[1].RotateX(QUARTER_TURN);
        else
            axis[1].RotateZ(QUARTER_TURN);
        axis[3].Copy(axis[0] * b->w);
        axis[1].Copy(axis[1] * b->w);
        Vertex vertex;
        vertex.Set(0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        for (section = 0; section < TUBE_SECTIONS; section++)
        {
            vertices[section * TUBE_VERTICES_PER_SECTION + 0] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 1] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 2] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 3] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 4] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 5] = vertex;
            axis[2].Copy(axis[1]);
            axis[2].RotateAxis(axis[0], (float)section * EIGHTH_TURN);
            vertices[section * TUBE_VERTICES_PER_SECTION + 0].position[0] = a->x - axis[3].x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 0].position[1] = a->y - axis[3].y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 0].position[2] = a->z - axis[3].z;
            vertices[section * TUBE_VERTICES_PER_SECTION + 1].position[0] = axis[2].x + a->x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 1].position[1] = axis[2].y + a->y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 1].position[2] = axis[2].z + a->z;
            vertices[section * TUBE_VERTICES_PER_SECTION + 2].position[0] = axis[2].x * CONE_APEX_SCALE + b->x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 2].position[1] = axis[2].y * CONE_APEX_SCALE + b->y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 2].position[2] = axis[2].z * CONE_APEX_SCALE + b->z;
            vertices[section * TUBE_VERTICES_PER_SECTION + 3].position[0] = axis[3].x * CONE_APEX_SCALE + b->x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 3].position[1] = axis[3].y * CONE_APEX_SCALE + b->y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 3].position[2] = axis[3].z * CONE_APEX_SCALE + b->z;
            vertices[section * TUBE_VERTICES_PER_SECTION + 4].position[0] = b->x - axis[2].x * CONE_APEX_SCALE;
            vertices[section * TUBE_VERTICES_PER_SECTION + 4].position[1] = b->y - axis[2].y * CONE_APEX_SCALE;
            vertices[section * TUBE_VERTICES_PER_SECTION + 4].position[2] = b->z - axis[2].z * CONE_APEX_SCALE;
            vertices[section * TUBE_VERTICES_PER_SECTION + 5].position[0] = a->x - axis[2].x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 5].position[1] = a->y - axis[2].y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 5].position[2] = a->z - axis[2].z;
        }
        vertices[section * TUBE_VERTICES_PER_SECTION] = vertex;
        vertices[section * TUBE_VERTICES_PER_SECTION].position[0] = a->x - axis[3].x;
        vertices[section * TUBE_VERTICES_PER_SECTION].position[1] = a->y - axis[3].y;
        vertices[section * TUBE_VERTICES_PER_SECTION].position[2] = a->z - axis[3].z;
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_LINESTRIP_, TUBE_LINE_COUNT, vertices, sizeof(Vertex));
    }
    else if (kind == DEBUG_SHAPE_CYLINDER)
    {
        int section;
        TubeFrame frame;
        Vec4 (&axis)[4] = frame.axis;
        axis[0].Copy(*b - *a);
        axis[0].Normalized3();
        axis[1].Copy(axis[0]);
        if (axis[0].z > VERTICAL_AXIS_LIMIT)
            axis[1].RotateX(QUARTER_TURN);
        else
            axis[1].RotateZ(QUARTER_TURN);
        axis[3].Copy(axis[0] * a->w);
        axis[1].Copy(axis[1] * a->w);
        Vertex vertex;
        vertex.Set(0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        for (section = 0; section < TUBE_SECTIONS; section++)
        {
            vertices[section * TUBE_VERTICES_PER_SECTION + 0] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 1] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 2] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 3] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 4] = vertex;
            vertices[section * TUBE_VERTICES_PER_SECTION + 5] = vertex;
            axis[2].Copy(axis[1]);
            axis[2].RotateAxis(axis[0], (float)section * EIGHTH_TURN);
            vertices[section * TUBE_VERTICES_PER_SECTION + 0].position[0] = a->x - axis[3].x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 0].position[1] = a->y - axis[3].y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 0].position[2] = a->z - axis[3].z;
            vertices[section * TUBE_VERTICES_PER_SECTION + 1].position[0] = axis[2].x + a->x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 1].position[1] = axis[2].y + a->y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 1].position[2] = axis[2].z + a->z;
            vertices[section * TUBE_VERTICES_PER_SECTION + 2].position[0] = axis[2].x + b->x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 2].position[1] = axis[2].y + b->y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 2].position[2] = axis[2].z + b->z;
            vertices[section * TUBE_VERTICES_PER_SECTION + 3].position[0] = axis[3].x + b->x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 3].position[1] = axis[3].y + b->y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 3].position[2] = axis[3].z + b->z;
            vertices[section * TUBE_VERTICES_PER_SECTION + 4].position[0] = b->x - axis[2].x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 4].position[1] = b->y - axis[2].y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 4].position[2] = b->z - axis[2].z;
            vertices[section * TUBE_VERTICES_PER_SECTION + 5].position[0] = a->x - axis[2].x;
            vertices[section * TUBE_VERTICES_PER_SECTION + 5].position[1] = a->y - axis[2].y;
            vertices[section * TUBE_VERTICES_PER_SECTION + 5].position[2] = a->z - axis[2].z;
        }
        vertices[section * TUBE_VERTICES_PER_SECTION] = vertex;
        vertices[section * TUBE_VERTICES_PER_SECTION].position[0] = a->x - axis[3].x;
        vertices[section * TUBE_VERTICES_PER_SECTION].position[1] = a->y - axis[3].y;
        vertices[section * TUBE_VERTICES_PER_SECTION].position[2] = a->z - axis[3].z;
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_LINESTRIP_, TUBE_LINE_COUNT, vertices, sizeof(Vertex));
    }
    else if (kind == DEBUG_SHAPE_ELLIPSOID)
    {
        Vec4 point;
        Vertex vertex;
        vertex.Set(0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        for (i = 0; i < RING_VERTEX_COUNT; i++)
        {
            vertices[i] = vertex;
            point.x = vertex.position[0];
            point.y = vertex.position[1];
            point.z = vertex.position[2];
            point.RotateX((float)i * FULL_TURN / RING_SEGMENTS);
            vertices[i].position[0] = point.x * a->x;
            vertices[i].position[1] = point.y * a->y;
            vertices[i].position[2] = point.z * a->z;
        }
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_LINESTRIP_, RING_SEGMENTS, vertices, sizeof(Vertex));
        vertex.Set(1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        for (i = 0; i < RING_VERTEX_COUNT; i++)
        {
            vertices[i] = vertex;
            point.x = vertex.position[0];
            point.y = vertex.position[1];
            point.z = vertex.position[2];
            point.RotateY((float)i * FULL_TURN / RING_SEGMENTS);
            vertices[i].position[0] = point.x * a->x;
            vertices[i].position[1] = point.y * a->y;
            vertices[i].position[2] = point.z * a->z;
        }
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_LINESTRIP_, RING_SEGMENTS, vertices, sizeof(Vertex));
        vertex.Set(1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1);
        for (i = 0; i < RING_VERTEX_COUNT; i++)
        {
            vertices[i] = vertex;
            point.x = vertex.position[0];
            point.y = vertex.position[1];
            point.z = vertex.position[2];
            point.RotateZ((float)i * FULL_TURN / RING_SEGMENTS);
            vertices[i].position[0] = point.x * a->x;
            vertices[i].position[1] = point.y * a->y;
            vertices[i].position[2] = point.z * a->z;
        }
        DAT_10f1fcf0->DrawPrimitiveUP(D3DPT_LINESTRIP_, RING_SEGMENTS, vertices, sizeof(Vertex));
    }
    else if (kind == DEBUG_SHAPE_BOX)
    {
        vertices[0].SetPosition(a->x, a->y, a->z);
        vertices[1].SetPosition(b->x, a->y, a->z);
        vertices[2].SetPosition(b->x, b->y, a->z);
        vertices[3].SetPosition(a->x, b->y, a->z);
        vertices[4].SetPosition(a->x, a->y, b->z);
        vertices[5].SetPosition(b->x, a->y, b->z);
        vertices[6].SetPosition(b->x, b->y, b->z);
        vertices[7].SetPosition(a->x, b->y, b->z);
        unsigned short indices[BOX_EDGE_COUNT * 2];
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 0;
        indices[3] = 3;
        indices[4] = 0;
        indices[5] = 4;
        indices[6] = 3;
        indices[7] = 2;
        indices[8] = 2;
        indices[9] = 1;
        indices[10] = 6;
        indices[11] = 7;
        indices[12] = 5;
        indices[13] = 1;
        indices[14] = 5;
        indices[15] = 6;
        indices[16] = 5;
        indices[17] = 4;
        indices[18] = 4;
        indices[19] = 7;
        indices[20] = 7;
        indices[21] = 3;
        indices[22] = 6;
        indices[23] = 2;
        DAT_10f1fcf0->DrawIndexedPrimitiveUP(D3DPT_LINELIST_, 0, BOX_VERTEX_COUNT, BOX_EDGE_COUNT, indices,
                                             D3DFMT_INDEX16_, vertices, sizeof(Vertex));
    }
    else if (kind == DEBUG_SHAPE_SOLID_BOX)
    {
        FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
        vertices[0].SetPosition(a->x, a->y, a->z);
        vertices[1].SetPosition(b->x, a->y, a->z);
        vertices[2].SetPosition(b->x, b->y, a->z);
        vertices[3].SetPosition(a->x, b->y, a->z);
        vertices[4].SetPosition(a->x, a->y, b->z);
        vertices[5].SetPosition(b->x, a->y, b->z);
        vertices[6].SetPosition(b->x, b->y, b->z);
        vertices[7].SetPosition(a->x, b->y, b->z);
        for (int i = 0; i < BOX_VERTEX_COUNT; i++)
        {
            vertices[i].colour[0] = 0.5f;
            vertices[i].colour[1] = 0.5f;
            vertices[i].colour[2] = 1.0f;
            vertices[i].colour[3] = SOLID_BOX_ALPHA;
        }
        unsigned short indices[BOX_TRIANGLE_COUNT * 3];
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 5;
        indices[3] = 0;
        indices[4] = 5;
        indices[5] = 4;
        indices[6] = 4;
        indices[7] = 5;
        indices[8] = 6;
        indices[9] = 4;
        indices[10] = 6;
        indices[11] = 7;
        indices[12] = 7;
        indices[13] = 6;
        indices[14] = 2;
        indices[15] = 7;
        indices[16] = 2;
        indices[17] = 3;
        indices[18] = 3;
        indices[19] = 2;
        indices[20] = 1;
        indices[21] = 3;
        indices[22] = 1;
        indices[23] = 0;
        indices[24] = 1;
        indices[25] = 2;
        indices[26] = 6;
        indices[27] = 1;
        indices[28] = 6;
        indices[29] = 5;
        indices[30] = 3;
        indices[31] = 0;
        indices[32] = 4;
        indices[33] = 3;
        indices[34] = 4;
        indices[35] = 7;
        DAT_10f1fcf0->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST_, 0, BOX_VERTEX_COUNT, BOX_TRIANGLE_COUNT, indices,
                                             D3DFMT_INDEX16_, vertices, sizeof(Vertex));
    }
    FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
    return 1;
}
